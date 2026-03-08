#include "Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace redis_engine::utils {

std::shared_ptr<spdlog::logger> Logger::s_CoreLogger;

void Logger::Init() {
    spdlog::set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%n] [%l]:%$ %v");
    s_CoreLogger = spdlog::stdout_color_mt("ENGINE");
    s_CoreLogger->set_level(spdlog::level::trace);
}

std::shared_ptr<spdlog::logger>& Logger::GetCoreLogger() {
    return s_CoreLogger;
}

} // namespace redis_engine::utils
