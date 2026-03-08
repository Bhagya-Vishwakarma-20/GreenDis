#pragma once

#include <spdlog/spdlog.h>
#include <memory>
#include <string>

namespace redis_engine::utils {

class Logger {
public:
    static void Init();
    static std::shared_ptr<spdlog::logger>& GetCoreLogger();

private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
};

} // namespace redis_engine::utils

// Client macros for clean usage 
#define LOG_TRACE(...)    ::redis_engine::utils::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)     ::redis_engine::utils::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)     ::redis_engine::utils::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    ::redis_engine::utils::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::redis_engine::utils::Logger::GetCoreLogger()->critical(__VA_ARGS__)
