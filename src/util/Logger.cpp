#include "Logger.h"
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

// 初始化静态成员变量
std::ofstream Logger::log_file_;
std::mutex Logger::mutex_;
Logger::Level Logger::level_ = INFO;
std::string Logger::file_path_;
long Logger::max_file_size_ = 10485760L;
int Logger::max_backup_files_ = 5;
long Logger::current_file_size_ = 0L;

bool Logger::init(const std::string& file_path, Level level, long max_file_size, int max_backup_files) {
    try {
        // 关闭已打开的日志文件
        if (log_file_.is_open()) {
            log_file_.close();
        }
        
        // 设置成员变量
        file_path_ = file_path;
        level_ = level;
        max_file_size_ = max_file_size;
        max_backup_files_ = max_backup_files;
        current_file_size_ = 0L;
        
        // 创建日志文件所在的目录
        fs::path dir_path = fs::path(file_path).parent_path();
        if (!fs::exists(dir_path)) {
            if (!fs::create_directories(dir_path)) {
                std::cerr << "Failed to create log directory: " << dir_path.string() << std::endl;
                return false;
            }
        }
        
        // 打开日志文件（追加模式）
        log_file_.open(file_path, std::ios::out | std::ios::app | std::ios::binary);
        if (!log_file_.is_open()) {
            std::cerr << "Failed to open log file: " << file_path << std::endl;
            return false;
        }
        
        // 获取当前日志文件大小
        log_file_.seekp(0, std::ios::end);
        current_file_size_ = log_file_.tellp();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize logger: " << e.what() << std::endl;
        return false;
    }
}

void Logger::debug(const std::string& message) {
    log(DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(INFO, message);
}

void Logger::warn(const std::string& message) {
    log(WARN, message);
}

void Logger::error(const std::string& message) {
    log(ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(FATAL, message);
}

void Logger::setLevel(Level level) {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

void Logger::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

void Logger::log(Level level, const std::string& message) {
    // 检查日志级别
    if (level < level_) {
        return;
    }
    
    // 加锁保证线程安全
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查日志文件是否打开
    if (!log_file_.is_open()) {
        std::cerr << "Log file is not open, cannot log message: " << message << std::endl;
        return;
    }
    
    // 检查是否需要滚动日志文件
    if (current_file_size_ >= max_file_size_) {
        if (!rollLogFile()) {
            std::cerr << "Failed to roll log file, cannot log message: " << message << std::endl;
            return;
        }
    }
    
    // 格式化日志信息
    std::time_t now = std::time(nullptr);
    std::string time_str = formatTime(now);
    std::string level_str = formatLevel(level);
    std::string log_line = time_str + " [" + level_str + "] " + message + "\n";
    
    // 写入日志文件
    log_file_.write(log_line.c_str(), log_line.size());
    if (!log_file_.good()) {
        std::cerr << "Failed to write log message: " << message << std::endl;
        return;
    }
    
    // 刷新日志文件
    log_file_.flush();
    
    // 更新当前日志文件大小
    current_file_size_ += log_line.size();
}

bool Logger::rollLogFile() {
    try {
        // 关闭当前日志文件
        log_file_.close();
        
        // 生成备份文件路径
        fs::path log_path(file_path_);
        std::string log_name = log_path.stem().string();
        std::string log_ext = log_path.extension().string();
        fs::path log_dir = log_path.parent_path();
        
        // 删除最旧的备份文件（如果超过最大备份文件数）
        std::vector<fs::path> backup_files;
        for (const auto& entry : fs::directory_iterator(log_dir)) {
            if (entry.is_regular_file()) {
                fs::path entry_path = entry.path();
                std::string entry_name = entry_path.stem().string();
                std::string entry_ext = entry_path.extension().string();
                
                // 检查是否是当前日志文件的备份文件
                if (entry_ext == log_ext && entry_name.find(log_name + ".") == 0) {
                    backup_files.push_back(entry_path);
                }
            }
        }
        
        // 按修改时间排序备份文件（最旧的在前）
        std::sort(backup_files.begin(), backup_files.end(), [](const fs::path& a, const fs::path& b) {
            return fs::last_write_time(a) < fs::last_write_time(b);
        });
        
        // 删除超过最大备份文件数的最旧备份文件
        while (backup_files.size() >= static_cast<size_t>(max_backup_files_)) {
            if (!fs::remove(backup_files.front())) {
                std::cerr << "Failed to remove old backup file: " << backup_files.front().string() << std::endl;
                // 继续删除其他备份文件
            }
            backup_files.erase(backup_files.begin());
        }
        
        // 重命名当前日志文件为备份文件
        std::string backup_file_name = log_name + "." + std::to_string(std::time(nullptr)) + log_ext;
        fs::path backup_file_path = log_dir / backup_file_name;
        try {
            fs::rename(log_path, backup_file_path);
        } catch (const std::exception& e) {
            std::cerr << "Failed to rename log file to backup: " << backup_file_path.string() << std::endl;
            // 尝试重新打开原始日志文件
            log_file_.open(log_path, std::ios::out | std::ios::app | std::ios::binary);
            if (!log_file_.is_open()) {
                std::cerr << "Failed to reopen log file: " << log_path.string() << std::endl;
                return false;
            }
            return true;
        }
        
        // 创建新的日志文件
        log_file_.open(log_path, std::ios::out | std::ios::app | std::ios::binary);
        if (!log_file_.is_open()) {
            std::cerr << "Failed to create new log file: " << log_path.string() << std::endl;
            // 尝试重新打开备份文件作为当前日志文件
            log_file_.open(backup_file_path, std::ios::out | std::ios::app | std::ios::binary);
            if (!log_file_.is_open()) {
                std::cerr << "Failed to reopen backup file as log file: " << backup_file_path.string() << std::endl;
                return false;
            }
            return true;
        }
        
        // 重置当前日志文件大小
        current_file_size_ = 0L;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to roll log file: " << e.what() << std::endl;
        return false;
    }
}

std::string Logger::formatTime(const std::time_t& time) {
    char buffer[32];
    std::tm tm = *std::localtime(&time);
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buffer);
}

std::string Logger::formatLevel(Level level) {
    switch (level) {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case WARN:
            return "WARN";
        case ERROR:
            return "ERROR";
        case FATAL:
            return "FATAL";
        default:
            return "UNKNOWN";
    }
}