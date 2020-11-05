#pragma once
#include <sstream>
#include <string>

namespace util {
    
template<typename Output, typename Input>
bool convert(const Input& in, Output& out) {
    std::stringstream ss;
    ss << in;
    ss >> out;
    return !ss.fail();
}

std::string toUpper(const std::string& str);
std::string toLower(const std::string& str);

std::string& trim(std::string& input);

bool isDigit(const char ch);
bool isDigit(const std::string& str);

bool isAlnum(const char ch);
bool isAlnum(const std::string& str);


} // namespace util
