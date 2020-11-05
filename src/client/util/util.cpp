#include "util.h"

#include <ctime>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <string>

namespace util {

// 把 *十六进制* 字符串转为无符号整型
unsigned int hexStrToUInt(const char* str) {
    unsigned int ret = 0;
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        sscanf(str+2, "%x", &ret);
    }
    else {
        sscanf(str, "%x", &ret);
    }
    return ret;
}

} // namespace util

