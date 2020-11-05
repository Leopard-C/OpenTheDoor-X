#include "fp_module.h"
#include "as608.h"
#include "../util/util.h"
#include "../../share/log/logger.h"
#include "../../share/json/json.h"

#include <wiringSerial.h>
#include <wiringPi.h>
#include <fstream>
#include <string>
#include <chrono>

extern int g_fd;
extern int g_verbose;
extern AS608 g_as608;
extern std::string g_cfgDir;


FpModule::FpModule() : relayPin_(-1), redLed_(2, "red") {
    g_fd = -1;
    g_verbose = 0;
}

FpModule::~FpModule() {
    powerOFF_();
}


// 初始化配置
bool FpModule::setup() {
    if (!readConfig_(g_cfgDir + "/as608.json")) {
        LError("Read file:{}/as608.json failed", g_cfgDir);
        return false;
    }

    if (!redLed_.setup()) {
        LError("Red led setup failed");
        return false;
    }

    pinMode(g_as608.detect_pin, INPUT);
    pinMode(relayPin_, OUTPUT);
    digitalWrite(relayPin_, LOW);

    // 临时供电
    // 每次供电，都会初始化AS608模块，检查是否初始化成功
    if (!powerON_()) {
        LError("FpModule power on failed");
        return false;
    }
    powerOFF_();

    return true;
}


// 采集指纹并匹配
// @maxFailCount: 最多尝试次数
// @maxBlockTimeMs: 最长等待时间 (毫秒)
bool FpModule::match(int maxFailCount, int maxBlockTimeMs, int& pageID, int& score) {
    if (!powerON_()) {
        LError("Power on failed");
        return false;
    }

    int timeConsumedUs = 0;
    int maxBlockTimeUs = maxBlockTimeMs * 1000;
    while (maxFailCount--) {
        if (match_(pageID, score, timeConsumedUs)) {
            break;
        }
        LError("match failed, timeConsumed(us):{}", timeConsumedUs);
        redLed_.lightT(500, false);
        maxBlockTimeUs -= timeConsumedUs;
        if (maxBlockTimeUs <= 0) {
            LError("Timeout");
            break;
        }
    }

    powerOFF_();
    return maxFailCount > 0 && maxBlockTimeUs > 0;
}

bool FpModule::matchOnce(int maxBlockTimeMs, int& pageID, int& score) {
    return match(1, maxBlockTimeMs, pageID, score);
}

bool FpModule::match_(int& pageID, int& score, int& timeConsumedUs) {
    auto start = std::chrono::system_clock::now();
    int ret = false;
    if (PS_GetImage()) {
        if (PS_GenChar(1)) {
            if (PS_Search(1, 0, 299, &pageID, &score)) {
                ret = true;
            }
        }
    }
    //int ret = PS_Identify(&pageID, &score);
    auto end = std::chrono::system_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    timeConsumedUs = dur.count();
    return ret;
}


// 给芯片供电
bool FpModule::powerON_() {
    // 打开继电器，供电
    digitalWrite(relayPin_, HIGH); 
    // 延迟一下，等待AS608模块就绪
    delay(500);

    if (g_fd > 0) {
        // won't get here
        serialClose(g_fd);
        g_fd = -1;
    }

    // 打开串口
    g_fd = serialOpen(serialFile_.c_str(), g_as608.baud_rate);
    if (g_fd <= 0) {
        LError("Open serial file failed");
        return false;
    }

    // 初始化AS608模块
    delay(500);
    if (!PS_Setup(g_as608.chip_addr, g_as608.password)) {
        LError("AS608 setup failed");
        return false;
    }

    return true;
}

// 给芯片断电
bool FpModule::powerOFF_() {
    // 关闭串口
    if (g_fd > 0) {
        serialClose(g_fd);
        g_fd = -1;     // Very very important ! ! !
    }

    // 关闭继电器，模块断电
    delay(100);
    digitalWrite(relayPin_, LOW);

    return true;
}

// 读取json类型配置文件
bool FpModule::readConfig_(const std::string& cfgFile) {
    std::ifstream ifs(cfgFile);
    if (!ifs) {
        LError("Open file:{} failed", cfgFile);
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(ifs, root)) {
        LError("Parse file:{} failed", cfgFile);
        return false;
    }

    if (checkNull(root, "verbose", "address", "password", "baud_rate",
                  "relay_pin", "detect_pin", "serial_file"))
    {
        LError("Missing param:verbose/address/...");
        return false;
    }

    // 芯片地址
    std::string addr = getStrVal(root, "address");
    if (addr.empty()) {
        LError("Invalid address");
        return false;
    }
    g_as608.chip_addr = util::hexStrToUInt(addr.c_str());
    // 是否有密码，以及密码是多少
    std::string pwd = getStrVal(root, "password");
    if (pwd.empty() || pwd == "none") {
        g_as608.has_password = 0;
    }
    else {
        g_as608.has_password = 1;
        g_as608.password = util::hexStrToUInt(pwd.c_str());
    }
    // 波特率
    g_as608.baud_rate = getIntVal(root, "baud_rate");
    // 检测手指的GPIO端口
    g_as608.detect_pin = getIntVal(root, "detect_pin");
    // 输出信息详细程度
    g_verbose = getIntVal(root, "verbose");

    // 继电器信号GPIO端口
    relayPin_ = getIntVal(root, "relay_pin");
    // 串口通信入口
    serialFile_ = getStrVal(root, "serial_file");

    return true;
}

