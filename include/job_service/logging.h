#ifndef JOB_SERVICE_LOGGING_H
#define JOB_SERVICE_LOGGING_H

#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <mutex>

namespace job_service {

// 日志级别
enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

// 转换日志级别到字符串
std::string log_level_to_string(LogLevel level);

// 从字符串转换到日志级别
LogLevel string_to_log_level(const std::string& str);

// 日志类
class Logger {
private:
    LogLevel min_level_;
    std::mutex mutex_;
    
    // 获取当前时间字符串
    std::string get_current_time() const;
    
    // 打印日志消息
    void print_log(LogLevel level, const std::string& message) const;

public:
    // 构造函数
    explicit Logger(LogLevel min_level = LogLevel::INFO);
    
    // 设置日志级别
    void set_level(LogLevel level);
    
    // 获取日志级别
    LogLevel get_level() const;
    
    // 日志记录函数
    void log(LogLevel level, const std::string& message) const;
    
    // 模板日志函数
    template<typename... Args>
    void log(LogLevel level, Args&&... args) const {
        if (level < min_level_) {
            return;
        }
        
        std::ostringstream oss;
        (oss << ... << std::forward<Args>(args));
        print_log(level, oss.str());
    }
    
    // 便捷日志函数
    void trace(const std::string& message) const;
    void debug(const std::string& message) const;
    void info(const std::string& message) const;
    void warn(const std::string& message) const;
    void error(const std::string& message) const;
    void fatal(const std::string& message) const;
    
    template<typename... Args>
    void trace(const std::string& format, Args&&... args) const {
        log(LogLevel::TRACE, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void debug(const std::string& format, Args&&... args) const {
        log(LogLevel::DEBUG, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void info(const std::string& format, Args&&... args) const {
        log(LogLevel::INFO, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void warn(const std::string& format, Args&&... args) const {
        log(LogLevel::WARN, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void error(const std::string& format, Args&&... args) const {
        log(LogLevel::ERROR, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void fatal(const std::string& format, Args&&... args) const {
        log(LogLevel::FATAL, format, std::forward<Args>(args)...);
    }
};

// 全局日志实例
extern Logger global_logger;

} // namespace job_service

#endif // JOB_SERVICE_LOGGING_H
