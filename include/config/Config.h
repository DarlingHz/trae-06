#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace pet_hospital {

class Config {
public:
    enum class LogLevel {
        DEBUG = 0,
        INFO = 1,
        WARN = 2,
        ERROR = 3,
        FATAL = 4
    };

    enum class LogOutput {
        CONSOLE = 0,
        FILE = 1
    };

    Config() = default;
    explicit Config(const std::string& config_file_path);
    ~Config() = default;

    // 加载并解析配置文件
    bool load();

    // 服务器配置
    int get_server_port() const;
    int get_server_thread_pool_size() const;
    int get_server_max_request_size() const;

    // 数据库配置
    std::string get_database_type() const;
    std::string get_database_connection_string() const;

    // 日志配置
    LogLevel get_logging_level() const;
    std::vector<LogOutput> get_logging_output() const;
    std::string get_logging_file_path() const;
    int get_logging_max_file_size() const;
    int get_logging_max_backup_files() const;

    // 认证配置
    int get_authentication_token_expiration_hours() const;
    std::string get_authentication_token_secret() const;

    // 缓存配置
    int get_cache_doctors_ttl_seconds() const;

private:
    // 解析日志级别
    LogLevel parse_log_level(const std::string& level_str) const;

    // 解析日志输出
    std::vector<LogOutput> parse_log_output(const json& output_json) const;

    // 验证配置的有效性
    bool validate() const;

private:
    std::string config_file_path_;
    json config_json_;

    // 服务器配置
    int server_port_ = 8080;
    int server_thread_pool_size_ = 4;
    int server_max_request_size_ = 1048576;

    // 数据库配置
    std::string database_type_ = "sqlite";
    std::string database_connection_string_ = "./pet_hospital.db";

    // 日志配置
    LogLevel logging_level_ = LogLevel::INFO;
    std::vector<LogOutput> logging_output_ = {LogOutput::CONSOLE};
    std::string logging_file_path_ = "./pet_hospital.log";
    int logging_max_file_size_ = 10485760;
    int logging_max_backup_files_ = 5;

    // 认证配置
    int authentication_token_expiration_hours_ = 24;
    std::string authentication_token_secret_ = "pet_hospital_secret_key";

    // 缓存配置
    int cache_doctors_ttl_seconds_ = 300;
};

} // namespace pet_hospital
