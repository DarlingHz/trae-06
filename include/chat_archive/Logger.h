#pragma once

#include <spdlog/spdlog.h>
#include <memory>
#include <string>

namespace chat_archive {

class Logger {
public:
    static void init(const std::string& level = "info");
    
    static std::shared_ptr<spdlog::logger>& get() {
        return instance_;
    }
    
private:
    static std::shared_ptr<spdlog::logger> instance_;
};

} // namespace chat_archive

#define CHAT_ARCHIVE_LOG_TRACE(...) ::chat_archive::Logger::get()->trace(__VA_ARGS__)
#define CHAT_ARCHIVE_LOG_DEBUG(...) ::chat_archive::Logger::get()->debug(__VA_ARGS__)
#define CHAT_ARCHIVE_LOG_INFO(...) ::chat_archive::Logger::get()->info(__VA_ARGS__)
#define CHAT_ARCHIVE_LOG_WARN(...) ::chat_archive::Logger::get()->warn(__VA_ARGS__)
#define CHAT_ARCHIVE_LOG_ERROR(...) ::chat_archive::Logger::get()->error(__VA_ARGS__)
#define CHAT_ARCHIVE_LOG_CRITICAL(...) ::chat_archive::Logger::get()->critical(__VA_ARGS__)