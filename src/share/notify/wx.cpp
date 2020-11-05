#include "wx.h"
#include "../log/logger.h"
#include "../client/client.h"

#include <ctime>
#include <fstream>

extern std::string g_cfgDir;


// 初始化配置
bool WxNotifier::setup() {
    if (!readConfig_(g_cfgDir + "/token.json")) {
        LError("Read config {}/token.json failed", g_cfgDir);
        return false;
    }
    return true;
}


// 微信通知
bool WxNotifier::notify_(const std::string& sckey, std::string text, std::string desp/* = "" */) {
    Client client;
    if (!client.init()) {
        LError("client init failed");
        return false;
    }

    std::string baseUrl = "https://sc.ftqq.com/";
    baseUrl += sckey;
    baseUrl += ".send?";

    // 拼接text
    client.encode(text); // URL编码
    std::string params = "text=";
    params += text;

    // 拼接desp(加上时间戳, 这样每条消息都是唯一的)
    desp = std::to_string(time(NULL)) + "\n" + desp;
    markdownString_(desp);
    client.encode(desp);  // URL编码
    params += "&desp=";
    params += desp;

    // 发送微信通知
    std::string url = baseUrl + params;
    client.setUrl(url);
    LDebug("Notify By Wechat: {}", url);

    return client.Get();
}

// 微信通知所有寝室成员
bool WxNotifier::notifyAll(const std::string& text, const std::string& desp) {
    bool flag = true;
    for (const auto& sckey : sckeysOfRoot_) {
        if (!notify_(sckey, text, desp)) {
            flag = false;
        }
    }
    for (const auto& sckey : sckeysOfAdmin_) {
        if (!notify_(sckey, text, desp)) {
            flag = false;
        }
    }
    LDebug("Notify WX ALL over");
    return flag;
}

// 通知root用户
bool WxNotifier::notifyRoot(const std::string& text, const std::string& desp) {
    bool flag = true;
    for (const auto& sckey : sckeysOfRoot_) {
        if (!notify_(sckey, text, desp)) {
            flag = false;
        }
    }
    return flag;
}

// 通知管理员
bool WxNotifier::notifyAdmin(const std::string& text, const std::string& desp) {
    bool flag = true;
    for (const auto& sckey : sckeysOfAdmin_) {
        if (!notify_(sckey, text, desp)) {
            flag = false;
        }
    }
    return flag;
}


// 读取json类型配置文件
bool WxNotifier::readConfig_(const std::string& cfgFile) {
    std::ifstream ifs(cfgFile);
    if (!ifs) {
        LError("Open file failed: {}", cfgFile);
        return false;
    }

    Json::Reader reader;
    Json::Value json;
    if (!reader.parse(ifs, json, false)) {
        LError("Parse file failed: {}", cfgFile);
        return false;
    }
    if (checkNull(json, "root", "admin")) {
        LError("Missing key:root/admin");
        return false;
    }

    /* root */
    parseRootNode_(json["root"]);

    /* admin */
    parseAdminNode_(json["admin"]);

    if (sckeysOfRoot_.empty()) {
        LError("There is no sckey of root user");
        return false;
    }

    LInfo("Read config file ok: {}", cfgFile);
    return true;
}

void WxNotifier::parseRootNode_(Json::Value& node) {
    int size = node.size();
    for (int i = 0; i < size; ++i) {
        Json::Value& subNode = node[i];
        if (checkNull(subNode, "sckey")) {
            LWarn("Missing key:sckey");
            continue;
        }
        std::string sckey = getStrVal(subNode, "sckey");
        if (!sckey.empty()) {
            sckeysOfRoot_.emplace_back(sckey);
        }
    }
}

void WxNotifier::parseAdminNode_(Json::Value& node) {
    int size = node.size();
    for (int i = 0; i < size; ++i) {
        Json::Value& subNode = node[i];
        if (checkNull(subNode, "sckey")) {
            LWarn("Missing key:sckey");
            continue;
        }
        std::string sckey = getStrVal(subNode, "sckey");
        if (!sckey.empty()) {
            sckeysOfAdmin_.emplace_back(sckey);
        }
    }
}

// 把字符串格式化为 markdown 型
// 两个换行表示真正换行
void WxNotifier::markdownString_(std::string& str) {
    for (std::string::size_type pos(0); pos != std::string::npos; pos += 2) {
        if ((pos = str.find("\n", pos)) != std::string::npos) {
            str.replace(pos, 1, "\n\n");
        }
        else {
            break;
        }
    }
}


