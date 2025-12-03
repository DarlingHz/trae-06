#include "logger.h"
#include <stdexcept>
#include <iostream>

bool Logger::init(const std::string& file_path, LogLevel level) {
    log_file_.open(file_path, std::ios::app | std::ios::out);
    if (!log_file_.is_open()) {
        return false;
    }
    current_level_ = level;
    return true;
}

std::string Logger::get_level_string(LogLevel level) {
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO: return "INFO";
        case WARN: return "WARN";
        case ERROR: return "ERROR";
        case FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::get_timestamp() {
    time_t now = time(nullptr);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return std::string(buffer);
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < current_level_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string log_line = "[" + get_timestamp() + "] " + 
                          get_level_string(level) + ": " + 
                          message + "\n";
    
    // 输出到控制台
    std::cout << log_line;
    
    // 输出到文件
    if (log_file_.is_open()) {
        log_file_ << log_line;
        log_file_.flush();
    }
}