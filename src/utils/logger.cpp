#include "utils/logger.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <sstream>

Logger::Logger() {
    // 默认日志级别为INFO
    log_level_ = LogLevel::INFO;
    log_to_file_ = false;
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

void Logger::setLogLevel(LogLevel level) {
    log_level_ = level;
}

void Logger::enableFileLogging(const std::string& filename) {
    log_to_file_ = true;
    log_file_.open(filename, std::ios::app);
    if (!log_file_.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
        log_to_file_ = false;
    }
}

void Logger::disableFileLogging() {
    log_to_file_ = false;
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

std::string Logger::getCurrentTime() {
    std::time_t now = std::time(nullptr);
    char time_str[20];
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(time_str);
}

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < log_level_) {
        return;
    }

    std::ostringstream log_message;
    log_message << "[" << getCurrentTime() << "][" << logLevelToString(level) << "] " << message;

    // 输出到控制台
    std::cout << log_message.str() << std::endl;

    // 输出到文件
    if (log_to_file_) {
        log_file_ << log_message.str() << std::endl;
    }
}
