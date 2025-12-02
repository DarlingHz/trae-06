#ifndef LOG_H
#define LOG_H

#include <string>
#include <sstream>
#include <fstream>

namespace recruitment {

/**
 * @brief 日志级别枚举
 */
enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

/**
 * @brief 日志类
 */
class Log {
public:
    /**
     * @brief 获取日志类实例
     * @return 日志类实例
     */
    static Log& getInstance();

    /**
     * @brief 设置日志级别
     * @param level 日志级别
     */
    void setLevel(LogLevel level);

    /**
     * @brief 设置日志输出文件
     * @param filename 日志文件路径
     * @return 设置成功返回true，否则返回false
     */
    bool setOutputFile(const std::string& filename);

    /**
     * @brief 记录日志
     * @param level 日志级别
     * @param message 日志消息
     * @param file 文件名
     * @param line 行号
     */
    void log(LogLevel level, const std::string& message, const std::string& file, int line);

private:
    /**
     * @brief 构造函数
     */
    Log();

    /**
     * @brief 析构函数
     */
    ~Log();

    /**
     * @brief 获取日志级别名称
     * @param level 日志级别
     * @return 日志级别名称
     */
    std::string getLevelName(LogLevel level);

    LogLevel level_; ///< 当前日志级别
    bool output_to_file_; ///< 是否输出到文件
    std::ofstream output_file_; ///< 日志输出文件
    std::mutex mutex_; ///< 互斥锁，用于保证线程安全
};

// 日志宏定义
#define LOG_TRACE(message) \
    do { \
        std::ostringstream oss; \
        oss << message; \
        Log::getInstance().log(LogLevel::TRACE, oss.str(), __FILE__, __LINE__); \
    } while (false)

#define LOG_DEBUG(message) \
    do { \
        std::ostringstream oss; \
        oss << message; \
        Log::getInstance().log(LogLevel::DEBUG, oss.str(), __FILE__, __LINE__); \
    } while (false)

#define LOG_INFO(message) \
    do { \
        std::ostringstream oss; \
        oss << message; \
        Log::getInstance().log(LogLevel::INFO, oss.str(), __FILE__, __LINE__); \
    } while (false)

#define LOG_WARN(message) \
    do { \
        std::ostringstream oss; \
        oss << message; \
        Log::getInstance().log(LogLevel::WARN, oss.str(), __FILE__, __LINE__); \
    } while (false)

#define LOG_ERROR(message) \
    do { \
        std::ostringstream oss; \
        oss << message; \
        Log::getInstance().log(LogLevel::ERROR, oss.str(), __FILE__, __LINE__); \
    } while (false)

#define LOG_FATAL(message) \
    do { \
        std::ostringstream oss; \
        oss << message; \
        Log::getInstance().log(LogLevel::FATAL, oss.str(), __FILE__, __LINE__); \
    } while (false)

} // namespace recruitment

#endif // LOG_H