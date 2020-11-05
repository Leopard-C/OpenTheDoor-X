#pragma once
#include <string>
#include <vector>
#include "../json/json.h"

class WxNotifier {
public:
    bool setup();
    bool notifyRoot(const std::string& text, const std::string& desp = "");
    bool notifyAdmin(const std::string& text, const std::string& desp = "");
    bool notifyAll(const std::string& text, const std::string& desp = "");

private:
    bool readConfig_(const std::string& filename);
    void markdownString_(std::string& str);  // 将字符串转为Markdown格式(如两个换行符才表示换行)
    bool notify_(const std::string& sckey, std::string text, std::string desp = "");  // not use const ref
    void parseRootNode_(Json::Value& node);
    void parseAdminNode_(Json::Value& node);

private:
    std::vector<std::string> sckeysOfRoot_;
    std::vector<std::string> sckeysOfAdmin_;
};

