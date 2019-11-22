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
  bool Post();
  void clearParam();
  void setParamType(const std::string& type);
  void setParamWho(const std::string& who); 

private:
  bool readConfig_(const std::string& configFile);
  CURLcode curlPost_(const std::string& url, const std::string& param, std::string& response);
  static size_t reqReply_(void* ptr, size_t size, size_t nmemb, void* stream);
private:
  CURL* curl_;
  std::multimap<std::string, std::string> params_;
  std::string privateKey_;
  std::string serverAddr_;
  int serverPort_;
  bool setuped_;
};


#endif // __CLIENT_H__
