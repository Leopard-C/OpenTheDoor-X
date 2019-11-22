#include "../include/notifier.h"
#include "../include/utils.h"
#include <jsoncpp/json/json.h>
#include <curl/curl.h>

#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>

using std::string;

#define NDEBUG

Notifier::Notifier(): 
  setuped_(false), curl_(nullptr), sckeys_{"", "", "", ""}
{
}

Notifier::~Notifier() {
  // 清理libcurl资源
  curl_easy_cleanup(curl_);
}


// 初始化配置
bool Notifier::setup() {
  if (!readJson_("config/notifier.json"))
    return false;

  // 初始化libcurl
  curl_ = curl_easy_init();
  if (!curl_)
    return false;

  setuped_ = true;
  return true;
}


// 通知某人 指定who
void Notifier::notify(Who who, string text, string desp/*=""*/) {
  if (!setuped_)
    return;

  if (text.size() == 0)
    return;

  string baseUrl = "https://sc.ftqq.com/";
  string params;
  string url;
  string response;

  // 拼接text
  markdownString_(text);
  char* encodedText = curl_easy_escape(curl_, text.c_str(), text.size());
  params += "text=";
  params += string(encodedText);
  curl_free(encodedText);

  // 拼接desp(时间戳)
  desp = std::to_string(time(0)) + "\n" + desp;
  markdownString_(desp);
  char* encodedDesp = curl_easy_escape(curl_, desp.c_str(), desp.size());
  params += "&desp=";
  params += string(encodedDesp);
  curl_free(encodedDesp);

  // 发送微信通知
  if (sckeys_[who].size() > 0) {
    baseUrl += sckeys_[who];
    baseUrl += ".send?";
    url = baseUrl + params;
    if (curl_get_(url, response) == CURLE_OK) {
      string msg = "Notify ";
      msg += getName_(who);
    }
  }
} // end function notify()


// 通知所有成员
void Notifier::notifyAll(Type type, Who whoDid /*=Who::CJB*/) {
  string msg;

  switch (type) {
  case Type::DoorOpen:
    msg = getName_(whoDid) + "开了门";
    break;
  case Type::DoorNotClosed:
    msg = "门没关，请关门";
    break;
  default:
    return ;
    //break;
  }

  if (sckeys_[Who::CJB].size() > 0)
    notify(Who::CJB, msg);
  if (sckeys_[Who::PYH].size() > 0)
    notify(Who::PYH, msg);
  if (sckeys_[Who::JWC].size() > 0)
    notify(Who::JWC, msg);
  if (sckeys_[Who::SQL].size() > 0)
    notify(Who::SQL, msg);
}


// 通知 系统管理员
void Notifier::notifyMaster(Type type) {
  switch (type) {
    case Type::Alarm:
      notify(Who::CJB, "Alarm", "Alarm");
      break;
    case Type::Error:
      notify(Who::CJB, "Error", "Error");
      break;
    default:
      break;
  }
}

// curl辅助函数
size_t req_reply_(void* ptr, size_t size, size_t nmemb, void* stream) {
#ifndef NDEBUG
  string *str = (string*)stream;
  std::cout << "----->reply" << std::endl;
  std::cout << *str << std::endl;
  (*str).append((char*)ptr, size*nmemb);
#endif

  return size * nmemb;
}


// 发送get请求
CURLcode Notifier::curl_get_(const string& url, string& response) {
  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str()); // url
  curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, false); // if want to use https
  curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, false); // set peer and host verify false
  curl_easy_setopt(curl_, CURLOPT_VERBOSE, 0);
  curl_easy_setopt(curl_, CURLOPT_READFUNCTION, NULL);
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, req_reply_);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (void *)&response);
  curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl_, CURLOPT_HEADER, 1);
  curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, 3); // set transport and time out time

  return curl_easy_perform(curl_);
}


// 读取json类型配置文件
bool Notifier::readJson_(const string& filename) {
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

    if (root["sckeys"].isNull()) {
      ifs.close();
      return false;
    }
    else {
      Json::Value sckeys = root["sckeys"];
      if (!sckeys["CJB"].isNull())
        sckeys_[Who::CJB] = sckeys["CJB"].asString();
      if (!sckeys["PYH"].isNull())
        sckeys_[Who::PYH] = sckeys["PYH"].asString();
      if (!sckeys["JWC"].isNull())
        sckeys_[Who::JWC] = sckeys["JWC"].asString();
      if (!sckeys["SQL"].isNull())
        sckeys_[Who::SQL] = sckeys["SQL"].asString();
    }
  }

  ifs.close();
  return true;
}


// 根据枚举类型 Who 获得姓名(汉字) 
std::string Notifier::getName_(Who who) {
  string name;
  switch (who) {
    case Who::CJB:  name = "XXX"; break;
    case Who::PYH:  name = "XXX"; break;
    case Who::JWC:  name = "XXX"; break;
    case Who::SQL:  name = "XXX"; break;
    default: name = ""; break;
  }

  return name;
}


// 把字符串格式化为 markdown 型
// 两个换行表示真正换行
void Notifier::markdownString_(string& str) {
  for (string::size_type pos(0); pos != string::npos; pos += 2) {
    if ((pos = str.find("\n", pos)) != string::npos)
      str.replace(pos, 1, "\n\n");
    else
      break;
  }
}

