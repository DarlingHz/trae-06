#pragma once

#include <string>
#include <map>
#include <cstdlib>

class Config {
public:
    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    void load() {
        // 数据库配置
        dbHost = getEnvOrDefault("DB_HOST", "localhost");
        dbPort = std::stoi(getEnvOrDefault("DB_PORT", "3306"));
        dbName = getEnvOrDefault("DB_NAME", "device_warranty");
        dbUser = getEnvOrDefault("DB_USER", "root");
        dbPassword = getEnvOrDefault("DB_PASSWORD", "");

        // Redis配置
        redisHost = getEnvOrDefault("REDIS_HOST", "localhost");
        redisPort = std::stoi(getEnvOrDefault("REDIS_PORT", "6379"));
        redisPassword = getEnvOrDefault("REDIS_PASSWORD", "");
        redisDb = std::stoi(getEnvOrDefault("REDIS_DB", "0"));

        // HTTP服务配置
        httpHost = getEnvOrDefault("HTTP_HOST", "0.0.0.0");
        httpPort = std::stoi(getEnvOrDefault("HTTP_PORT", "8080"));

        // 缓存配置
        cacheExpireSeconds = std::stoi(getEnvOrDefault("CACHE_EXPIRE", "60"));
    }

    std::string dbHost;
    int dbPort;
    std::string dbName;
    std::string dbUser;
    std::string dbPassword;

    std::string redisHost;
    int redisPort;
    std::string redisPassword;
    int redisDb;

    std::string httpHost;
    int httpPort;

    int cacheExpireSeconds;

private:
    Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    std::string getEnvOrDefault(const std::string& key, const std::string& defaultValue) {
        const char* value = std::getenv(key.c_str());
        return value ? value : defaultValue;
    }
};
