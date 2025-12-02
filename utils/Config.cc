#include "Config.h"
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace giftcard {

bool Config::load(const std::string& config_file_path) {
    try {
        YAML::Node config = YAML::LoadFile(config_file_path);
        
        // 解析应用程序配置
        if (config["app"]) {
            auto app_config = config["app"];
            if (app_config["port"]) app_config_.port = app_config["port"].as<int>();
            if (app_config["workers"]) app_config_.workers = app_config["workers"].as<int>();
            if (app_config["max_connections"]) app_config_.max_connections = app_config["max_connections"].as<int>();
            if (app_config["log_level"]) app_config_.log_level = app_config["log_level"].as<std::string>();
        }
        
        // 解析数据库配置
        if (config["database"]) {
            auto database_config = config["database"];
            if (database_config["host"]) database_config_.host = database_config["host"].as<std::string>();
            if (database_config["port"]) database_config_.port = database_config["port"].as<int>();
            if (database_config["user"]) database_config_.user = database_config["user"].as<std::string>();
            if (database_config["password"]) database_config_.password = database_config["password"].as<std::string>();
            if (database_config["dbname"]) database_config_.dbname = database_config["dbname"].as<std::string>();
            if (database_config["max_connections"]) database_config_.max_connections = database_config["max_connections"].as<int>();
            if (database_config["charset"]) database_config_.charset = database_config["charset"].as<std::string>();
        }
        
        // 解析Redis配置
        if (config["redis"]) {
            auto redis_config = config["redis"];
            if (redis_config["host"]) redis_config_.host = redis_config["host"].as<std::string>();
            if (redis_config["port"]) redis_config_.port = redis_config["port"].as<int>();
            if (redis_config["password"]) redis_config_.password = redis_config["password"].as<std::string>();
            if (redis_config["db"]) redis_config_.db = redis_config["db"].as<int>();
            if (redis_config["max_connections"]) redis_config_.max_connections = redis_config["max_connections"].as<int>();
            if (redis_config["timeout"]) redis_config_.timeout = redis_config["timeout"].as<int>();
        }
        
        // 解析业务配置
        if (config["business"]) {
            auto business_config = config["business"];
            if (business_config["giftcard_lock_ttl"]) business_config_.giftcard_lock_ttl = business_config["giftcard_lock_ttl"].as<int>();
            if (business_config["idempotency_key_ttl"]) business_config_.idempotency_key_ttl = business_config["idempotency_key_ttl"].as<int>();
            
            if (business_config["rate_limit"]) {
                auto rate_limit_config = business_config["rate_limit"];
                if (rate_limit_config["enabled"]) business_config_.rate_limit.enabled = rate_limit_config["enabled"].as<bool>();
                if (rate_limit_config["user_requests_per_second"]) business_config_.rate_limit.user_requests_per_second = rate_limit_config["user_requests_per_second"].as<int>();
                if (rate_limit_config["ip_requests_per_second"]) business_config_.rate_limit.ip_requests_per_second = rate_limit_config["ip_requests_per_second"].as<int>();
            }
        }
        
        return true;
    } catch (const YAML::Exception& e) {
        std::cerr << "Failed to load config file: " << e.what() << std::endl;
        return false;
    }
}

bool Config::parseJson(const json& j) {
    try {
        // 解析应用程序配置
        if (j.contains("app")) {
            auto app_config = j["app"];
            if (app_config.contains("port")) app_config_.port = app_config["port"];
            if (app_config.contains("workers")) app_config_.workers = app_config["workers"];
            if (app_config.contains("max_connections")) app_config_.max_connections = app_config["max_connections"];
            if (app_config.contains("log_level")) app_config_.log_level = app_config["log_level"];
        }
        
        // 解析数据库配置
        if (j.contains("database")) {
            auto database_config = j["database"];
            if (database_config.contains("host")) database_config_.host = database_config["host"];
            if (database_config.contains("port")) database_config_.port = database_config["port"];
            if (database_config.contains("user")) database_config_.user = database_config["user"];
            if (database_config.contains("password")) database_config_.password = database_config["password"];
            if (database_config.contains("dbname")) database_config_.dbname = database_config["dbname"];
            if (database_config.contains("max_connections")) database_config_.max_connections = database_config["max_connections"];
            if (database_config.contains("charset")) database_config_.charset = database_config["charset"];
        }
        
        // 解析Redis配置
        if (j.contains("redis")) {
            auto redis_config = j["redis"];
            if (redis_config.contains("host")) redis_config_.host = redis_config["host"];
            if (redis_config.contains("port")) redis_config_.port = redis_config["port"];
            if (redis_config.contains("password")) redis_config_.password = redis_config["password"];
            if (redis_config.contains("db")) redis_config_.db = redis_config["db"];
            if (redis_config.contains("max_connections")) redis_config_.max_connections = redis_config["max_connections"];
            if (redis_config.contains("timeout")) redis_config_.timeout = redis_config["timeout"];
        }
        
        // 解析业务配置
        if (j.contains("business")) {
            auto business_config = j["business"];
            if (business_config.contains("giftcard_lock_ttl")) business_config_.giftcard_lock_ttl = business_config["giftcard_lock_ttl"];
            if (business_config.contains("idempotency_key_ttl")) business_config_.idempotency_key_ttl = business_config["idempotency_key_ttl"];
            
            if (business_config.contains("rate_limit")) {
                auto rate_limit_config = business_config["rate_limit"];
                if (rate_limit_config.contains("enabled")) business_config_.rate_limit.enabled = rate_limit_config["enabled"];
                if (rate_limit_config.contains("user_requests_per_second")) business_config_.rate_limit.user_requests_per_second = rate_limit_config["user_requests_per_second"];
                if (rate_limit_config.contains("ip_requests_per_second")) business_config_.rate_limit.ip_requests_per_second = rate_limit_config["ip_requests_per_second"];
            }
        }
        
        return true;
    } catch (const json::exception& e) {
        std::cerr << "Failed to parse JSON config: " << e.what() << std::endl;
        return false;
    }
}

} // namespace giftcard
