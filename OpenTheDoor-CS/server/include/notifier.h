#ifndef __NOTIFIER_H__
#define __NOTIFIER_H__

#include <string>

#include "./client.h"

enum NOTIFY_TYPE {
  NOTIFY_DOOR_OPEN = 0,
  NOTIFY_SYS_START,
  NOTIFY_SYS_STOP,
  NOTIFY_ALARM,
  NOTIFY_ERROR
};

// 宿舍成员ID
enum WHO {
  CJB = 0,
  PYH,
  JWC,
  SQL
};

WHO getWho(int fingerprintID);
std::string getName(WHO who);

class Notifier {
public:
  Notifier();
  ~Notifier();
public:
  bool setup();
  bool notifyQQ(std::string msg);   // not use const ref
  bool notifyWX(WHO who, std::string text, std::string desp = "");  // not use const ref
  bool notifyWXAll(const std::string& text, const std::string& desp);
  bool notifyAll(NOTIFY_TYPE type, const std::string& from, WHO whoDid = WHO::CJB);
  bool notifyMaster(NOTIFY_TYPE type);
  friend std::string getName(WHO who);

private:
  bool readJson_(const std::string& filename);
  void markdownString_(std::string& str);  // 将字符串转为Markdown格式(如两个换行符才表示换行)
  std::string getLog_(int line = 20);  // 获取日志的最后几行（默认最后20行）
private:
  bool  setuped_;
  int64_t groupID_;      // 寝室QQ群ID
  std::string sckeysWX_[4];  // 微信通知的秘钥
  std::string tokenQQ_;      // QQ通知的token
  Client clientQQ_;
  Client clientWX_;
  std::string protocolQQ_;
  std::string protocolWX_;
  std::string serverAddrQQ_;
  std::string serverAddrWX_;
  std::string coolqHttpAPI_; 
  int serverPortQQ_;
  int serverPortWX_;
};

#endif // __NOTIFIER_H__
