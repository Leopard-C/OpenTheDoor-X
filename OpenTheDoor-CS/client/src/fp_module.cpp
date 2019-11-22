#include "../include/fp_module.h"

#include <wiringSerial.h>
#include <wiringPi.h>
#include <fstream>
#include <iostream>
#include <string>

#include <jsoncpp/json/json.h>

#include "../include/as608.h"
#include "../include/utils.h"

extern int g_fd;
extern int g_verbose;
extern AS608 g_as608;


FpModule::FpModule() :
    relayPin_(0), setuped_(false), powered(false)
{
  g_fd = -1;
  g_verbose = 0;
}

FpModule::~FpModule() {
  power(POWER_OFF);
  if (g_fd > 0) {
    serialClose(g_fd);
    g_fd = -1;
  }
}


// 初始化配置
bool FpModule::setup() {
  if (!readJson_(utils::absolutePath("config/fp.json")))
    return false;

  // 临时供电
  // 每次供电，都会初始化AS608模块，检查是否初始化成功
  if (!power(POWER_ON))
    return false;
  power(POWER_OFF);

  setuped_ = true;
  return true;
}


// 采集指纹并匹配
bool FpModule::match(int& pageID, int& score) {
  if (!setuped_ || !powered)
    return false;

  bool ret = false;
  do {
    //if (!PS_GetImage())
    //  break;
    //if (!PS_GenChar(1))
    //  break;
    //if (!PS_Search(1, 0, 299, &pageID, &score))
    //  break;

    if (!PS_Identify(&pageID, &score))
      break;

    ret = true;
  } while (false);

  if (ret == false) {
    DEBUGF("err_code=%d, err_desc=%s\n", g_error_code, PS_GetErrorDesc());
  }

  return ret;
}


// 下载图像
bool FpModule::downImage() {
  if (!setuped_ || !powered)
    return false;

  power(POWER_ON);
  
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
    DEBUG("Open file error");
    return false;
  }

  if (!reader.parse(ifs, root)) {
    DEBUG("Parse error");
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
      DEBUG("Missing parameter");
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
