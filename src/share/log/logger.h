#pragma once
#define SPDLOG_COMPILED_LIB 

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>


class Logger {
public:
    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    ~Logger() { spdlog::drop_all(); }
public:
	static Logger& GetInstance() {
		static Logger m_instance;
		return m_instance;
	}
	auto GetLogger() {
        //return spdlog::get("daily");
        return logger;
    }

private:
	std::shared_ptr<spdlog::logger> logger;
};


#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)

#ifndef suffixsth
#define suffixsth(msg) std::string(msg).append(" <")\
	.append(__FILENAME__).append("> <").append(__func__)\
	.append("> <").append(std::to_string(__LINE__))\
	.append(">").c_str()
#endif // suffix


// print in console
#ifdef NDEBUG
#    include <stdio.h>
#    define Log(x)
#    define LogF(fmt, ...)
#    define LogEx(fmt, ...)
#else
#    include <iostream>
#    define Log(x) std::cout << x << std::endl
#    define LogF(fmt, ...) printf("%s"##fmt"\n", ##__VA_ARGS__)
#    define LogEx(fmt, ...) printf("%s(%d)-<%s>: "##fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#endif


// output to file
#ifdef __DEV__
  #define LTrace(msg,...) \
      Logger::GetInstance().GetLogger()->trace(suffixsth(msg),  ##__VA_ARGS__)
  #define LDebug(msg,...) \
      Logger::GetInstance().GetLogger()->debug(suffixsth(msg),  ##__VA_ARGS__)
  #define LInfo(msg,...) \
      Logger::GetInstance().GetLogger()->info(suffixsth(msg),  ##__VA_ARGS__)
  #define LWarn(msg,...) \
      Logger::GetInstance().GetLogger()->warn(suffixsth(msg),  ##__VA_ARGS__)
  #define LError(msg,...) \
      Logger::GetInstance().GetLogger()->error(suffixsth(msg),  ##__VA_ARGS__)
  #define LCritical(msg,...) \
      Logger::GetInstance().GetLogger()->critical(suffixsth(msg),  ##__VA_ARGS__)
#else
  #define LTrace(...) Logger::GetInstance().GetLogger()->trace(__VA_ARGS__)
  #define LDebug(...) Logger::GetInstance().GetLogger()->debug(__VA_ARGS__)
  #define LInfo(...) Logger::GetInstance().GetLogger()->info(__VA_ARGS__)
  //#define LWarn(...) Logger::GetInstance().GetLogger()->warn(__VA_ARGS__)
  //#define LError(...) Logger::GetInstance().GetLogger()->error(__VA_ARGS__)
  //#define LCritical(...) Logger::GetInstance().GetLogger()->critical(__VA_ARGS__)
  #define LWarn(msg,...) \
      Logger::GetInstance().GetLogger()->warn(suffixsth(msg),  ##__VA_ARGS__)
  #define LError(msg,...) \
      Logger::GetInstance().GetLogger()->error(suffixsth(msg),  ##__VA_ARGS__)
  #define LCritical(msg,...) \
      Logger::GetInstance().GetLogger()->critical(suffixsth(msg),  ##__VA_ARGS__)
#endif

#define CHECK_RET(ret, msg, args...) if (ret != 0) { \
    LError(msg, args); \
    return ret; \
} \

#define CHECK_ERR(ret, msg, args...) if (ret) { \
    LError(msg, args); \
    return ret; \
} \

// for DEBUG
//
#ifdef __DEV__
#  define DBG LError("LINE: {}", __LINE__);
#else
#  define DBG
#endif



