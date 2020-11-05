#pragma once
#include <string>
#include "../../share/module/led.h"


class FpModule {
public:
    FpModule();
    ~FpModule();

public:
    bool setup();

    bool match(int maxFailCount, int maxBlockTimeMs, int& pageID, int& score);
    bool matchOnce(int maxBlockTimeMs, int& pageID, int& score);

private:
    bool readConfig_(const std::string& cfgFile);
    bool powerON_();
    bool powerOFF_();
    bool match_(int& pageID, int& score, int& timeConsumedUs);

private:
    int  relayPin_;  // 继电器信号引脚号
    std::string serialFile_;
    Led  redLed_;
};

