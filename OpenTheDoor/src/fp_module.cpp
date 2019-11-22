#include "../include/fp_module.h"
#include "../include/as608.h"
#include "../include/utils.h"

#include <jsoncpp/json/json.h>

#include <wiringSerial.h>
#include <wiringPi.h>
#include <fstream>

extern int g_fd;
extern int g_verbose;
extern AS608 g_as608;

FpModule::FpModule() :
  relayPin_(0), setuped_(false), powered(false)
{
  g_fd = -1;
  g_verbose = 1;
}

FpModule::~FpModule() {
  power(POWER_OFF);
  if (g_fd > 0)
    serialClose(g_fd);
}


// 初始化配置
bool FpModule::setup() {
  if (!readJson_("config/fp.json"))
    return false;

  // 临时供电，用于初始化AS608模块
  if (!power(POWER_ON))
    return false;

  power(POWER_OFF); // 关闭供电

  setuped_ = true;
  return true;
}


// 采集指纹并匹配
bool FpModule::match(int& pageID, int& score) {
  if (!setuped_ || !powered)
    return false;

  bool ret = false;

  //if (!PS_GetImage())
  //  return false;

  //if (!PS_GenChar(1))
  //  return false;

  //if (!PS_Search(1, 0, 299, &pageID, &score))
  //  return false;
  //else
  //  return true;

  if (PS_Identify(&pageID, &score))
    ret = true;
  else
    ret = false;

  return ret;
}


// 下载图像
bool FpModule::downImage() {
  if (!setuped_ || !powered)
    return false;

  power(POWER_ON);
  // sleep(1); // 暂停1s，等待初始化
  
  bool ret = false;

  // 构造文件名
  char filename[32] = { 0 };
  sprintf(filename, "image/%s.bmp", utils::getTime("%Y-%m-%d-%H_%M_%S"));

  if (PS_DownImage(filename)) {
    ret = true;
  }
  else {
    ret = false;
  }

  power(POWER_OFF);
  return false;
}


// 给芯片供电/断电
bool FpModule::power(POWER_STATUS status) {
  if (status == POWER_OFF) {
    // 关闭串口
    if (g_fd > 0) {
      serialClose(g_fd);
      g_fd = -1;     // Very very important ! ! !
    }

    // 关闭继电器，模块断电
    delay(100);
    digitalWrite(relayPin_, LOW);
    powered = false;

    return true;
  }
  else {
    if (!powered) {
      // 打开继电器，供电
      digitalWrite(relayPin_, HIGH); 
      delay(500);
    }

    // this will not happen
    if (g_fd > 0) {
      serialClose(g_fd);
      g_fd = -1;
    }

    // 打开串口
    if ((g_fd = serialOpen(serialFile_.c_str(), g_as608.baud_rate)) > 0) {
      delay(500);
      // 初始化AS608模块
      if (PS_Setup(g_as608.chip_addr, g_as608.password)) {
        powered = true;
        return true;
      }
    }

    powered = false;
    return false;
  }
}


// 读取json类型配置文件
bool FpModule::readJson_(const std::string& filename) {
  Json::Reader reader;
  Json::Value root;
  std::ifstream ifs(filename);
  if (!ifs.is_open()) {
    return false;
  }

  if (!reader.parse(ifs, root)) {
    ifs.close();
    return false;
  }
  else {
    if (root["verbose"].isNull() ||
        root["address"].isNull() ||
        root["password"].isNull() ||
        root["baud_rate"].isNull() ||
        root["relay_pin"].isNull() ||
        root["detect_pin"].isNull() ||
        root["serial_file"].isNull()) {
      ifs.close();
      return false;
    }
    else {
      // 芯片地址
      g_as608.chip_addr = utils::toUInt(root["address"].asString().c_str());
      // 是否有密码，以及密码是多少
      if (root["password"].asString() == "none") {
        g_as608.has_password = 0;
      }
      else {
        g_as608.has_password = 1;
        g_as608.password = utils::toUInt(root["password"].asString().c_str());
      }
      // 波特率
      g_as608.baud_rate = root["baud_rate"].asInt();
      // 继电器信号GPIO端口
      relayPin_ = root["relay_pin"].asInt();
      pinMode(relayPin_, OUTPUT);
      digitalWrite(relayPin_, LOW);
      // 检测手指的GPIO端口
      g_as608.detect_pin = root["detect_pin"].asInt();
      pinMode(g_as608.detect_pin, INPUT);
      // 串口通信入口
      serialFile_ = root["serial_file"].asString(); 

      // 输出信息详细程度
      g_verbose = root["verbose"].asInt();
    }
  }
  
  ifs.close();
  return true;
}
