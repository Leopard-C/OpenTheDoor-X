#include "token_manager.h"
#include "../../share/log/logger.h"
#include <ctime>
#include <fstream>

extern std::string g_cfgDir;


bool TokenManager::setup() {
    if (!readConfig_(g_cfgDir + "/token.json")) {
        LError("Read config {}/token.json failed", g_cfgDir);
        return false;
    }
    return true;
}

/* 生成临时token */
std::string TokenManager::newToken(const std::string& parentTokenValue,
                                   const std::string& user,
                                   int max_times, int seconds)
{
    std::lock_guard<std::mutex> lck(mutex_);
    std::string parentTokenUser;
    if (LV_INVALID == checkTokenWithNoLock_(parentTokenValue, LV_ROOT | LV_ADMIN, &parentTokenUser)) {
        return "";
    }
    Token token;
    token.level = LV_GUEST;
    token.parent = parentTokenValue;
    token.user = user;
    token.remain_times = max_times;
    token.ttl = time(NULL) + seconds;
    token.value = genRandomToken_();
    /* 防止重复 */
    while (tokens_.find(token.value) != tokens_.end()) {
        token.value = genRandomToken_();
    }
    tokens_.emplace(token.value, token);
    LInfo("New Token, level:{}, user:{} parent:{}, max_times:{} seconds:{}",
          token.level, token.user, parentTokenUser, max_times, seconds);
    return token.value;
}


/* 生成随机token, 可能重复，需要进一步去重 */
std::string TokenManager::genRandomToken_() {
    std::string charsetTmp = charset_;
    std::random_shuffle(charsetTmp.begin(), charsetTmp.end());
    return charsetTmp.substr(0, length_);
}


/* 检查token的合法性 */
int TokenManager::checkToken(const std::string& value, int level, std::string* user) {
    std::lock_guard<std::mutex> lck(mutex_);
    return checkTokenWithNoLock_(value, level, user);
}

int TokenManager::checkTokenWithNoLock_(const std::string& value, int level, std::string* user) {
    removeInvalidToken_();
    LTrace("level:{}", level);
    auto iter = tokens_.find(value);
    if (iter == tokens_.end()) {
        return LV_INVALID;
    }
    Token& token = iter->second;
    LTrace("token.level:{}", token.level);
    if (user) {
        *user = token.user;
    }
    if (token.level & LV_ROOT) {
        return LV_ROOT;
    }
    else if (token.level & LV_ADMIN) {
        return LV_ADMIN;
    }
    else {
        token.remain_times--;
        return LV_GUEST;
    }
}


/* 自动清除无效的token */
void TokenManager::removeInvalidToken_() {
    time_t now = time(NULL);
    for (auto iter = tokens_.begin(); iter != tokens_.end(); /*++iter*/) {
        Token& token = iter->second;
        if (token.level > LV_ADMIN && (token.remain_times < 1 || token.ttl < now)) {
            LInfo("Remove Token: {}, {}", token.user, token.value);
            iter = tokens_.erase(iter);
        }
        else {
            ++iter;
        }
    }
}

/* 清除token */
void TokenManager::removeToken(const std::string& token) {
    std::lock_guard<std::mutex> lck(mutex_);
    auto iter = tokens_.find(token); 
    if (iter != tokens_.end()) {
        Token& token = iter->second;
        /* 禁止删除root的token */
        if (token.level > LV_ROOT) {
            LInfo("Remove Token. level:{}, user:{}, value: {}",
                  token.level, token.user, token.value);
            tokens_.erase(iter);
        }
    }
}

/* 读取配置文件 */
bool TokenManager::readConfig_(const std::string& cfgFile) {
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
    if (checkNull(json, "root", "admin", "config")) {
        LError("Missing key:root/admin/config");
        return false;
    }

    /* root */
    if (!parseTokenNode_(json["root"], LV_ROOT)) {
        return false;
    }

    /* admin */
    if (!parseTokenNode_(json["admin"], LV_ADMIN)) {
        return false;
    }

    /* config */
    if (!parseConfigNode_(json["config"])) {
        return false;
    }

    return true;
}

bool TokenManager::parseTokenNode_(Json::Value& node, TokenLevel level) {
    int size = node.size();
    for (int i = 0; i < size; ++i) {
        Json::Value& subNode = node[i];
        if (checkNull(subNode, "user", "value")) {
            LError("Missing key:user/value");
            return false;
        }
        Token token;
        token.level = level;
        token.remain_times = -1;
        token.ttl = -1;
        token.user = getStrVal(subNode, "user");
        token.value = getStrVal(subNode, "value");
        if (token.value.empty() || token.user.empty()) {
            LError("User/value can't be empty");
            return false;
        }
        if (tokens_.find(token.value) != tokens_.end()) {
            LError("Duplicate token");
            return false;

        }
        tokens_.emplace(token.value, token);
    }
    return true;
}

bool TokenManager::parseConfigNode_(Json::Value& node) {
    if (checkNull(node, "charset", "length")) {
        LError("Missing key:charset/length");
        return false;
    }

    /* charset */
    charset_ = getStrVal(node, "charset");
    if (charset_.length() < 16) {
        LError("key:charset's length must be 16 at least");
        return false;
    }

    /* length */
    length_ = getIntVal(node, "length");
    if (length_ > 128 || length_ < 8) {
        LError("8 <= key:length <= 128");
        return false;
    }
    // 如果长度大于字符集长度
    // 就将扩充字符集(复制)
    if (length_ > charset_.length()) {
        int base = length_ / charset_.length();
        std::string temp = charset_;
        for (int i = 0; i < base; ++i) {
            charset_.append(temp);
        }
    }
    return true;
}

