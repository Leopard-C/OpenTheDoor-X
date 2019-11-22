#ifndef __NOTIFIER_H__
#define __NOTIFIER_H__

#include <string>
#include <curl/curl.h>


enum Type {
  DoorOpen = 0,
  DoorNotClosed,
  Alarm,
  Error
};

// 宿舍成员ID
enum Who {
  CJB = 0,
  PYH,
  JWC,
  SQL
};

class Notifier {
public:
  Notifier();
  ~Notifier();
public:
  bool setup();
  void notify(Who who, std::string text, std::string desp="");
  void notifyAll(Type type, Who whoDid = Who::CJB);
  void notifyMaster(Type type);

private:
  friend size_t req_reply_(void* ptr, size_t size, size_t nmemb, void* stream);
  CURLcode curl_get_(const std::string &url, std::string& response);
  bool readJson_(const std::string& filename);
  void markdownString_(std::string& str);
  std::string getLog_(int line = 20);  // 获取日志的最后几行（默认最后20行）
  std::string getName_(Who who);
private:
  bool  setuped_;
  CURL* curl_;
  std::string sckeys_[4];
};

#endif // __NOTIFIER_H__
