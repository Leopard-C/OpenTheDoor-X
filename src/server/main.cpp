#include <stdlib.h>
#include <signal.h>
#include <wiringPi.h>

#include "../share/notify/wx.h"
#include "../share/util/path.h"
#include "../share/log/logger.h"
#include "../share/json/json.h"
#include "../share/module/led.h"
#include "module/motor.h"
#include "server/http_server.h"
#include "token/token_manager.h"

#include "path/door/open/open.h"
#include "path/server/stop/stop.h"
#include "path/token/new/new.h"

// 项目目录
std::string g_projDir = util::getProjDir();
// 数据目录
std::string g_dataDir = g_projDir + "/data/server";
// 配置文件目录
//   服务器客户端公用一个目录
std::string g_cfgDir  = g_projDir + "/config";
// 日志目录
std::string g_logsDir = g_projDir + "/logs/server";

Led          g_greenLed(1, "green");
Motor        g_motor;
WxNotifier   g_wxNotifier;
TokenManager g_tokenMgr;
HttpServer   g_svr;

bool setup();
void handleCtrlC(int num);


/********************************* main() begin **********************/

int main(int argc, char* argv[])
{
    LInfo("============================================");

    // 捕获 ctrl+c
    signal(SIGINT, handleCtrlC);

    // 随机种子
    srand(time(NULL));

    // 初始化配置
    if (!setup()) {
        LError("Setup failed");
        return 1;
    }

    ThreadPool tp;
    tp.set_pool_size(2);

    g_svr.set_thread_pool(&tp);
    g_svr.add_bind_ip("127.0.0.1");
    g_svr.set_port(5001);

    /* 注册URI回调函数 */
    g_svr.add_mapping("/server/stop", cb_server_stop, GET_METHOD);
    g_svr.add_mapping("/door/open",   cb_door_open,   GET_METHOD);
    g_svr.add_mapping("/token/new",   cb_token_new,   GET_METHOD);

    // 告知管理员，程序正常启动
    if (!g_wxNotifier.notifyRoot("DoorServerStarted")) {
        LError("Notify master failed");
    }

    // 开启监听客户端
    g_svr.start_sync();

    // 告知管理员，程序正常退出
    if (!g_wxNotifier.notifyRoot("DoorServerStopped")) {
        LError("Notify master failed");
    }

    LInfo("Server stopped");
    spdlog::shutdown();
    return 0;
}

/********************************* main() end *************************/

// 捕获Ctrl+C
void handleCtrlC(int num) {
    LWarn("Catch Ctrl+C");
    g_svr.stop();
}

// 初始化配置
bool setup() {
    // 初始化wiringPi库
    if (-1 == wiringPiSetup()) {
        LError("WiringPi setup failed");
        return false;
    }

    // 初始化电机
    if (!g_motor.setup()) {
        LError("Motor setup failed");
        return false;
    }

    // 初始化Led
    if (!g_greenLed.setup()) {
        LError("Led setup failed");
        return false;
    }

    // 初始化通知器
    if (!g_wxNotifier.setup()) {
        LError("WxNotifier setup failed");
        return false;
    }

    // 初始化token管理器
    if (!g_tokenMgr.setup()) {
        LError("TokenManager setup failed");
        return false;
    }

    return true;
}

