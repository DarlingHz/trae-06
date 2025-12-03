#pragma once

#include <iostream>
#include <string>
#include <ctime>
#include <sstream>

enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLevel(LogLevel level) { currentLevel = level; }

    template<typename... Args>
    void debug(const char* fmt, Args... args) {
        log(DEBUG, fmt, args...);
    }

    template<typename... Args>
    void info(const char* fmt, Args... args) {
        log(INFO, fmt, args...);
    }

    template<typename... Args>
    void warn(const char* fmt, Args... args) {
        log(WARN, fmt, args...);
    }

    template<typename... Args>
    void error(const char* fmt, Args... args) {
        log(ERROR, fmt, args...);
    }

private:
    LogLevel currentLevel = INFO;

    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    const char* getLevelStr(LogLevel level) const {
        switch(level) {
            case DEBUG: return "DEBUG";
            case INFO: return "INFO";
            case WARN: return "WARN";
            case ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    std::string getCurrentTime() const {
        time_t now = time(nullptr);
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return buf;
    }

    template<typename... Args>
    void log(LogLevel level, const char* fmt, Args... args) {
        if (level < currentLevel) return;

        char buffer[1024];
        snprintf(buffer, sizeof(buffer), fmt, args...);

        std::cout << "[" << getCurrentTime() << "] "
                  << getLevelStr(level) << ": "
                  << buffer << std::endl;
    }
};

#define LOG_DEBUG Logger::getInstance().debug
#define LOG_INFO Logger::getInstance().info
#define LOG_WARN Logger::getInstance().warn
#define LOG_ERROR Logger::getInstance().error
