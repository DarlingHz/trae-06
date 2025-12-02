#include "Logger.h"
#include <iostream>

namespace giftcard {

bool Logger::init(const std::string& log_file_path, LogLevel level) {
    try {
        // 如果指定了日志文件路径，则打开日志文件
        if (!log_file_path.empty()) {
            log_file_.open(log_file_path, std::ios::app | std::ios::out);
            if (!log_file_.is_open()) {
                std::cerr << "Failed to open log file: " << log_file_path << std::endl;
                return false;
            }
        }

        // 设置日志级别
        log_level_ = level;

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize logger: " << e.what() << std::endl;
        return false;
    }
}

void Logger::setLogLevel(LogLevel level) {
    std::unique_lock<std::mutex> lock(mutex_);
    log_level_ = level;
}

} // namespace giftcard
