#pragma once

#include <map>
#include <string>
#include <curl/curl.h>


class Client {
public:
  Client();
  ~Client();
public:
  bool init();
  void setUrl(const std::string& url) { url_ = url; }
  void emplaceParam(const std::string& key, const std::string& value) { params_.emplace(key, value); }
  void emplaceParam(const std::string& key, const char* value) { params_.emplace(key, value); }
  template<typename T>
  void emplaceParam(const std::string& key, const T& value) { params_.emplace(key, std::to_string(value)); }
  void emplaceHeader(const std::string& key, const std::string& value) { headers_.emplace(key, value); }
  std::string getMergedParam();
  void clearParams() { params_.clear(); }
  void clearHeaders() { headers_.clear(); }
  void encode(std::string& str);    // URL编码
  bool Get();

private:
  CURLcode curlGet_(const std::string &url, std::string& response);
  static size_t reqReply_(void* ptr, size_t size, size_t nmemb, void* stream);
  void addHeaders_();

private:
  CURL* curl_;
  std::string url_;
  std::multimap<std::string, std::string> params_;
  std::multimap<std::string, std::string> headers_;
};

