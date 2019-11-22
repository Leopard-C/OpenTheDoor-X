#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>

namespace utils {

  extern char g_now[];

  // 去除字符串末尾的换行符
  void removeLF(char* str, int size);
  void removeLF(std::string& str);

  // 获取当前时间
  char* getTime(const char* fmt);

  // 把字符串转为unsigned int整数
  unsigned int toUInt(const char* str);

  // 执行shell命令并获取结果
  bool execShell(const char* cmd, char* result, int maxSize);

}

#endif // __UTILS_H__
