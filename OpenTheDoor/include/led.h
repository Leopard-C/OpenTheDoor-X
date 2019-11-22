#ifndef __LED_H__
#define __LED_H__

#include <mutex>
#include <string>

#include "../include/thread_pool.hpp"

enum LED_STATUS {
  LED_OFF = 0,
  LED_ON = 1
};


class Led {
  public:
    Led();
    ~Led();
  public:
    bool setup();
    void red(LED_STATUS status);
    void green(LED_STATUS status);
    void redT(int ms);    // 红灯亮ms毫秒
    void greenT(int ms);  // 绿灯亮ms毫秒
    void redFlash(int ms);  // 红灯闪烁ms毫秒
    void greenFlash(int ms); // 绿灯闪烁ms毫秒

  private:
    void flash_(int pin, int seconds);
    bool readJson_(const std::string& filename);
  private:
    bool setuped_;
    int redPin_;
    int greenPin_;
    int flashFreq_;  // times per second
  private:
    ThreadPool pool_;
    std::mutex redMutex_;
    std::mutex greenMutex_;
};

#endif // __LED_H__
