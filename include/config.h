#pragma once
#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include "json_utils.h"

struct ServerConfig {
    int port = 8080;
    std::string host = "0.0.0.0";
};

struct DatabaseConfig {
    std::string path = "lost_and_found.db";
};

struct LogConfig {
    std::string level = "info";
    std::string file = "app.log";
};

struct AppConfig {
    ServerConfig server;
    DatabaseConfig database;
    LogConfig log;
};

class ConfigManager {
private:
    AppConfig config_;
    ConfigManager() = default;
    
public:
    static ConfigManager& instance() {
        static ConfigManager instance;
        return instance;
    }
    
    bool load_config(const std::string& file_path);
    const AppConfig& get_config() const;
};

#endif // CONFIG_H