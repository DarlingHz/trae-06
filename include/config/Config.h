#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <nlohmann/json.hpp>

namespace event_signup_service::config {

struct ServiceConfig {
    int port = 8080;
    std::string host = "0.0.0.0";
    std::string log_level = "info";
};

struct DatabaseConfig {
    std::string path = "event_signup.db";
};

struct AppConfig {
    ServiceConfig service;
    DatabaseConfig database;
};

class Config {
private:
    static AppConfig instance_;
    static bool initialized_;

    Config() = default;

public:
    // 禁止拷贝
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    // 初始化配置
    static void initialize(const std::string& config_path = "config/config.json");

    // 获取配置实例
    static const AppConfig& get() {
        if (!initialized_) {
            throw std::runtime_error("配置未初始化");
        }
        return instance_;
    }
};

} // namespace event_signup_service::config

#endif // CONFIG_H
