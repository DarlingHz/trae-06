#include "config/Config.h"
#include <fstream>
#include <stdexcept>
#include <iostream>

namespace pet_hospital {

Config::Config(const std::string& config_file_path)
    : config_file_path_(config_file_path) {}

bool Config::load() {
    try {
        // 打开配置文件
        std::ifstream config_file(config_file_path_);
        if (!config_file.is_open()) {
            throw std::runtime_error("Failed to open config file: " + config_file_path_);
        }

        // 解析 JSON 配置
        config_file >> config_json_;
        config_file.close();

        // 读取服务器配置
        if (config_json_.contains("server")) {
            const json& server_json = config_json_["server"];
            if (server_json.contains("port")) {
                server_port_ = server_json["port"].get<int>();
            }
            if (server_json.contains("thread_pool_size")) {
                server_thread_pool_size_ = server_json["thread_pool_size"].get<int>();
            }
            if (server_json.contains("max_request_size")) {
                server_max_request_size_ = server_json["max_request_size"].get<int>();
            }
        }

        // 读取数据库配置
        if (config_json_.contains("database")) {
            const json& database_json = config_json_["database"];
            if (database_json.contains("type")) {
                database_type_ = database_json["type"].get<std::string>();
            }
            if (database_json.contains("connection_string")) {
                database_connection_string_ = database_json["connection_string"].get<std::string>();
            }
        }

        // 读取日志配置
        if (config_json_.contains("logging")) {
            const json& logging_json = config_json_["logging"];
            if (logging_json.contains("level")) {
                logging_level_ = parse_log_level(logging_json["level"].get<std::string>());
            }
            if (logging_json.contains("output")) {
                logging_output_ = parse_log_output(logging_json["output"]);
            }
            if (logging_json.contains("file_path")) {
                logging_file_path_ = logging_json["file_path"].get<std::string>();
            }
            if (logging_json.contains("max_file_size")) {
                logging_max_file_size_ = logging_json["max_file_size"].get<int>();
            }
            if (logging_json.contains("max_backup_files")) {
                logging_max_backup_files_ = logging_json["max_backup_files"].get<int>();
            }
        }

        // 读取认证配置
        if (config_json_.contains("authentication")) {
            const json& authentication_json = config_json_["authentication"];
            if (authentication_json.contains("token_expiration_hours")) {
                authentication_token_expiration_hours_ = authentication_json["token_expiration_hours"].get<int>();
            }
            if (authentication_json.contains("token_secret")) {
                authentication_token_secret_ = authentication_json["token_secret"].get<std::string>();
            }
        }

        // 读取缓存配置
        if (config_json_.contains("cache")) {
            const json& cache_json = config_json_["cache"];
            if (cache_json.contains("doctors_ttl_seconds")) {
                cache_doctors_ttl_seconds_ = cache_json["doctors_ttl_seconds"].get<int>();
            }
        }

        // 验证配置有效性
        if (!validate()) {
            throw std::runtime_error("Invalid config");
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load config: " << e.what() << std::endl;
        return false;
    }
}

int Config::get_server_port() const {
    return server_port_;
}

int Config::get_server_thread_pool_size() const {
    return server_thread_pool_size_;
}

int Config::get_server_max_request_size() const {
    return server_max_request_size_;
}

std::string Config::get_database_type() const {
    return database_type_;
}

std::string Config::get_database_connection_string() const {
    return database_connection_string_;
}

Config::LogLevel Config::get_logging_level() const {
    return logging_level_;
}

std::vector<Config::LogOutput> Config::get_logging_output() const {
    return logging_output_;
}

std::string Config::get_logging_file_path() const {
    return logging_file_path_;
}

int Config::get_logging_max_file_size() const {
    return logging_max_file_size_;
}

int Config::get_logging_max_backup_files() const {
    return logging_max_backup_files_;
}

int Config::get_authentication_token_expiration_hours() const {
    return authentication_token_expiration_hours_;
}

std::string Config::get_authentication_token_secret() const {
    return authentication_token_secret_;
}

int Config::get_cache_doctors_ttl_seconds() const {
    return cache_doctors_ttl_seconds_;
}

Config::LogLevel Config::parse_log_level(const std::string& level_str) const {
    if (level_str == "debug") {
        return LogLevel::DEBUG;
    } else if (level_str == "info") {
        return LogLevel::INFO;
    } else if (level_str == "warn") {
        return LogLevel::WARN;
    } else if (level_str == "error") {
        return LogLevel::ERROR;
    } else if (level_str == "fatal") {
        return LogLevel::FATAL;
    } else {
        throw std::invalid_argument("Invalid log level: " + level_str);
    }
}

std::vector<Config::LogOutput> Config::parse_log_output(const json& output_json) const {
    std::vector<LogOutput> output;
    for (const auto& output_item : output_json) {
        std::string output_str = output_item.get<std::string>();
        if (output_str == "console") {
            output.push_back(LogOutput::CONSOLE);
        } else if (output_str == "file") {
            output.push_back(LogOutput::FILE);
        } else {
            throw std::invalid_argument("Invalid log output: " + output_str);
        }
    }
    return output;
}

bool Config::validate() const {
    // 验证服务器端口
    if (server_port_ < 1 || server_port_ > 65535) {
        std::cerr << "Invalid server port: " << server_port_ << std::endl;
        return false;
    }

    // 验证线程池大小
    if (server_thread_pool_size_ < 1) {
        std::cerr << "Invalid thread pool size: " << server_thread_pool_size_ << std::endl;
        return false;
    }

    // 验证最大请求大小
    if (server_max_request_size_ < 1) {
        std::cerr << "Invalid max request size: " << server_max_request_size_ << std::endl;
        return false;
    }

    // 验证数据库配置
    if (database_type_.empty()) {
        std::cerr << "Database type is empty" << std::endl;
        return false;
    }
    if (database_connection_string_.empty()) {
        std::cerr << "Database connection string is empty" << std::endl;
        return false;
    }

    // 验证认证配置
    if (authentication_token_expiration_hours_ < 1) {
        std::cerr << "Invalid token expiration hours: " << authentication_token_expiration_hours_ << std::endl;
        return false;
    }
    if (authentication_token_secret_.empty()) {
        std::cerr << "Authentication token secret is empty" << std::endl;
        return false;
    }

    // 验证缓存配置
    if (cache_doctors_ttl_seconds_ < 0) {
        std::cerr << "Invalid doctors cache TTL: " << cache_doctors_ttl_seconds_ << std::endl;
        return false;
    }

    return true;
}

} // namespace pet_hospital
