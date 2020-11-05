#include "base.h"
#include "../../share/notify/wx.h"
#include "../token/token_manager.h"
#include <fstream>
#include <sstream>

extern WxNotifier   g_wxNotifier;
extern TokenManager g_tokenMgr;


std::string getToken(Request& req) {
    std::string token = req.get_param("token");
    if (token.empty()) {
        token = req.get_cookie("token");
    }
    return token;
}

int checkToken(const std::string& token, Response& res, int level, std::string* user) {
    Json::Value root;
    if (token.empty()) {
        RETURN_MISSING_PARAM(token) LV_INVALID;
    }

    int ret = g_tokenMgr.checkToken(token, level, user);
    if (ret == LV_INVALID) {
        g_wxNotifier.notifyRoot("InvalidToken", token);
        RETURN_CODE(api::StatusCode::INVALID_TOKEN) LV_INVALID;
    }
    else {
        return ret;
    }
}

