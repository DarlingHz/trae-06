#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <ctime>

enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

class Logger {
private:
    std::ofstream log_file_;
    LogLevel current_level_ = INFO;
    std::mutex mutex_;
    Logger() = default;
    
    std::string get_level_string(LogLevel level);
    std::string get_timestamp();
    
public:
    static Logger& instance() {
        static Logger instance;
        return instance;
    }
    
    bool init(const std::string& file_path, LogLevel level = INFO);
    void log(LogLevel level, const std::string& message);
    
    static void debug(const std::string& message) {
        instance().log(DEBUG, message);
    }
    
    static void info(const std::string& message) {
        instance().log(INFO, message);
    }
    
    static void warn(const std::string& message) {
        instance().log(WARN, message);
    }
    
    static void error(const std::string& message) {
        instance().log(ERROR, message);
    }
    
    static void fatal(const std::string& message) {
        instance().log(FATAL, message);
    }
};

#endif // LOGGER_H