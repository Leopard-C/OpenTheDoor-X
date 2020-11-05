#pragma once
#include <string>
#include <set>
#include "../../share/json/json.h"

struct User {
    std::string name;
    std::string token;
    std::set<int> fpIDs;
};

class UserManager {
public:
    bool setup();
    std::string getToken(int fpID) const;

private:
    bool readConfig_(const std::string& cfgFile);
    bool parseUserNode_(Json::Value& value);

private:
    std::vector<User> users_;
};

