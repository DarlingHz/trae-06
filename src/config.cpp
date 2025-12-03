#include "config.h"
#include "json_utils.h"
#include <fstream>
#include <stdexcept>

bool ConfigManager::load_config(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + file_path);
    }
    
    std::string json_str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    json::JsonValue j = json::Parser::parse(json_str);
    
    // 解析server配置
    if (j.has("server")) {
        const auto& server = j.get("server");
        if (server.has("port")) {
            config_.server.port = server.get("port").as_int();
        }
        if (server.has("host")) {
            config_.server.host = server.get("host").as_string();
        }
    }
    
    // 解析database配置
    if (j.has("database")) {
        const auto& db = j.get("database");
        if (db.has("path")) {
            config_.database.path = db.get("path").as_string();
        }
    }
    
    // 解析log配置
    if (j.has("log")) {
        const auto& log = j.get("log");
        if (log.has("level")) {
            config_.log.level = log.get("level").as_string();
        }
        if (log.has("file")) {
            config_.log.file = log.get("file").as_string();
        }
    }
    
    return true;
}

const AppConfig& ConfigManager::get_config() const {
    return config_;
}