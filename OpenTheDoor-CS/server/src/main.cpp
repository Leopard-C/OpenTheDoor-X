
#include <iostream>
#include <mutex>

#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <wiringPi.h>

#include <jsoncpp/json/json.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include "../include/httplib.h"
#include "../include/thread_pool.hpp"

#include "../include/led.h"
#include "../include/notifier.h"
#include "../include/motor.h"
#include "../include/utils.h"

using namespace httplib;

#define SIGLOCK 45
#define SIGUNLOCK 46


// 全局变量
Led         g_led;
Motor       g_motor;
Notifier    g_notifier;
int         g_server_port;
std::string g_server_addr;
std::string g_private_key;
auto daily_logger = spdlog::daily_logger_mt("daily_logger", 
      utils::absolutePath("logs/daily.txt"), 2, 30);
SSLServer g_svr(utils::absolutePath("cert/server_cert.pem").c_str(),
      utils::absolutePath("cert/server_key.pem").c_str());

bool readConfig(const std::string& configFile);
int  getPid(const std::string& name);   // 获取指定进程的pid
void setup();
void run();
void exitFunc();
void handleCtrlC(int num);

void handleRequest(const Request& req);
void handleOpen(const Request& req);
void handleLock(const Request& req);
void handleUnLock(const Request& req);
void handleStop(const Request& req);


/********************************* main() begin **********************/

int main(int argc, char* argv[]) {
  daily_logger->flush_on(spdlog::level::warn);
  daily_logger->info("Start");
  DEBUG("Start");

  // 注册退出函数
  atexit(exitFunc);
  // 捕获 ctrl+c
  signal(SIGINT, handleCtrlC);

  // 初始化配置
  setup();

  std::mutex mutexRunning;
  ThreadPool pool(4);

  g_svr.Post("/", [&](const Request& req, Response& res){
    DEBUG("Post request");
    pool.enqueue([&, req]{ 
      if (mutexRunning.try_lock()) {
        handleRequest(req);
        mutexRunning.unlock();
        DEBUG("Handle requset OVER");
      }
      else {
        DEBUG("System is busy");
        daily_logger->error("System is busy");
      }
    });

    res.set_content("OK!", "text/plain");
  }); // end svr.Post

  // 告知管理员，程序正常启动
  if (!g_notifier.notifyMaster(NOTIFY_SYS_START)) {
    daily_logger->error("Notify master failed");
  }

  daily_logger->flush();

  // 开启监听客户端
  g_svr.listen(g_server_addr.c_str(), g_server_port);

  DEBUG("Started error");
  daily_logger->error("Started error!");
  if (!g_notifier.notifyMaster(NOTIFY_ERROR)) {
    daily_logger->error("Notify master error");
  }

  return 0;
}

/********************************* main() end *************************/


// 处理请求
void handleRequest(const Request& req) {
  DEBUG("Handling request");
  do {
    // 判断private key
    if (!req.has_param("pk")) {
      daily_logger->error("Request has no param: 'pk'");
      g_notifier.notifyMaster(NOTIFY_ERROR);
      break;
    }

    std::string pk = req.get_param_value("pk");
    if (pk != g_private_key) {
      daily_logger->error("Private key is error. pk={0}", pk);
      g_notifier.notifyMaster(NOTIFY_ERROR);
      break;
    }

    if (!req.has_param("type")) {
      daily_logger->warn("Request has no param: 'type'");
      g_notifier.notifyMaster(NOTIFY_ERROR);
      break;
    }

    std::string type = req.get_param_value("type");
    if (type == "open")
      handleOpen(req);
    else if (type == "lock")
      handleLock(req);
    else if (type == "unlock")
      handleUnLock(req);
    else if (type == "stop")
      handleStop(req);
  } while (false);

  daily_logger->flush();
}


// 处理开门请求
void handleOpen(const Request& req) {
  DEBUG("Handling open");
  // @from 请求来源：树莓派内置client、Windows软件请求、手机
  // @who  请求开门的宿舍成员id
  if (!req.has_param("from")) {
    daily_logger->warn("Request has no param: 'from'");
    g_notifier.notifyMaster(NOTIFY_ERROR);
    return;
  }
  if (!req.has_param("who")) {
    daily_logger->warn("Request has no param: 'who'");
    g_notifier.notifyMaster(NOTIFY_ERROR);
  }
  std::string from = req.get_param_value("from");
  std::string who  = req.get_param_value("who");
  int whoID = utils::toInt(who.c_str());

  // whoID 0~3
  if (whoID > 3) {
    daily_logger->warn("Request's who_id > 3. who_id = {0:d}", whoID);
    g_notifier.notifyMaster(NOTIFY_ERROR);
    return;
  }

  // 外部请求
  if (from != "pc" && from != "built-in" && from != "app" 
      && from != "mini-app" && from != "qq") {
    daily_logger->error("From unknown way, '{0}'", from);
    g_notifier.notifyMaster(NOTIFY_ERROR);
    return;
  }

  DEBUG("Opening the door");

  // 开门
  g_motor.rotateCW();

  // 判断开门渠道：指纹、微信小程序、QQ、PC
  if (from == "pc") {
    daily_logger->info("From PC. {0} 开了门", getName(WHO(whoID)) );
    g_led.greenT(2000);
    sleep(8);
  }
  else if (from == "app") {
    daily_logger->info("From APP. {0} 开了门", getName(WHO(whoID)) );
    g_led.greenT(2000);
    sleep(5);
  }
  else if (from == "mini-app") {
    daily_logger->info("From MiniAPP. {0} 开了门", getName(WHO(whoID)) );
    g_led.greenT(2000);
    sleep(5);
  }
  // 树莓派内置clien的请求（即通过指纹开门）
  else if (from == "built-in") {
    daily_logger->info("From Built-in. {0} 开了门", getName(WHO(whoID)) );
    sleep(5);
  }
  else if (from == "qq") {
    daily_logger->info("From QQ. {0} 开了门", getName(WHO(whoID)) );
    sleep(5);
  }

  // 关门
  g_motor.rotateCCW();

  DEBUG("Door opened");

  // 通知所有寝室成员
  if (!g_notifier.notifyAll(NOTIFY_DOOR_OPEN, from, WHO(whoID))) {
    daily_logger->error("Notify dorm members error");
  }
}


// 停止指纹识别(锁住客户端) 
void handleLock(const Request& req) {
  DEBUG("Handling lock");
  // 获取客户端pid
  int clientPid = getPid("door_client");
  if (clientPid > 0)
    kill(clientPid, SIGLOCK);
}


// 解锁指纹识别(客户端) 
void handleUnLock(const Request& req) {
  DEBUG("Handling unlock");
  // 获取客户端pid
  int clientPid = getPid("door_client");
  if (clientPid > 0)
    kill(clientPid, SIGUNLOCK);
}

// 停止服务器
void handleStop(const Request& req) {
  g_svr.stop();
}

// 获取指定进程的pid
int  getPid(const std::string& name) {
  char pidStr[32];  // big enough
  std::string cmd = "pidof ";
  cmd += name;
  if (!utils::execShell(cmd.c_str(), pidStr, 32)) {
    DEBUG("Exec shell error");
    return 0;
  }
  pidStr[31] = 0;
  DEBUGF("%s\n", pidStr);
  int pid = utils::toInt(pidStr);
  DEBUG(pid);
  return pid;
}

// 初始化配置
void setup() {
  // 读取配置文件
  if (!readConfig(utils::absolutePath("config/server.json"))) {
    daily_logger->error("Read config/server.json ERROR");
    DEBUG("Read config/server.json error");
    exit(1);
  }

  // 初始化wiringPi库
  if (-1 == wiringPiSetup()) {
    daily_logger->error("WiringPi setup failed");
    DEBUG("WiringPi setup failed!");
    exit(1);
  }

  // 初始化电机
  if (!g_motor.setup()) {
    daily_logger->error("Motor setup failed");
    DEBUG("Motor setup failed!");
    exit(1);
  }

  // 初始化Led
  if (!g_led.setup()) {
    daily_logger->error("Led setup failed");
    DEBUG("Led setup failed");
    exit(1);
  }

  // 初始化通知器
  if (!g_notifier.setup()) {
    daily_logger->error("Notifier setup failed");
    DEBUG("Notifier setup failed");
    exit(1);
  }
} // end setup()


// 退出函数
void exitFunc() {
  DEBUG("Exit");
  g_notifier.notifyMaster(NOTIFY_SYS_STOP);
  daily_logger->info("Exit!");
  spdlog::shutdown();
}

// 捕获Ctrl+C
void handleCtrlC(int num) {
  DEBUG("Catch SIGINT");
  daily_logger->info("Catch SIGINT");
  exit(1);
}


// 读取配置文件
bool readConfig(const std::string& configFile) {
  std::ifstream ifs(configFile);
  if (!ifs.is_open())
    return false;

  bool ret = false;
  Json::Reader reader;
  Json::Value  root;

  do {
    if (!reader.parse(ifs, root, false)) {
      break;
    }

    if (root["private_key"].isNull() ||
        root["server_addr"].isNull() ||
        root["server_port"].isNull()) {
      break;
    }

    g_server_addr = root["server_addr"].asString();
    g_server_port = root["server_port"].asInt();
    g_private_key = root["private_key"].asString();

    ret = true;
  } while (false);

  ifs.close();
  return ret;
}


