#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#include "../include/as608.h"
#include "../include/utils.h"
#include "../include/notifier.h"
#include "../include/motor.h"
#include "../include/fp_module.h"
#include "../include/led.h"

#define LOG(x) std::cerr << (x) << std::endl

// 全局变量
FpModule g_fp;
Motor    g_motor;
Notifier g_notifier;
Led      g_led;

void setup();
void run();
void exitFunc();
void handleCtrlC(int num);


/********************************* main() begin **********************/

int main(int argc, char* argv[]) {
  // 注册退出函数
  atexit(exitFunc);
  // 捕获 ctrl+c
  signal(SIGINT, handleCtrlC);

  // 初始化配置
  setup();

  // 主循环
  run();

  return 0;
}

/********************************* main() end *************************/



// 初始化配置
void setup() {
  // 初始化wiringPi库
  if (-1 == wiringPiSetup()) {
    LOG("WiringPi setup failed!");
    exit(1);
  }

  // 初始化指纹识别模块
  if (!g_fp.setup()) {
    LOG("Fp module setup failed!");
    exit(1);
  }

  // 初始化电机
  if (!g_motor.setup()) {
    LOG("Motor setup failed!");
    exit(1);
  }

  // 初始化Led
  if (!g_led.setup()) {
    LOG("Led setup failed");
    exit(1);
  }

  // 初始化通知器
  if (!g_notifier.setup()) {
    LOG("Notifier setup failed");
    exit(1);
  }
} // end setup()


// 退出函数
void exitFunc() {
  LOG("Exit!\n----------------------------------------------------------------");
}

// 捕获Ctrl+C
void handleCtrlC(int num) {
  LOG("Catch SIGINT, exit!");
  exit(2);
}


// 主循环
void run() {
  int pageID = 0;
  int score  = 0;
  int poweredTime = 0;

  while(true) {
    // 检测手指存在
    if (PS_DetectFinger(HIGH)) {
      delay(100);
      // 如果没有供电，就供电
      if (!g_fp.powered) {
        if (!g_fp.power(POWER_ON)) {
          LOG("Power error");
          continue;
        }
        poweredTime = 0;
      }
      
      // 采集指纹并匹配
      if (g_fp.match(pageID, score)) {
        // 匹配成功后断电
        g_fp.power(POWER_OFF);
        poweredTime = 0;

        g_led.greenT(1000); // 绿灯亮1s, 非阻塞
        g_motor.rotateCW(); // 电机转动，开门

        // 微信通知
        g_notifier.notifyAll(Type::DoorOpen, Who(pageID / 10));
       
        delay(5000);
        g_motor.rotateCCW(); // 电机转动，关门
      }
      else {
        g_led.redT(500);
        //g_notifier.notifyMaster(Type::Alarm);
        LOG("match fail");
      }

    } // end if detectFinger(HIGH)

    delay(200);
    if (g_fp.powered) {
      poweredTime += 200;
      if (poweredTime >= 15000)
        g_fp.power(POWER_OFF);
    }

  } // end while

} // end function run()
