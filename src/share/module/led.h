#pragma once

#include <string>
#include <mutex>

class Led {
public:
    // @id: 应该全局唯一
    Led(int id, const std::string& color) : id_(id), color_(color) {}

    bool setup();

    void lightON();
    void lightOFF();
    void lightT(int ms, bool block = false);   // 点亮亮ms毫秒
    void flash(int ms, int freq, bool block = false);    // 闪烁ms毫秒

private:
    bool readConfig_(const std::string& cfgFile, const std::string& key);

private:
    int id_;
    int pin_ = -1;
    std::string color_;
    std::mutex mutex_;
};

