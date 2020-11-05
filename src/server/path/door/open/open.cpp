#include "open.h"
#include "../../base.h"
#include "../../../module/motor.h"
#include "../../../../share/module/led.h"

#include <unistd.h>
#include <mutex>
#include <thread>

extern Led   g_greenLed;
extern Motor g_motor;
extern WxNotifier g_wxNotifier;


static std::mutex s_mutex;

/* 通知所有成员 */
static bool notify_door_opened(const std::string& from, const std::string& user) {
    std::string msg = user + "通过";
    if (from == "built-in") {
        msg += "指纹";
    }
    else {
        msg += from;
    }
    msg += "开了门";
    LDebug("{}", msg);
    return g_wxNotifier.notifyAll(msg);
}

/* 后台线程：执行开门过程 */
static void open_the_door_thread(const std::string& from, const std::string& user) {
    if (!s_mutex.try_lock()) {
        LWarn("Motor is busy");
        return;
    }

    // 开门
    g_motor.rotateCW();

    // 判断开门渠道：C软件、手机APP、指纹
    if (from == "pc") {
        LInfo("From PC. {} 开了门", user);
        g_greenLed.lightT(2000, false);
        sleep(8);
    }
    else if (from == "app") {
        LInfo("From APP. {} 开了门", user);
        g_greenLed.lightT(2000, false);
        sleep(5);
    }
    // 树莓派内置clien的请求（即通过指纹开门）
    else if (from == "built-in") {
        LInfo("From built-in. {} 开了门", user);
        //g_greenLed.lightT(2000, false);
        sleep(5);
    }
    else if (from == "web") {
        LInfo("From Web. {} 开了门", user);
        g_greenLed.lightT(2000, false);
        sleep(5);
    }
    else {
        // won't get here
        LInfo("From UNKNOWN way:{}. {} 开了门", from, user);
        g_greenLed.lightT(2000, false);
        sleep(5);
    }

    // 关门
    g_motor.rotateCCW();
    LInfo("Door opened");

    // attention!
    s_mutex.unlock();

    // 通知所有寝室成员
    if (!notify_door_opened(from, user)) {
        LError("Notify dorm members failed");
    }
}


/* uri回调函数 */
void cb_door_open(Request& req, Response& res) {
    LTrace("Handle door_open");
    Json::Value root, data;

    // 这里展开时，会定义一个 `_user` 变量, 即开门者的姓名
    CHECK_TOKEN_EX(LV_ROOT | LV_ADMIN | LV_GUEST);

    // @from 请求来源：树莓派内置client、Windows软件请求、手机
    // @who  请求开门的宿舍成员id
    CHECK_PARAM_STR(from);

    // 外部请求
    if (from != "pc" && from != "built-in" && from != "app" && from != "web") {
        RETURN_INVALID_PARAM(from);
    }

    // 后台执行开门过程
    std::thread t(open_the_door_thread, from, _user);
    t.detach();

    RETURN_OK();
}

