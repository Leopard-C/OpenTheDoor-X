#include "new.h"
#include "../../base.h"
#include "../../../token/token_manager.h"

#include <unistd.h>

extern WxNotifier   g_wxNotifier;
extern TokenManager g_tokenMgr;

void cb_token_new(Request& req, Response& res) {
    Json::Value root, data;

    CHECK_TOKEN_EX(LV_ROOT | LV_ADMIN);
    CHECK_PARAM_INT(max_times);
    CHECK_PARAM_INT(ttl);
    CHECK_PARAM_STR(user);

    if (_level == LV_ADMIN) {
        if (max_times > 1) {
            RETURN_ERROR("有效次数最多1次");
        }
        if (ttl > 300) {
            RETURN_ERROR("有效时间最长300s");
        }
    }
    else {
        if (max_times > 10) {
            RETURN_ERROR("有效次数最多10次");
        }
        if (ttl > 86400) {
            RETURN_ERROR("有效时间最长86400s");
        }
    }

    std::string value = g_tokenMgr.newToken(_token, user, max_times, ttl);
    if (value.empty()) {
        RETURN_INTERNAL_SERVER_ERROR();
    }
    else {
        std::string text = _user + "生成了开门链接";
        std::string desp = "有效次数:" + std::to_string(max_times);
        desp += "\n有效时间:" + std::to_string(ttl);
        desp += "\n使用者:" + user;
        g_wxNotifier.notifyRoot(text, desp);
        data["token"] = value;
        RETURN_OK();
    }
}

