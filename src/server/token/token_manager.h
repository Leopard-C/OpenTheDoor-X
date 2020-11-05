#pragma once
#include <chrono>
#include <set>
#include <map>
#include <string>
#include <mutex>
#include "../../share/json/json.h"

enum TokenLevel {
    LV_INVALID = 0,
    LV_ROOT    = 1,
    LV_ADMIN   = 1 << 1,
    LV_GUEST   = 1 << 2
};


class TokenManager {
public:    
    struct Token {
        TokenLevel level = LV_GUEST;
        int id;
        int remain_times = -1;
        time_t ttl = -1;
        std::string user;   /* 用户名 */
        std::string parent; /* 哪个用户生成的该token, [该变量是value，而不是user] */
        std::string value;  /* token值 */
    };

    bool setup();
    
    // @max_times: 最多使用次数
    // @sec:       有效秒数
    // 生成的token一定是LV_GUEST级别的
    std::string newToken(const std::string& parentTokenValue,
                         const std::string& user,
                         int max_times, int seconds);

    /* 检查token的合法性 */
    int checkToken(const std::string& value, int level, std::string* user = nullptr);

    /* 移除token */
    void removeToken(const std::string& value);

private:
    /* 读取配置文件 */
    bool readConfig_(const std::string& cfgFile);

    /* 自动清除失效的token */
    void removeInvalidToken_();

    /* 生成随机token, 可能重复，需要进一步去重 */
    std::string genRandomToken_();

    bool parseTokenNode_(Json::Value& node, TokenLevel level);
    bool parseConfigNode_(Json::Value& node);

    int checkTokenWithNoLock_(const std::string& value, int level, std::string* user = nullptr);

private:
    std::map<std::string, Token> tokens_;
    std::mutex mutex_;

    int currTokenId;

    /* config */
    std::string charset_ = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^%*()[]-+<>/";
    int  length_ = 32;
};

