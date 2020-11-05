#include "door_client.h"
#include <wiringPi.h>
#include <wiringSerial.h>
#include "module/as608.h"
#include "../share/client/client.h"
#include "../share/notify/wx.h"
#include "../share/util/path.h"
#include "../share/log/logger.h"
#include "../share/module/led.h"

bool DoorClient::setup() {
    // 初始化wiringPi库
    if (-1 == wiringPiSetup()) {
        LError("WiringPi setup failed");
        return false;
    }

    // 初始化指纹识别模块
    if (!fpModule_.setup()) {
        LError("FpModule setup failed");
        return false;
    }

    // user管理
    if (!userManager_.setup()) {
        LError("UserManager setup failed");
        return false;
    }

    return true;
}

void DoorClient::run() {
    int pageID = 0;
    int score  = 0;
    errorCount_ = 0;

    while(!shouldQuit_) {
        /* 是否检测到手指 */
        if (!PS_DetectFinger(HIGH)) {
            delay(100);
            continue;
        }

        // 采集指纹并匹配
        // 最多尝试5次，最多等待30s
        if (fpModule_.match(5, 30000, pageID, score)) {
            LInfo("Matched, pageID={}, score={}", pageID, score);
            errorCount_ = 0;
            for (int i = 0; i < 5; ++i) {
                if (openTheDoor_(pageID, score)) {
                    LInfo("Open the door ok");
                    break;
                }
                else {
                    LError("Open the door failed: {}", i);
                }
            }
            delay(5000);
        }
        else {
            errorCount_ += 5;
            LError("error count: {}", errorCount_);
            if (errorCount_ > 30) {
                delay(1000 * 60 * 5);
            }
            else if (errorCount_ > 20) {
                delay(1000 * 60);
            }
            else if (errorCount_ > 10) {
                delay(1000 * 30);
            }
            else {
                delay(1000);
            }
        }
    } // end while
}

bool DoorClient::openTheDoor_(int pageID, int score) {
    Client client;
    if (!client.init()) {
        LError("Client init failed");
        return false;
    }

    /* 获取token */
    std::string token = userManager_.getToken(pageID);
    if (token.empty()) {
        LError("User was not registered");
        return false;
    }

    // 通知服务器开门
    client.emplaceParam("token", token);
    client.emplaceParam("from", "built-in");
    std::string url = "http://127.0.0.1:5001/door/open?" + client.getMergedParam();
    client.setUrl(url);
    if (!client.Get()) {
        LError("Request server failed");
        return false;
    }

   return true;
}

