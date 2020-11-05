#include "user_manager.h"
#include "../../share/log/logger.h"
#include <fstream>

extern std::string g_cfgDir;


bool UserManager::setup() {
    if (!readConfig_(g_cfgDir + "/token.json")) {
        LError("Read {}/token.json failed", g_cfgDir);
        return false;
    }
    return true;
}

std::string UserManager::getToken(int fpID) const {
    for (auto& user : users_) {
        auto findIt = user.fpIDs.find(fpID);
        if (findIt != user.fpIDs.end()) {
            return user.token;
        }
    }
    return std::string();
}

bool UserManager::readConfig_(const std::string& cfgFile) {
    std::ifstream ifs(cfgFile);
    if (!ifs) {
        LError("Open file:{} failed", cfgFile);
        return false;
    }

    Json::Reader reader;
    Json::Value json;
    if (!reader.parse(ifs, json)) {
        LError("Parse file:{} failed", cfgFile);
        return false;
    }

    if (checkNull(json, "root", "admin")) {
        LError("Missing param:root/admin");
        return false;
    }

    /* root */
    if (!parseUserNode_(json["root"])) {
        LError("parse root node failed");
        return false;
    }

    /* admin */
    if (!parseUserNode_(json["admin"])) {
        LError("parse admin node failed");
        return false;
    }

    return true;
}

bool UserManager::parseUserNode_(Json::Value& node) {
    size_t size = node.size();
    for (size_t i = 0; i < size; ++i) {
        Json::Value& userNode = node[i];
        if (checkNull(userNode, "user", "value", "fpid")) {
            LError("Missing key:user/value/.. in admin");
            return false;
        }
        User user;
        user.name = getStrVal(userNode, "user");
        user.token = getStrVal(userNode, "value");
        Json::Value& fpid = userNode["fpid"];
        for (size_t k = 0; k < fpid.size(); ++k) {
            user.fpIDs.emplace(fpid[k].asInt());
        }
        users_.emplace_back(user);
    }
    return true;
}

