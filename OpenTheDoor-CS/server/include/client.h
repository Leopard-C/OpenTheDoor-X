#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <map>
#include <string>
#include <curl/curl.h>


class Client {
public:
  Client();
  ~Client();
public:
  bool setup();
  void setUrl(const std::string& url) { url_ = url; }
  void emplaceParam(const std::string& key, const std::string& value) { params_.emplace(key, value); }
  void emplaceParam(const std::string& key, int value) { params_.emplace(key, std::to_string(value)); }
  void emplaceHeader(const std::string& key, const std::string& value) { headers_.emplace(key, value); }
  std::string getMergedParam();
  void clearParam() { params_.clear(); }
  void encode(std::string& str);    // URL编码
  bool Get();
  bool Post();

private:
  CURLcode curlPost_(const std::string& url, const std::string& param, std::string& response);
  CURLcode curlGet_(const std::string &url, std::string& response);
  static size_t reqReply_(void* ptr, size_t size, size_t nmemb, void* stream);
  void addHeader_();
private:
  bool setuped_;
  CURL* curl_;
  std::string url_;
  std::multimap<std::string, std::string> params_;
  std::multimap<std::string, std::string> headers_;
};


#endif // __CLIENT_H__
