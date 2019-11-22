#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>

//#define NDEBUG

#ifndef NDEBUG
  #include <iostream>
  #define DEBUG(x) std::cout << (x) << std::endl
  #define DEBUGF(...) printf(__VA_ARGS__)
#else
  #define DEBUG(x)
  #define DEBUGF(...)
#endif

namespace utils {

extern char g_now[];
extern char g_cwd[];

// 去除字符串末尾的换行符
void removeLF(char* str, int size);
void removeLF(std::string& str);

// 获取当前时间
char* getTime(const char* fmt);

// 可执行文件路径
std::string getCwd();
std::string absolutePath(const std::string& path);

// 把字符串转为unsigned int整数
unsigned int toUInt(const char* str);

// 执行shell命令并获取结果
bool execShell(const char* cmd, char* result, int maxSize);

}

#endif // __UTILS_H__
