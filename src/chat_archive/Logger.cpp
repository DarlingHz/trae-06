#include "chat_archive/Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace chat_archive {

std::shared_ptr<spdlog::logger> Logger::instance_;

void Logger::init(const std::string& level) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v");
    
    instance_ = spdlog::stdout_color_mt("chat_archive");
    
    if (level == "trace") {
        instance_->set_level(spdlog::level::trace);
    } else if (level == "debug") {
        instance_->set_level(spdlog::level::debug);
    } else if (level == "warn") {
        instance_->set_level(spdlog::level::warn);
    } else if (level == "error") {
        instance_->set_level(spdlog::level::err);
    } else if (level == "critical") {
        instance_->set_level(spdlog::level::critical);
    } else {
        instance_->set_level(spdlog::level::info);
    }
    
    CHAT_ARCHIVE_LOG_INFO("Logger initialized with level: {}", level);
}

} // namespace chat_archive