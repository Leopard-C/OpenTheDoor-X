#include "../include/client.h"

#include <iostream>
#include <fstream>
#include <map>

#include <jsoncpp/json/json.h>

#include "../include/utils.h"


Client::Client() :
    curl_(nullptr), serverPort_(0), setuped_(false)
{

}

Client::~Client() {
  curl_easy_cleanup(curl_);
}

bool Client::setup() {
  if (!readConfig_(utils::absolutePath("config/client.json")))
    return false;

  params_.emplace("from", "built-in");
  params_.emplace("pk", privateKey_);

  curl_ = curl_easy_init();
  if (!curl_)
    return false;

  setuped_ = true;
  return true;
}

void Client::clearParam() {
  params_.clear();
  params_.emplace("from", "built-in");
  params_.emplace("pk", privateKey_);
}

void Client::setParamType(const std::string& type) {
  params_.emplace("type", type);
}


void Client::setParamWho(const std::string& who) {
  params_.emplace("who", who);
}


// 发送Post请求
bool Client::Post() {
  std::string baseUrl = "https://";
  baseUrl += serverAddr_;
  baseUrl += ":";
  baseUrl += std::to_string(serverPort_);

  std::string param;
  for (auto it = params_.begin(); it != params_.end(); ++it) {
    if (it != params_.begin()) {
      param += "&";
    }
    param += it->first;
    param += "=";
    param += it->second;
  }

  DEBUG(baseUrl);

  std::string response;
  CURLcode res = curlPost_(baseUrl, param, response);

  return res == CURLE_OK;
}


// curl辅助函数
size_t Client::reqReply_(void* ptr, size_t size, size_t nmemb, void* stream) {
  std::string *str = (std::string*)stream;
  //DEBUG("---->reply");
  //DEBUG(*str);
  (*str).append((char*)ptr, size*nmemb);

  return size * nmemb;
}

CURLcode Client::curlPost_(const std::string& url, const std::string& param, std::string& response) {
  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str()); // url
  curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, param.length());
  curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, param.c_str());
  curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, false); // 
  curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, false); // Not verify cert & host
  curl_easy_setopt(curl_, CURLOPT_POST, 1);
  curl_easy_setopt(curl_, CURLOPT_VERBOSE, 0);
  curl_easy_setopt(curl_, CURLOPT_READFUNCTION, NULL);
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, reqReply_);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (void *)&response);
  curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1);   
  curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl_, CURLOPT_HEADER, 1);
  curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, 3); // 超时

  return curl_easy_perform(curl_);
}


// 读取配置文件
bool Client::readConfig_(const std::string& configFile) {
  Json::Reader reader;
  Json::Value  root;
  std::ifstream ifs;
  bool ret = false;

  do {
    ifs.open(configFile);
    if (!ifs.is_open()) {
      break;
    }

    if (!reader.parse(ifs, root, false)) {
      break;
    }

    if (root["private_key"].isNull() ||
        root["server_addr"].isNull() ||
        root["server_port"].isNull()) {
      break;
    }

    privateKey_ = root["private_key"].asString();
    serverAddr_ = root["server_addr"].asString();
    serverPort_ = root["server_port"].asInt();

    ret = true;
  } while (false);

  return ret;
}


