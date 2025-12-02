#ifndef Logger_hpp
#define Logger_hpp

#include <string>
#include <iostream>
#include <chrono>
#include <ctime>

class Logger {
public:
  enum Level {
    DEBUG,
    INFO,
    WARN,
    ERROR
  };

  // 设置日志级别
  static void setLevel(Level level);

  // 调试日志 - 单个字符串重载
  static void debug(const std::string& message) {
    log(DEBUG, message.c_str());
  }

  // 调试日志 - 格式化字符串
  template<typename... Args>
  static void debug(const char* format, Args&&... args) {
    log(DEBUG, format, std::forward<Args>(args)...);
  }

  // 信息日志 - 单个字符串重载
  static void info(const std::string& message) {
    log(INFO, message.c_str());
  }

  // 信息日志 - 格式化字符串
  template<typename... Args>
  static void info(const char* format, Args&&... args) {
    log(INFO, format, std::forward<Args>(args)...);
  }

  // 警告日志 - 单个字符串重载
  static void warn(const std::string& message) {
    log(WARN, message.c_str());
  }

  // 警告日志 - 格式化字符串
  template<typename... Args>
  static void warn(const char* format, Args&&... args) {
    log(WARN, format, std::forward<Args>(args)...);
  }

  // 错误日志 - 单个字符串重载
  static void error(const std::string& message) {
    log(ERROR, message.c_str());
  }

  // 错误日志 - 格式化字符串
  template<typename... Args>
  static void error(const char* format, Args&&... args) {
    log(ERROR, format, std::forward<Args>(args)...);
  }

private:
  static Level s_level;

  // 获取当前时间字符串
  static std::string getCurrentTime();

  // 获取日志级别字符串
  static std::string getLevelString(Level level);

  // 格式化字符串
  template<typename... Args>
  static std::string formatString(const char* format, Args&&... args) {
    size_t size = snprintf(nullptr, 0, format, std::forward<Args>(args)...);
    std::string result(size, '\0');
    snprintf(&result[0], size + 1, format, std::forward<Args>(args)...);
    return result;
  }

  // 日志核心函数 - 单个字符串重载
  static void log(Level level, const char* message) {
    if (level < s_level) {
      return;
    }

    std::string timeStr = getCurrentTime();
    std::string levelStr = getLevelString(level);

    std::cout << timeStr << " [" << levelStr << "] " << message << std::endl;
  }

  // 日志核心函数 - 格式化字符串
  template<typename... Args>
  static void log(Level level, const char* format, Args&&... args) {
    if (level < s_level) {
      return;
    }

    std::string timeStr = getCurrentTime();
    std::string levelStr = getLevelString(level);
    std::string message = formatString(format, std::forward<Args>(args)...);

    std::cout << timeStr << " [" << levelStr << "] " << message << std::endl;
  }
};

#endif /* Logger_hpp */
