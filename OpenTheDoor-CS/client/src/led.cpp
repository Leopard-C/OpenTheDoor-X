#include "../include/led.h"

#include <wiringPi.h>
#include <fstream>
#include <mutex>
#include <string>

#include <jsoncpp/json/json.h>

#include "../include/utils.h"


Led::Led() :
  setuped_(false),
  redPin_(0), greenPin_(0), flashFreq_(5),
  pool_(2)
{
  
}

Led::~Led() { }


// 初始化配置
bool Led::setup() {
  if (!readJson_(utils::absolutePath("config/led.json")))
    return false;

  pinMode(redPin_, OUTPUT);
  pinMode(greenPin_, OUTPUT);

  setuped_ = true;
  return true;
}


// 点亮/熄灭 红灯
void Led::red(LED_STATUS status) {
  if (!setuped_)
    return ;

  if (this->redMutex_.try_lock()) {
    if (status == LED_OFF)
      digitalWrite(redPin_, LOW);
    else
      digitalWrite(redPin_, HIGH);
    this->redMutex_.unlock();
  }
}

// 红灯亮ms毫秒
void Led::redT(int ms, bool block/* = false*/) {
  if (!setuped_)
    return;

  auto lambdaRedT = [this, ms]{
      if (this->redMutex_.try_lock()) {
        digitalWrite(redPin_, HIGH);
        delay(ms);
        digitalWrite(this->redPin_, LOW);
        this->redMutex_.unlock();
      }
  };

  if (block)
    lambdaRedT();
  else
    pool_.enqueue(lambdaRedT);
}


// 点亮/熄灭 绿灯
void Led::green(LED_STATUS status) {
  if (!setuped_)
    return ;

  if (this->greenMutex_.try_lock()) {
    if (status == LED_OFF)
      digitalWrite(greenPin_, LOW);
    else
      digitalWrite(greenPin_, HIGH);
    this->greenMutex_.unlock();
  }
}

// 绿灯亮ms毫秒
void Led::greenT(int ms, bool block/* = false*/) {
  if (!setuped_)
    return;

  auto lambdaGreenT = [this, ms]{
      if (this->greenMutex_.try_lock()) {
        digitalWrite(greenPin_, HIGH);
        delay(ms);
        digitalWrite(this->greenPin_, LOW);
        this->greenMutex_.unlock();
      }
  };

  if (block)
    lambdaGreenT();
  else
    pool_.enqueue(lambdaGreenT);
}


// 闪烁(私有函数)
void Led::flash_(int pin, int ms) {
  int perDelay = 1000 / 2 / flashFreq_;
  int count = ms / perDelay;
  for (int i = 0; i < count; ++i) {
    digitalWrite(pin, HIGH);
    delay(perDelay);
    digitalWrite(pin, LOW);
    delay(perDelay);
  }
}


// 红灯闪烁
// @block: 是否阻塞
void Led::redFlash(int ms, bool block /*=false*/) {
  if (!setuped_)
    return;

  DEBUG("Red Flash");

  if (block) {
    if (this->redMutex_.try_lock()) {
      flash_(this->redPin_, ms);
      this->redMutex_.unlock();
    }
  }
  else {
    pool_.enqueue([ms, this]{
        if (this->redMutex_.try_lock()) {
          flash_(this->redPin_, ms);
          this->redMutex_.unlock();
        }
      });
  }
}


// 绿灯闪烁
void Led::greenFlash(int ms, bool block/* = false*/) {
  if (!setuped_)
    return ;

  if (block) {
    if (this->greenMutex_.try_lock()) {
      flash_(this->greenPin_, ms);
      this->greenMutex_.unlock();
    }
  }
  else {
    pool_.enqueue([ms, this]{
        if (this->greenMutex_.try_lock()) {
          flash_(this->greenPin_, ms);
          this->greenMutex_.unlock();
        }
      });
  }
}


// 读取json类型配置文件
bool Led::readJson_(const std::string& filename) {
  Json::Reader reader;
  Json::Value  root;
  
  std::ifstream ifs(filename);
  if (!ifs.is_open())
    return false;

  bool ret = false;

  do {
    if (!reader.parse(ifs, root))
      break;
    else {
      if (root["red_pin"].isNull() || 
          root["green_pin"].isNull() ||
          root["flash_freq"].isNull())
        break;
      redPin_ = root["red_pin"].asInt();
      greenPin_ = root["green_pin"].asInt();
      flashFreq_ = root["flash_freq"].asInt();
    }
  } while (false);

  ifs.close();
  return true;
}
