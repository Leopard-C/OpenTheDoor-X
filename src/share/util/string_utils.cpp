#include "string_utils.h"
#include <unistd.h>
#include <chrono>
#include <cstring>
#include <cassert>

namespace util {

std::string toUpper(const std::string& str) {
    std::string newStr = str;
    for (auto& c : newStr) {
        if (c >= 'a' && c <= 'z') {
            c -= 32;
        }
    }
    return newStr;
}

std::string toLower(const std::string& str) {
    std::string newStr = str;
    for (auto& c : newStr) {
        if (c >= 'A' && c <= 'Z') {
            c += 32;
        }
    }
    return newStr;
}


bool isDigit(const char ch) {
    return ch >= '0' && ch <= '9';
}

bool isDigit(const std::string& str) {
    for (auto c : str) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

bool isAlnum(const char ch) {
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool isAlnum(const std::string& str) {
    for (auto c : str) {
        if (!isAlnum(c)) {
            return false;
        }
    }
    return true;
}

/* 去除左右的空格 */
std::string& trim(std::string& input) {
    if (input.empty())
        return input;
    input.erase(0, input.find_first_not_of(" "));
    input.erase(input.find_last_not_of(" ") + 1);
    return input;
}

} // namespace util

