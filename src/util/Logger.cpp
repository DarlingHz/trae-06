#include "Logger.hpp"

// 静态成员初始化
Logger::Level Logger::s_level = Logger::INFO;

void Logger::setLevel(Level level) {
  s_level = level;
}

std::string Logger::getCurrentTime() {
  auto now = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
    now.time_since_epoch()
  ) % 1000;

  std::tm tm_now;
  localtime_r(&time_t_now, &tm_now);

  char buffer[30];
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
    tm_now.tm_year + 1900,
    tm_now.tm_mon + 1,
    tm_now.tm_mday,
    tm_now.tm_hour,
    tm_now.tm_min,
    tm_now.tm_sec,
    static_cast<int>(ms.count())
  );

  return buffer;
}

std::string Logger::getLevelString(Level level) {
  switch (level) {
    case DEBUG:
      return "DEBUG";
    case INFO:
      return "INFO";
    case WARN:
      return "WARN";
    case ERROR:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}
