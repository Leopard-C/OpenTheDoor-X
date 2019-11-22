#include "../include/motor.h"

#include <wiringPi.h>
#include <fstream>

#include <jsoncpp/json/json.h>

#include "../include/utils.h"


Motor::Motor() :
  pin1_(0), pin2_(0), relayPin_(0), l298nRelayPin_(0),
  stopPinCW_(0), stopPinCCW_(0),
  setuped_(false), verbose_(false)
{
}

Motor::~Motor() {
  reset_();
}


// 初始化
bool Motor::setup() {
  if (!readJson_(utils::absolutePath("config/motor.json")))
    return false;
  
  pinMode(pin1_, OUTPUT);
  pinMode(pin2_, OUTPUT);
  pinMode(relayPin_, OUTPUT);
  pinMode(l298nRelayPin_, OUTPUT);
  pinMode(stopPinCW_, INPUT);
  pinMode(stopPinCCW_, INPUT);
  pullUpDnControl(stopPinCW_, PUD_DOWN);
  pullUpDnControl(stopPinCCW_, PUD_DOWN);

  digitalWrite(relayPin_, HIGH);
  digitalWrite(pin1_, LOW);
  digitalWrite(pin2_, LOW);

  // 给L298N供电（固态继电器, 低电压接通）
  digitalWrite(l298nRelayPin_, LOW);
  delay(20);
  digitalWrite(l298nRelayPin_, HIGH);

  setuped_ = true;
  return true;
}


// 顺时针旋转
void Motor::rotateCW() {
  if (!setuped_)
    return;

  // 开始转动
  digitalWrite(pin1_, HIGH);
  digitalWrite(pin2_, LOW);

  // 停止转动
  stop_(stopPinCW_);
}


// 逆时针旋转
void Motor::rotateCCW() {
  if (!setuped_)
    return;

  // 打开继电器，开始检测信号
  digitalWrite(relayPin_, LOW);

  // 开始转动
  digitalWrite(pin1_, LOW);
  digitalWrite(pin2_, HIGH);

  // 停止转动
  stop_(stopPinCCW_);
}


// 停止电机转动
void Motor::stop_(int stop_pin) {
  int count = 0;
  while (true) {
    if (digitalRead(stop_pin) == HIGH) {
      reset_();
      break;
    }
    delay(100);
    count++;
    if (count > 50) {  // 最长等待5s
      reset_();
      break;
    }
  }
}


// 重置GPIO引脚
void Motor::reset_() {
  digitalWrite(pin1_, LOW);
  digitalWrite(pin2_, LOW);
  digitalWrite(relayPin_, HIGH); // 固态继电器，低电平接通，高电平断开
}


// 读取配置文件
bool Motor::readJson_(const std::string& filename) {
  Json::Reader reader;
  Json::Value  root;
  std::ifstream ifs(filename);
  if (!ifs.is_open())
    return false;

  if (!reader.parse(ifs, root)) {
    ifs.close();
    return false;
  }
  else {
    if (root["verbose"].isNull() ||
        root["motor_pin1"].isNull() ||
        root["motor_pin2"].isNull() ||
        root["stop_pin_cw"].isNull() ||
        root["stop_pin_ccw"].isNull() ||
        root["l298n_relay_pin"].isNull() ||
        root["relay_pin"].isNull()) {
      ifs.close();
      return false;
    }
    else {
      pin1_ = root["motor_pin1"].asInt();
      pin2_ = root["motor_pin2"].asInt();
      relayPin_ = root["relay_pin"].asInt();
      l298nRelayPin_ = root["l298n_relay_pin"].asInt();
      stopPinCW_ = root["stop_pin_cw"].asInt();
      stopPinCCW_ = root["stop_pin_ccw"].asInt();
      verbose_ = root["verbose"].asBool();

      ifs.close();
      return true;
    }
  }

}
