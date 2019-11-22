#ifndef __FP_MODULE_H__
#define __FP_MODULE_H__

#include <string>

enum POWER_STATUS {
  POWER_OFF = 0,
  POWER_ON = 1
};

class FpModule {
public:
  FpModule();
  ~FpModule();
public:
  bool setup();
  bool power(POWER_STATUS status);
  bool match(int& pageID, int& score);
  bool downImage(); 
public:
  bool powered; // 是否已经供电

private:
  bool readJson_(const std::string& filename);
private:
  int  relayPin_;  // 继电器信号引脚号
  bool setuped_;
  std::string serialFile_;
};


#endif // __FP_MODULE_H__
