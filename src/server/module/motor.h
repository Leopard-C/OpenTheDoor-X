#ifndef __MOTOR_H__
#define __MOTOR_H__

#include <string>
#include <mutex>


class Motor {
public:
    ~Motor();
public:
    bool setup();

    bool rotateCW();
    bool rotateCCW();

private:
    bool readConfig_(const std::string& cfgFile); // 读取配置文件
    void stop_(int stop_pin);   // 检测stopPin_为高电平，停止电机
    void reset_();              // 复位引脚电平全为低电平，使电机停转

private:
    int pin1_ = -1;
    int pin2_ = -1;
    int stopPinCW_  = -1;  // 普通开关
    int stopPinCCW_ = -1;  // 光电开关
    std::mutex mutex_;
};


#endif // __MOTOR_H__
