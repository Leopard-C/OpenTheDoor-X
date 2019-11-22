
#include <cstdlib>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#include <chrono>
#include <iostream>
#include <string>

#include <jsoncpp/json/json.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>

#include "../include/as608.h"
#include "../include/client.h"
#include "../include/fp_module.h"
#include "../include/httplib.h"
#include "../include/led.h"
#include "../include/utils.h"


#define SIGLOCK   45
#define SIGUNLOCK 46

// 全局变量
FpModule g_fp;
Led      g_led;
Client   g_client;
int  g_error_count = 0;
bool g_is_locked = false;
auto daily_logger = spdlog::daily_logger_mt("daily_logger", utils::absolutePath("logs/daily.txt"), 2, 30);

void setup();
void run();
void createSignal(int sig, void (*fun)(int sigNum, siginfo_t* info, void* action));
void handleLockSignal(int sigNum, siginfo_t* info, void* myAction);
void handleUnlockSignal(int sigNum, siginfo_t* info, void* myAction);
void exitFunc();
void handleCtrlC(int num);

void handleMatched(int pageID, int score);
void handleUnmatched();


/********************************* main() begin **********************/

int main(int argc, char* argv[]) {
  // 注册退出函数
  atexit(exitFunc);
  // 捕获 SIGINT 信号
  signal(SIGINT, handleCtrlC);
  // 创建并捕获 45、46 信号
  createSignal(SIGLOCK, &handleLockSignal);
  createSignal(SIGUNLOCK, &handleUnlockSignal);

  // 初始化配置
  setup();

  // 主循环
  run();

  return 0;
}

/********************************* main() end *************************/


// 初始化配置
void setup() {
  daily_logger->info("Start");

  // 初始化wiringPi库
  if (-1 == wiringPiSetup()) {
    daily_logger->error("WiringPi setup failed");
    exit(1);
  }

  // 初始化指纹识别模块
  if (!g_fp.setup()) {
    daily_logger->error("Fp module setup failed");
    exit(1);
  }

  // 初始化Led
  if (!g_led.setup()) {
    daily_logger->error("Led setup failed");
    exit(1);
  }

  // 读取配置文件
  if (!g_client.setup()) {
    daily_logger->error("Read config/client.json failed");
    exit(1);
  }

} // end setup()


// 退出函数
void exitFunc() {
  daily_logger->info("Exit");
  spdlog::shutdown();
}

// 捕获Ctrl+C
void handleCtrlC(int num) {
  daily_logger->info("Catch SIGINT");
  exit(2);
}


// 主循环
void run() {
  int pageID = 0;
  int score  = 0;
  int poweredTime = 0;

  while(true) {
    // 如果被锁住，每5s检测一次是否解锁
    if (g_is_locked) {
      std::this_thread::sleep_for(std::chrono::seconds(5));
      continue;
    }

    if (PS_DetectFinger(HIGH)) {
      delay(100);
      // 如果没有供电，就供电
      if (!g_fp.powered) {
        if (!g_fp.power(POWER_ON))
          continue;
        poweredTime = 0;
      }
      
      // 采集指纹并匹配
      if (g_fp.match(pageID, score))
        handleMatched(pageID, score);
      else
        handleUnmatched();
    }

    delay(200);
    if (g_fp.powered) {
      poweredTime += 200;
      // 10s内仍未匹配成功，AS608断电
      if (poweredTime >= 10000) {
        daily_logger->warn("10s内门未打开");
        daily_logger->flush();
        g_fp.power(POWER_OFF);
      }
    }

  } // end while

} // end function run()


// 处理指纹匹配成功的情况
void handleMatched(int pageID, int score) {
  g_error_count = 0;
  g_fp.power(POWER_OFF);

  DEBUGF("matched, pageID=%d score=%d\n", pageID, score);
  daily_logger->info("Matched, pageID={0:d}, score={1:d}", pageID, score);

  // 通知服务器开门
  g_client.clearParam();
  g_client.setParamWho(std::to_string(pageID / 10));
  g_client.setParamType("open");
  if (!g_client.Post()) {
    DEBUG("Post to server error");
    daily_logger->error("Post to server failed, pageID={0:d}, score={1:d}", pageID, score);
  }

  // 绿灯亮1s
  g_led.greenT(1000);
  // 系统暂停5s
  std::this_thread::sleep_for(std::chrono::seconds(5));
  
  daily_logger->flush();
}


// 处理指纹匹配识失败的情况
void handleUnmatched() {
  DEBUG("Not matched");
  g_error_count++;
  // 10此识别失败，锁住本程序
  if (g_error_count > 9) {
    g_error_count = 0;
    g_is_locked = true; 
    g_fp.power(POWER_OFF);
  }
  else if (g_error_count > 7) {
    g_led.redFlash(2000, true);
  }
  else {
    // 指纹匹配错误，红灯亮0.5s
    g_led.redT(500);
  }
  daily_logger->info("Not matched, error_count={0:d}", g_error_count);
  daily_logger->flush();
}


// 创建信号并注册
void createSignal(int sig, void (*fun)(int sigNum, siginfo_t* info, void* myAction)) {
  struct sigaction action;

  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_SIGINFO;
  action.sa_sigaction = fun;

  if (sigaction(sig, &action, NULL) < 0) {
    daily_logger->error("Create signal error, sig_num={0:d}", sig);
  }
}


// 接收服务器进程发送的解锁信号
// 解锁
void handleUnlockSignal(int sigNum, siginfo_t* info, void* myAction) {
  DEBUG("Catch unlock siginal");
  g_is_locked = false;
}

// 接收服务器进程发送的加锁信号
// 加锁
void handleLockSignal(int sigNum, siginfo_t* info, void* myAction) {
  DEBUG("Catch lock siginal");
  g_is_locked = true;
}


