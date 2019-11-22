#ifndef __MOTOR_H__
#define __MOTOR_H__

#include <string>

class Motor {
public:
  Motor();
  ~Motor();
public:
  bool setup();
  void setPin(int pin1, int pin2, int relayPin, int stopPinCW, int stopPinCCW);
  void rotateCW();
  void rotateCCW();

private:
  bool readJson_(const std::string& filename); // 读取配置文件
  void stop_(int stop_pin);   // 检测stopPin_为高电平，停止电机
  void reset_();  // 复位引脚电平全为低电平，使电机停转
private:
  int pin1_;
  int pin2_;
  int relayPin_; // 继电器pin，导通stopPin_与树莓派的连接
  int l298nRelayPin_; // 给L298n供电的继电器pin
  int stopPinCW_;  // 当检测到stopPin_为高电平时
  int stopPinCCW_;
private:
  bool setuped_;
  bool verbose_;  // 输出详细信息
};


#endif // __MOTOR_H__
