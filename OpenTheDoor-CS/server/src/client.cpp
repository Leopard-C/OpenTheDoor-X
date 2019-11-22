#include "../include/client.h"
#include "../include/utils.h"


Client::Client() :
    curl_(nullptr), setuped_(false)
{

}

Client::~Client() {
  if (curl_)
    curl_easy_cleanup(curl_);
}

// 初始化
bool Client::setup() {
  curl_ = curl_easy_init();
  if (!curl_)
    return false;

  setuped_ = true;
  return true;
}

// 合并参数为字符串(key1=value1&key2=value2&key2=value3)
std::string Client::getMergedParam() {
  std::string mergedParam;
  for (auto it = params_.begin(); it != params_.end(); ++it) {
    if (it != params_.begin()) {
      mergedParam += "&";
    }
    mergedParam += it->first;
    mergedParam += "=";
    mergedParam += it->second;
  }
  return mergedParam;
}

// 发送Get请求
bool Client::Get() {
  std::string response;
  CURLcode res = curlGet_(url_, response);
  return res == CURLE_OK;
}

// 发送Post请求
bool Client::Post() {
  std::string response;
  CURLcode res = curlPost_(url_, getMergedParam(), response);
  return res == CURLE_OK;
}

// URL编码
void Client::encode(std::string& str) {
  char* encodedStr = curl_easy_escape(curl_, str.c_str(), str.size());
  str = std::string(encodedStr);
  curl_free(encodedStr);
}


// curl辅助函数
size_t Client::reqReply_(void* ptr, size_t size, size_t nmemb, void* stream) {
  std::string *str = (std::string*)stream;
  //DEBUG("---->reply");
  //DEBUG(*str);
  (*str).append((char*)ptr, size*nmemb);
  return size * nmemb;
}

// 发送Post请求(私有函数)
CURLcode Client::curlPost_(const std::string& url, const std::string& param, std::string& response) {
  addHeader_();
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


// 发送get请求(私有函数)
CURLcode Client::curlGet_(const std::string& url, std::string& response) {
  addHeader_();
  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, false);
  curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, false);
  curl_easy_setopt(curl_, CURLOPT_VERBOSE, 0);
  curl_easy_setopt(curl_, CURLOPT_READFUNCTION, NULL);
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, reqReply_);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (void *)&response);
  curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl_, CURLOPT_HEADER, 1);
  curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, 3);

  return curl_easy_perform(curl_);
}


// 添加请求头
void Client::addHeader_() {
  if (headers_.empty())
    return;

  std::string header;
  struct curl_slist* chunk = nullptr;
  for (auto it = headers_.begin(); it != headers_.end(); ++it) {
    header = it->first;
    header += ":";
    header += it->second;
    chunk = curl_slist_append(chunk, header.c_str());
  }
  curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, chunk);
}
