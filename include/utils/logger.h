#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>

// 日志级别枚举
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    LogLevel log_level_;
    bool log_to_file_;
    std::ofstream log_file_;

    // 私有构造函数，实现单例模式
    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // 获取当前时间
    std::string getCurrentTime();

    // 将日志级别转换为字符串
    std::string logLevelToString(LogLevel level);

public:
    // 单例模式获取实例
    static Logger& getInstance();

    // 析构函数
    ~Logger();

    // 设置日志级别
    void setLogLevel(LogLevel level);

    // 启用文件日志记录
    void enableFileLogging(const std::string& filename);

    // 禁用文件日志记录
    void disableFileLogging();

    // 日志记录方法
    void log(LogLevel level, const std::string& message);
};

// 日志宏定义
#define LOG_DEBUG(message) Logger::getInstance().log(LogLevel::DEBUG, message)
#define LOG_INFO(message) Logger::getInstance().log(LogLevel::INFO, message)
#define LOG_WARNING(message) Logger::getInstance().log(LogLevel::WARNING, message)
#define LOG_ERROR(message) Logger::getInstance().log(LogLevel::ERROR, message)

#endif // LOGGER_H
