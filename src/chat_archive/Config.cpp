#include "chat_archive/Config.h"
#include "chat_archive/Logger.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <cstdlib>

namespace chat_archive {

bool Config::load(const std::string& file_path) {
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> config;
    
    // 从文件加载配置
    if (!file_path.empty()) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            CHAT_ARCHIVE_LOG_WARN("Failed to open config file: {}", file_path);
        } else {
            std::string line;
            std::string current_section;
            
            while (std::getline(file, line)) {
                // 去除空白字符
                line.erase(0, line.find_first_not_of(" \t\n\r"));
                line.erase(line.find_last_not_of(" \t\n\r") + 1);
                
                // 跳过空行和注释
                if (line.empty() || line[0] == '#') {
                    continue;
                }
                
                // 处理section
                if (line[0] == '[' && line.back() == ']') {
                    current_section = line.substr(1, line.size() - 2);
                    continue;
                }
                
                // 处理key=value
                size_t eq_pos = line.find('=');
                if (eq_pos == std::string::npos) {
                    continue;
                }
                
                std::string key = line.substr(0, eq_pos);
                std::string value = line.substr(eq_pos + 1);
                
                // 去除key和value的空白字符
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                config[current_section][key] = value;
            }
            
            file.close();
        }
    }
    
    // 从环境变量加载配置（覆盖文件配置）
    auto get_env = [](const char* name) -> std::optional<std::string> {
        const char* value = std::getenv(name);
        return value ? std::optional<std::string>(value) : std::nullopt;
    };
    
    // 服务器配置
    if (auto env = get_env("CHAT_ARCHIVE_PORT")) {
        try {
            port_ = std::stoi(*env);
        } catch (...) {
            CHAT_ARCHIVE_LOG_WARN("Invalid port from environment: {}", *env);
        }
    } else if (config.count("server") && config["server"].count("port")) {
        try {
            port_ = std::stoi(config["server"]["port"]);
        } catch (...) {
            CHAT_ARCHIVE_LOG_WARN("Invalid port from config file: {}", config["server"]["port"]);
        }
    }
    
    // 数据库配置
    if (auto env = get_env("CHAT_ARCHIVE_DB_PATH")) {
        db_path_ = *env;
    } else if (config.count("database") && config["database"].count("path")) {
        db_path_ = config["database"]["path"];
    }
    
    // 日志配置
    if (auto env = get_env("CHAT_ARCHIVE_LOG_LEVEL")) {
        log_level_ = *env;
    } else if (config.count("logging") && config["logging"].count("level")) {
        log_level_ = config["logging"]["level"];
    }
    
    // 验证配置
    if (port_ <= 0 || port_ > 65535) {
        CHAT_ARCHIVE_LOG_ERROR("Invalid port: {}", port_);
        return false;
    }
    
    CHAT_ARCHIVE_LOG_INFO("Config loaded successfully");
    CHAT_ARCHIVE_LOG_DEBUG("Server port: {}", port_);
    CHAT_ARCHIVE_LOG_DEBUG("Database path: {}", db_path_);
    CHAT_ARCHIVE_LOG_DEBUG("Log level: {}", log_level_);
    
    return true;
}

} // namespace chat_archive