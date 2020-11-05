#include "logger.h"
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <ctime>
#include <vector>

extern std::string g_projDir;
extern std::string g_logsDir;

Logger::Logger() {
    std::vector<spdlog::sink_ptr> sinks;
#ifdef __DEV__
    time_t now = time(NULL);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y%m%d_%H%M%S", localtime(&now));
    std::string logFileName = g_logsDir + "/" + std::string(tmp);
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logFileName, 1024 * 1024 * 5, 3));
#else
    sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink_mt>(g_logsDir + "/log", 0, 0));
#endif

    logger = std::make_shared<spdlog::logger>("log", std::begin(sinks), std::end(sinks));
    spdlog::register_logger(logger);
    logger->set_pattern("[%H:%M:%S][%l] %v");
#ifdef __DEV__
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::debug);
#else
    logger->set_level(spdlog::level::info);
    logger->flush_on(spdlog::level::err);
#endif
    spdlog::flush_every(std::chrono::seconds(1));
    SPDLOG_LOGGER_DEBUG(logger, "test4");
}

