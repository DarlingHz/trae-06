#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <mutex>
#include <type_traits>

namespace giftcard {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

class Logger {
public:
    // 单例模式
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    // 初始化日志系统
    bool init(const std::string& log_file_path = "", LogLevel level = LogLevel::INFO);

    // 设置日志级别
    void setLogLevel(LogLevel level);

    // 日志输出方法
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);

    // 带格式的日志输出方法
    template<typename... Args>
    void debug(const char* format, Args&&... args) {
        log(LogLevel::DEBUG, format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(const char* format, Args&&... args) {
        log(LogLevel::INFO, format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(const char* format, Args&&... args) {
        log(LogLevel::WARN, format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(const char* format, Args&&... args) {
        log(LogLevel::ERROR, format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void fatal(const char* format, Args&&... args) {
        log(LogLevel::FATAL, format, std::forward<Args>(args)...);
    }

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // 日志输出核心方法
    template<typename... Args>
    void log(LogLevel level, const char* format, Args&&... args) {
        if (level < log_level_) {
            return;
        }

        std::unique_lock<std::mutex> lock(mutex_);

        // 格式化日志消息
        char buffer[1024];
        // 处理std::string类型的参数
        auto convert_to_c_str = [](auto&& arg) {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>) {
                return arg.c_str();
            } else {
                return arg;
            }
        };
        int len = snprintf(buffer, sizeof(buffer), format, convert_to_c_str(std::forward<Args>(args))...);
        if (len < 0 || len >= sizeof(buffer)) {
            len = sizeof(buffer) - 1;
            buffer[len] = '\0';
        }

        // 生成日志时间
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        auto tm_now = std::localtime(&time_t_now);
        char time_buffer[64];
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_now);

        // 生成日志级别字符串
        const char* level_str = nullptr;
        switch (level) {
            case LogLevel::DEBUG: level_str = "DEBUG";
                break;
            case LogLevel::INFO: level_str = "INFO";
                break;
            case LogLevel::WARN: level_str = "WARN";
                break;
            case LogLevel::ERROR: level_str = "ERROR";
                break;
            case LogLevel::FATAL: level_str = "FATAL";
                break;
        }

        // 格式化日志行
        char log_line[2048];
        snprintf(log_line, sizeof(log_line), "%s [%s] %s\n", time_buffer, level_str, buffer);

        // 输出日志
        if (log_file_.is_open()) {
            log_file_.write(log_line, strlen(log_line));
            log_file_.flush();
        } else {
            std::cout << log_line;
        }
    }

    std::ofstream log_file_;
    LogLevel log_level_ = LogLevel::INFO;
    std::mutex mutex_;
};

// 日志宏定义
#define LOG_DEBUG Logger::getInstance().debug
#define LOG_INFO Logger::getInstance().info
#define LOG_WARN Logger::getInstance().warn
#define LOG_ERROR Logger::getInstance().error
#define LOG_FATAL Logger::getInstance().fatal

} // namespace giftcard

#endif // LOGGER_H
