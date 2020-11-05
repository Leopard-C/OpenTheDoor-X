#pragma once
#include <string>
#include "module/fp_module.h"
#include "user/user_manager.h"


class DoorClient {
public:
    bool setup();
    void run();
    bool quit() { shouldQuit_ = true; }

private:
    /* 向服务器发送请求开门 */
    bool openTheDoor_(int pageID, int score);

private:
    FpModule fpModule_;
    UserManager userManager_;
    int errorCount_;
    bool shouldQuit_ = false;
};
