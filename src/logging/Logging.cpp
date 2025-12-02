#include "logging/Logging.h"
#include <iostream>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace pet_hospital {

// 全局日志对象定义
Logging g_logger;

Logging::~Logging() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

bool Logging::init(const Config& config) {
    try {
        // 设置日志级别
        level_ = config.get_logging_level();

        // 设置日志输出
        output_ = config.get_logging_output();

        // 设置日志文件路径
        file_path_ = config.get_logging_file_path();

        // 设置日志文件大小限制
        max_file_size_ = config.get_logging_max_file_size();

        // 设置日志文件备份数量
        max_backup_files_ = config.get_logging_max_backup_files();

        // 如果需要输出到文件，打开日志文件
        if (std::find(output_.begin(), output_.end(), LogOutput::FILE) != output_.end()) {
            // 创建日志文件目录（如果不存在）
            std::filesystem::path log_file_path(file_path_);
            std::filesystem::create_directories(log_file_path.parent_path());

            // 打开日志文件（追加模式）
            log_file_.open(file_path_, std::ios::out | std::ios::app);
            if (!log_file_.is_open()) {
                throw std::runtime_error("Failed to open log file: " + file_path_);
            }
        }

        LOG_INFO("Logging system initialized successfully");
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize logging system: " << e.what() << std::endl;
        return false;
    }
}

void Logging::debug(const std::string& message, const std::string& file, int line) {
    log(LogLevel::DEBUG, message, file, line);
}

void Logging::info(const std::string& message, const std::string& file, int line) {
    log(LogLevel::INFO, message, file, line);
}

void Logging::warn(const std::string& message, const std::string& file, int line) {
    log(LogLevel::WARN, message, file, line);
}

void Logging::error(const std::string& message, const std::string& file, int line) {
    log(LogLevel::ERROR, message, file, line);
}

void Logging::fatal(const std::string& message, const std::string& file, int line) {
    log(LogLevel::FATAL, message, file, line);
}

void Logging::log(LogLevel level, const std::string& message, const std::string& file, int line) {
    // 检查日志级别是否启用
    if (level < level_) {
        return;
    }

    // 格式化日志消息
    std::string formatted_message = format_message(level, message, file, line);

    // 加锁保证线程安全
    std::lock_guard<std::mutex> lock(log_mutex_);

    // 输出到控制台
    if (std::find(output_.begin(), output_.end(), LogOutput::CONSOLE) != output_.end()) {
        // 根据日志级别设置控制台颜色
        switch (level) {
            case LogLevel::DEBUG:
                std::cout << "\033[36m" << formatted_message << "\033[0m" << std::endl;
                break;
            case LogLevel::INFO:
                std::cout << "\033[32m" << formatted_message << "\033[0m" << std::endl;
                break;
            case LogLevel::WARN:
                std::cout << "\033[33m" << formatted_message << "\033[0m" << std::endl;
                break;
            case LogLevel::ERROR:
                std::cout << "\033[31m" << formatted_message << "\033[0m" << std::endl;
                break;
            case LogLevel::FATAL:
                std::cout << "\033[1;31m" << formatted_message << "\033[0m" << std::endl;
                break;
        }
    }

    // 输出到文件
    if (std::find(output_.begin(), output_.end(), LogOutput::FILE) != output_.end()) {
        // 检查日志文件大小，如果超过限制则滚动日志文件
        if (check_log_file_size()) {
            roll_log_file();
        }

        // 写入日志文件
        log_file_ << formatted_message << std::endl;
        log_file_.flush();
    }
}

std::string Logging::format_message(LogLevel level, const std::string& message, const std::string& file, int line) const {
    // 获取当前时间
    auto now = std::chrono::system_clock::now();

    // 格式化时间戳
    std::string timestamp = format_timestamp(now);

    // 获取日志级别字符串
    std::string level_str = get_level_string(level);

    // 格式化日志消息
    std::string formatted_message = timestamp + " [" + level_str + "] ";

    // 如果提供了文件和行号，添加到日志消息
    if (!file.empty() && line > 0) {
        // 只保留文件名（去掉路径）
        std::filesystem::path file_path(file);
        std::string file_name = file_path.filename().string();

        formatted_message += "[" + file_name + ":" + std::to_string(line) + "] ";
    }

    // 添加日志消息内容
    formatted_message += message;

    return formatted_message;
}

std::string Logging::format_timestamp(const std::chrono::system_clock::time_point& time_point) const {
    // 将时间点转换为时间_t
    std::time_t time_t_now = std::chrono::system_clock::to_time_t(time_point);

    // 转换为本地时间
    std::tm tm_now = *std::localtime(&time_t_now);

    // 获取毫秒部分
    auto duration = time_point.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

    // 格式化时间戳
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << milliseconds;

    return oss.str();
}

std::string Logging::get_level_string(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::FATAL:
            return "FATAL";
        default:
            return "UNKNOWN";
    }
}

void Logging::roll_log_file() {
    // 关闭当前日志文件
    if (log_file_.is_open()) {
        log_file_.close();
    }

    // 滚动日志文件（重命名旧日志文件）
    for (int i = max_backup_files_ - 1; i >= 1; --i) {
        std::string old_file_path = file_path_ + "." + std::to_string(i);
        std::string new_file_path = file_path_ + "." + std::to_string(i + 1);

        // 如果旧日志文件存在，重命名为新的日志文件
        if (std::filesystem::exists(old_file_path)) {
            std::filesystem::rename(old_file_path, new_file_path);
        }
    }

    // 重命名当前日志文件为 .1
    if (std::filesystem::exists(file_path_)) {
        std::string new_file_path = file_path_ + ".1";
        std::filesystem::rename(file_path_, new_file_path);
    }

    // 打开新的日志文件
    log_file_.open(file_path_, std::ios::out | std::ios::app);
    if (!log_file_.is_open()) {
        LOG_ERROR("Failed to open new log file: " + file_path_);
    }
}

bool Logging::check_log_file_size() const {
    // 检查日志文件是否存在
    if (!std::filesystem::exists(file_path_)) {
        return false;
    }

    // 获取日志文件大小
    std::uintmax_t file_size = std::filesystem::file_size(file_path_);

    // 检查日志文件大小是否超过限制
    return file_size >= static_cast<std::uintmax_t>(max_file_size_);
}

} // namespace pet_hospital
