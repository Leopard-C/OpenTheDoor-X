#include <stdlib.h>
#include <signal.h>
#include "../share/log/logger.h"
#include "../share/util/path.h"
#include "door_client.h"

// 全局变量
DoorClient g_doorClient;

// 项目目录
std::string g_projDir = util::getProjDir();
// 数据目录
std::string g_dataDir = g_projDir + "/data";
// 配置文件目录
std::string g_cfgDir  = g_projDir + "/config";
// 日志目录
std::string g_logsDir = g_projDir + "/logs/client";


// 捕获Ctrl+C
void handleCtrlC(int num) {
    LWarn("Catch SIGINT");
    g_doorClient.quit();
}


// 主函数
int main(int argc, char* argv[]) {
    LInfo("============================================");
    signal(SIGINT, handleCtrlC);

    // 初始化
    if (!g_doorClient.setup()) {
        LError("Door client setup failed");
        return 1;
    }

    // 主循环
    g_doorClient.run();

    return 0;
}

