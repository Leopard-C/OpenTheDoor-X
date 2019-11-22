#include "../include/utils.h"

#include <ctime>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <string>

char utils::g_now[32] = { 0 };
char utils::g_cwd[256] = { 0 };

// 去除字符串最后的换行符
void utils::removeLF(char* str, int size) {
  for (int i = size - 1; i >= 0; --i) {
    if (str[i] == '\n') {
      str[i] = 0;
      return;
    }
  }
}

void utils::removeLF(std::string& str) {
  int size = str.size();
  for (int i = size - 1; i >= 0; --i) {
    if (str[i] == '\n') {
      str[i] = 0;
      return;
    }
  }
}


// 获取当前时间
char* utils::getTime(const char* fmt) {
  time_t now = time(0);
  strftime(g_now, sizeof(g_now), fmt, localtime(&now));

  return g_now;
}


// 获取可执行文件路径
std::string utils::getCwd() {
  getcwd(g_cwd, 256);
  return std::string(g_cwd);
}

// 生成绝对路径
std::string utils::absolutePath(const std::string& path) {
  std::string ab_path = "/home/pi/.door/client/";
  ab_path += path;

  return ab_path;
}


// 把 *十六进制* 字符串转为无符号整型
unsigned int utils::toUInt(const char* str) {
  unsigned int ret = 0;
  if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    sscanf(str+2, "%x", &ret);
  else
    sscanf(str, "%x", &ret);

  return ret;
}


// 执行shell命令并获取输出
bool utils::execShell(const char* cmd, char* result, int maxSize) {
  FILE* fp = popen(cmd, "r");
  if (!fp)
    return false;

  char buf[1024] = { 0 };
  while (fgets(buf, 1024, fp) != NULL) {
    if (strlen(result) + 1024 <= maxSize)
      strcat(result, buf);
    else
      strncat(result, buf, maxSize - strlen(result));
    memset(buf, 0, 1024);
  }
  pclose(fp);
  fp = NULL;

  return true;
}
