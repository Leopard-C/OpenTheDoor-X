#include "../include/notifier.h"
#include "../include/utils.h"

#include <cstring>
#include <ctime>
#include <fstream>
#include <unistd.h>
#include <iostream>

#include <curl/curl.h>
#include <jsoncpp/json/json.h>

using std::string;


Notifier::Notifier(): 
    setuped_(false), sckeysWX_{"", "", "", ""}
{

}


Notifier::~Notifier() {
  DEBUG("Destroy Notifier");
}


// 初始化配置
bool Notifier::setup() {
  if (!readJson_(utils::absolutePath("config/notifier.json")))
    return false;
  if (!clientQQ_.setup()) 
    return false;
  if (!clientWX_.setup())
    return false;

  setuped_ = true;
  return true;
}


// QQ群通知
bool Notifier::notifyQQ(string msg) {
  if (!setuped_ || msg.empty()) {
    DEBUGF("Notify QQ terminate: msg=%s\n", msg.c_str());
    return false;
  }

  std::string baseUrl = protocolQQ_;
  baseUrl += "://";
  baseUrl += serverAddrQQ_;
  baseUrl += ":";
  baseUrl += std::to_string(serverPortQQ_);
  baseUrl += "/";
  baseUrl += coolqHttpAPI_;
  baseUrl += "?";

  clientQQ_.emplaceHeader("Authorization", "Token " + tokenQQ_);

  clientQQ_.clearParam();
  clientQQ_.emplaceParam("group_id", std::to_string(groupID_));
  clientQQ_.encode(msg);  // URL编码
  clientQQ_.emplaceParam("message", msg);
  clientQQ_.emplaceParam("autoescape", "false");

  baseUrl += clientQQ_.getMergedParam(); 
  clientQQ_.setUrl(baseUrl);
  DEBUG(baseUrl);

  return clientQQ_.Get();
}

// 微信通知
bool Notifier::notifyWX(WHO who, std::string text, std::string desp/* = "" */) {
  if (!setuped_ || text.empty() || sckeysWX_[who].empty()) {
    DEBUGF("Notify WX terminate: text=%s, desp=%s, who=%d, scks(who)=%s\n", text, desp.c_str(), who, sckeysWX_[who].c_str());
    return false;
  }

  std::string baseUrl = protocolWX_;
  baseUrl += "://";
  baseUrl += serverAddrWX_;
  baseUrl += ":";
  baseUrl += std::to_string(serverPortWX_);
  baseUrl += "/";
  baseUrl += sckeysWX_[who];
  baseUrl += ".send?";

  // 拼接text
  markdownString_(text);
  clientWX_.encode(text); // URL编码
  std::string params = "text=";
  params += text;

  // 拼接desp(加上时间戳, 这样每条消息都是唯一的)
  desp = std::to_string(time(0)) + "\n" + desp;
  markdownString_(desp);
  clientWX_.encode(desp);  // URL编码
  params += "&desp=";
  params += desp;

  // 发送微信通知
  std::string url = baseUrl + params;
  clientWX_.setUrl(url);
  DEBUG(url);

  return clientWX_.Get();
}

// 微信通知所有寝室成员
bool Notifier::notifyWXAll(const string& text, const string& desp) {
  bool flag = true;
  if (!sckeysWX_[WHO::CJB].empty())
    if (!notifyWX(WHO::CJB, text, desp))
      flag = false;
  DEBUG("Notify CJB over");
  if (!sckeysWX_[WHO::PYH].empty())
    if (!notifyWX(WHO::PYH, text, desp))
      flag = false;
  DEBUG("Notify PYH over");
  if (!sckeysWX_[WHO::JWC].empty())
    if (!notifyWX(WHO::JWC, text, desp))
      flag = false;
  DEBUG("Notify JWC over");
  if (!sckeysWX_[WHO::SQL].empty())
    if (!notifyWX(WHO::SQL, text, desp))
      flag = false;

  DEBUG("Notify WX ALL over");
  return flag;
}


// QQ、微信通知所有人
bool Notifier::notifyAll(NOTIFY_TYPE type, const string& from, WHO whoDid/* = WHO::CJB*/) {
  bool ret = true;

  switch(type) {
    default:
      break;
    case NOTIFY_DOOR_OPEN: {
      std::string msg = getName(whoDid) + "通过";
      if (from == "built-in")
        msg += "指纹";
      else
        msg += from;
      msg += "开了门!";
      DEBUG(msg);
      if (!notifyWXAll(msg, ""))
        ret = false;
      if (!notifyQQ(msg))
        ret = false;
      break;
    }
  }

  return ret;
}


// 通知管理员
bool Notifier::notifyMaster(NOTIFY_TYPE type) {
  DEBUG("Notifying mater");
  string log = getLog_(20);   // 获取最近20条日志
  bool ret = true;

  switch(type) {
    default:
      break;
    case NOTIFY_ALARM:
      ret = notifyWX(WHO::CJB, "Alarm", log);
      break;
    case NOTIFY_ERROR:
      ret = notifyWX(WHO::CJB, "Error", log);
      break;
    case NOTIFY_SYS_START:
      ret = notifyWX(WHO::CJB, "SystemStartSuccessfully", log);
      break;
    case NOTIFY_SYS_STOP:
      ret = notifyWX(WHO::CJB, "SystemStopSuccessfully", log);
      break;
  }

  if (!ret)
    DEBUG("Notify master error");

  return ret;
}


// 读取json类型配置文件
bool Notifier::readJson_(const string& filename) {
  std::ifstream ifs(filename);
  if (!ifs.is_open()) {
    return false;
  }

  Json::Reader reader;
  Json::Value root;
  if (!reader.parse(ifs, root)) {
    ifs.close();
    return false;
  }
  else {
    if (root["sckeys_wx"].isNull() ||
        root["token_qq"].isNull() ||
        root["qq_group_id"].isNull() ||
        root["protocol_qq"].isNull() ||
        root["protocol_wx"].isNull() ||
        root["coolq_http_api"].isNull() ||
        root["server_addr_qq"].isNull() ||
        root["server_port_qq"].isNull() ||
        root["server_addr_wx"].isNull() ||
        root["server_port_wx"].isNull()) {
      DEBUG("Missing configuration item");
      ifs.close();
      return false;
    }
    else {
      protocolQQ_ = root["protocol_qq"].asString();
      protocolWX_ = root["protocol_wx"].asString();
      serverAddrQQ_ = root["server_addr_qq"].asString();
      serverPortQQ_ = root["server_port_qq"].asInt();
      serverAddrWX_ = root["server_addr_wx"].asString();
      serverPortWX_ = root["server_port_wx"].asInt();
      coolqHttpAPI_ = root["coolq_http_api"].asString();
      groupID_ = root["qq_group_id"].asInt64();
      tokenQQ_ = root["token_qq"].asString();

      Json::Value sckeys = root["sckeys_wx"];
      if (!sckeys["CJB"].isNull())
        sckeysWX_[WHO::CJB] = sckeys["CJB"].asString();
      if (!sckeys["PYH"].isNull())
        sckeysWX_[WHO::PYH] = sckeys["PYH"].asString();
      if (!sckeys["JWC"].isNull())
        sckeysWX_[WHO::JWC] = sckeys["JWC"].asString();
      if (!sckeys["SQL"].isNull())
        sckeysWX_[WHO::SQL] = sckeys["SQL"].asString();
    }
  }

  ifs.close();
  return true;
}


// 获取今天的日志最后几行
std::string Notifier::getLog_(int line/*=20*/) {
  // 获取日志路径
  char logFilename[128] = { 0 };
  sprintf(logFilename, "%s%s.txt", utils::absolutePath("logs/daily_").c_str(), utils::getTime("%Y-%m-%d"));
  
  // 使用tail命令得到log日志的最后几行，
  string cmd = "tail ";
  cmd += string(logFilename);
  cmd += " -n ";
  cmd += std::to_string(line);
  DEBUG(cmd);

  // 执行命令并获取返回值
  string ret;
  char* buf = new char[100*line];
  memset(buf, 0, 100*line);  // ！！！
  if (utils::execShell(cmd.c_str(), buf, 100*line)) {
    ret = string(buf);
  }
  else {
    ret = "";
  }

  delete[] buf;
  return ret;
}


// 根据枚举类型 Who 获得姓名(汉字) 
std::string getName(WHO who) {
  string name;
  switch (who) {
    case WHO::CJB:  name = "XXX"; break;
    case WHO::PYH:  name = "XXX"; break;
    case WHO::JWC:  name = "XXX"; break;
    case WHO::SQL:  name = "XXX"; break;
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


