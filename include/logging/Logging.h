#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include "config/Config.h"

namespace pet_hospital {

class Logging {
public:
    using LogLevel = Config::LogLevel;
    using LogOutput = Config::LogOutput;

    Logging() = default;
    ~Logging();

    // 初始化日志系统
    bool init(const Config& config);

    // 日志记录方法
    void debug(const std::string& message, const std::string& file = "", int line = 0);
    void info(const std::string& message, const std::string& file = "", int line = 0);
    void warn(const std::string& message, const std::string& file = "", int line = 0);
    void error(const std::string& message, const std::string& file = "", int line = 0);
    void fatal(const std::string& message, const std::string& file = "", int line = 0);

    // 获取日志级别
    LogLevel get_level() const { return level_; }

    // 检查是否启用了某个日志级别
    bool is_debug_enabled() const { return level_ <= LogLevel::DEBUG; }
    bool is_info_enabled() const { return level_ <= LogLevel::INFO; }
    bool is_warn_enabled() const { return level_ <= LogLevel::WARN; }
    bool is_error_enabled() const { return level_ <= LogLevel::ERROR; }
    bool is_fatal_enabled() const { return level_ <= LogLevel::FATAL; }

private:
    // 日志记录内部方法
    void log(LogLevel level, const std::string& message, const std::string& file, int line);

    // 格式化日志消息
    std::string format_message(LogLevel level, const std::string& message, const std::string& file, int line) const;

    // 格式化时间戳
    std::string format_timestamp(const std::chrono::system_clock::time_point& time_point) const;

    // 获取日志级别字符串
    std::string get_level_string(LogLevel level) const;

    // 滚动日志文件
    void roll_log_file();

    // 检查日志文件大小
    bool check_log_file_size() const;

private:
    LogLevel level_ = LogLevel::INFO;
    std::vector<LogOutput> output_ = {LogOutput::CONSOLE};
    std::string file_path_ = "./pet_hospital.log";
    int max_file_size_ = 10485760;
    int max_backup_files_ = 5;

    std::ofstream log_file_;
    std::mutex log_mutex_;
};

// 全局日志对象声明
extern Logging g_logger;

// 日志宏定义
#define LOG_DEBUG(message) g_logger.debug(message, __FILE__, __LINE__)
#define LOG_INFO(message) g_logger.info(message, __FILE__, __LINE__)
#define LOG_WARN(message) g_logger.warn(message, __FILE__, __LINE__)
#define LOG_ERROR(message) g_logger.error(message, __FILE__, __LINE__)
#define LOG_FATAL(message) g_logger.fatal(message, __FILE__, __LINE__)

} // namespace pet_hospital
