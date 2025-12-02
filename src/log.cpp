#include "log.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <mutex>

namespace recruitment {

Log::Log() : level_(LogLevel::INFO), output_to_file_(false) {
}

Log& Log::getInstance() {
    static Log instance;
    return instance;
}

void Log::setLevel(LogLevel level) {
    level_ = level;
}

bool Log::setOutputFile(const std::string& filename) {
    output_file_.open(filename, std::ios::app);
    output_to_file_ = output_file_.is_open();
    return output_to_file_;
}

void Log::log(LogLevel level, const std::string& message, const std::string& file, int line) {
    if (level < level_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    // 获取当前时间
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);

    // 构建日志消息
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " "
        << getLevelName(level) << " "
        << file << ":" << line << " - "
        << message;

    std::string log_message = oss.str();

    // 输出到控制台
    std::cout << log_message << std::endl;

    // 输出到文件
    if (output_to_file_) {
        output_file_ << log_message << std::endl;
        output_file_.flush();
    }
}

std::string Log::getLevelName(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE:
            return "TRACE";
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::FATAL:
            return "FATAL";
        default:
            return "UNKNOWN";
    }
}

Log::~Log() {
    if (output_to_file_) {
        output_file_.close();
    }
}

} // namespace recruitment