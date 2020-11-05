#include "path.h"
#include <unistd.h>
#include <limits.h>
#include <cstring>

namespace util {

/* 获取可执行文件路径 */
std::string getExePath() {
    char exePath[PATH_MAX] = { 0 };
    int n = readlink("/proc/self/exe", exePath, PATH_MAX);
    return std::string(exePath);
}

/* 获取可执行文件目录 */
std::string getExeDir() {
    std::string exePath = getExePath();
    auto pos = exePath.find_last_of("/");
    if (pos == std::string::npos) {
        return std::string();
    }
    else {
        return exePath.substr(0, pos);
    }
}

/* 获取项目目录                  */
/* $(projectDir)/bin/exefile  */
std::string getProjDir() {
    std::string exeDir = getExeDir();
    auto pos = exeDir.find_last_of("/");
    if (pos == std::string::npos) {
        return std::string();
    }
    else {
        return exeDir.substr(0, pos);
    }
}

} // namespace util

