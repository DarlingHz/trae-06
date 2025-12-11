#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <ctime>

class Logger {
public:
    enum Level {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };
    
    // 初始化日志系统
    static bool init(const std::string& file_path, Level level = INFO,
                     long max_file_size = 10485760L, int max_backup_files = 5);
    
    // 日志记录方法
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);
    static void fatal(const std::string& message);
    
    // 设置日志级别
    static void setLevel(Level level);
    
    // 关闭日志系统
    static void close();
    
private:
    // 私有构造函数，防止实例化
    Logger() = delete;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // 记录日志的实际方法
    static void log(Level level, const std::string& message);
    
    // 滚动日志文件
    static bool rollLogFile();
    
    // 格式化时间
    static std::string formatTime(const std::time_t& time);
    
    // 格式化日志级别
    static std::string formatLevel(Level level);
    
    // 成员变量
    static std::ofstream log_file_;
    static std::mutex mutex_;
    static Level level_;
    static std::string file_path_;
    static long max_file_size_;
    static int max_backup_files_;
    static long current_file_size_;
};

#endif // LOGGER_H