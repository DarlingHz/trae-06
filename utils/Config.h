#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace giftcard {

class Config {
public:
    // 应用程序配置
    struct AppConfig {
        int port = 8080;
        int workers = 4;
        int max_connections = 1000;
        std::string log_level = "info";
    };

    // 数据库配置
    struct DatabaseConfig {
        std::string host = "127.0.0.1";
        int port = 3306;
        std::string user = "root";
        std::string password = "password";
        std::string dbname = "giftcard_db";
        int max_connections = 20;
        std::string charset = "utf8mb4";
    };

    // Redis配置
    struct RedisConfig {
        std::string host = "127.0.0.1";
        int port = 6379;
        std::string password = "";
        int db = 0;
        int max_connections = 10;
        int timeout = 5000;
    };

    // 业务配置
    struct BusinessConfig {
        int giftcard_lock_ttl = 300;
        int idempotency_key_ttl = 86400;
        
        struct RateLimitConfig {
            bool enabled = true;
            int user_requests_per_second = 10;
            int ip_requests_per_second = 100;
        } rate_limit;
    };

    // 单例模式
    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    // 加载配置文件
    bool load(const std::string& config_file_path);

    // 获取应用程序配置
    const AppConfig& getAppConfig() const { return app_config_; }

    // 获取数据库配置
    const DatabaseConfig& getDatabaseConfig() const { return database_config_; }

    // 获取Redis配置
    const RedisConfig& getRedisConfig() const { return redis_config_; }

    // 获取业务配置
    const BusinessConfig& getBusinessConfig() const { return business_config_; }

private:
    Config() = default;
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    // 解析JSON配置
    bool parseJson(const json& j);

    AppConfig app_config_;
    DatabaseConfig database_config_;
    RedisConfig redis_config_;
    BusinessConfig business_config_;
};

} // namespace giftcard

#endif // CONFIG_H
