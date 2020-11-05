#include "led.h"
#include "../json/json.h"
#include "../log/logger.h"

#include <wiringPi.h>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>

extern std::string g_cfgDir;


// 初始化配置
bool Led::setup() {
    std::string key = std::to_string(id_) + "_" + color_;
    if (!readConfig_(g_cfgDir + "/led.json", key)) {
        LError("Read config {}/led.json failed", g_cfgDir);
        return false;
    }
    pinMode(pin_, OUTPUT);
    return true;
}


// 点亮
void Led::lightON() {
    LDebug("light on led. id:{}, color:{}", id_, color_);
    if (mutex_.try_lock()) {
        digitalWrite(pin_, HIGH);
        mutex_.unlock();
    }
}

//熄灭
void Led::lightOFF() {
    LDebug("light off led. id:{}, color:{}", id_, color_);
    if (mutex_.try_lock()) {
        digitalWrite(pin_, LOW);
        mutex_.unlock();
    }
}

// 点亮ms毫秒
void Led::lightT(int ms, bool block/* = false*/) {
    LDebug("Light {}. time:{}, block:{}", color_, ms, block);
    std::thread t([this, ms]{
        if (this->mutex_.try_lock()) {
            digitalWrite(this->pin_, HIGH);
            delay(ms);
            digitalWrite(this->pin_, LOW);
            this->mutex_.unlock();
        }
    });
    if (block) {
        t.join();
    }
    else {
        t.detach();
    }
}


// 闪烁
// @ms: 毫秒数
// @freq：频率，每秒几次
// @block: 是否阻塞
void Led::flash(int ms, int freq, bool block /*=false*/) {
    LDebug("Flash {}. time:{}, freq:{}, block:{}", color_, ms, freq, block);
    std::thread t([ms, freq, this]{
        if (this->mutex_.try_lock()) {
            int perDelay = 1000 / 2 / freq;
            int count = ms / perDelay;
            for (int i = 0; i < count; ++i) {
                digitalWrite(this->pin_, HIGH);
                delay(perDelay);
                digitalWrite(this->pin_, LOW);
                delay(perDelay);
            }
            this->mutex_.unlock();
        }
    });
    if (block) {
        t.join();
    }
    else {
        t.detach();
    }
}

// 读取json类型配置文件
bool Led::readConfig_(const std::string& cfgFile, const std::string& key) {
    std::ifstream ifs(cfgFile);
    if (!ifs) {
        LError("Open file {} failed", cfgFile);
        return false;
    }

    Json::Reader reader;
    Json::Value  root;
    if (!reader.parse(ifs, root)) {
        LError("Parse file {} failed", cfgFile);
        return false;
    }

    if (checkNull(root, key)) {
        LError("Missing key:{}", key);
        return false;
    }

    pin_ = getIntVal(root, key);

    return true;
}

