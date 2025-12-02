#include <drogon/drogon.h>
#include <utils/Config.h>
#include <utils/DatabasePool.h>
#include <utils/RedisPool.h>
#include <utils/Logger.h>
#include <iostream>
#include <string>

using namespace giftcard;

int main(int argc, char** argv) {
    try {
        // 初始化配置
        auto& config = Config::getInstance();
        if (!config.load("config.yaml")) {
            std::cerr << "加载配置文件失败" << std::endl;
            return 1;
        }

        // 初始化日志
        auto& logger = Logger::getInstance();
        LOG_INFO("礼品卡管理系统启动");

        // 初始化数据库连接池
        auto& db_pool = DatabasePool::getInstance();
        const Config::DatabaseConfig& db_config = config.getDatabaseConfig();
        if (!db_pool.init(db_config.host, db_config.port, db_config.user, db_config.password, db_config.dbname, db_config.max_connections, db_config.charset)) {
            LOG_ERROR("初始化数据库连接池失败");
            return 1;
        }
        LOG_INFO("数据库连接池初始化成功");

        // 初始化Redis连接池
        auto& redis_pool = RedisPool::getInstance();
        const Config::RedisConfig& redis_config = config.getRedisConfig();
        if (!redis_pool.init(redis_config.host, redis_config.port, redis_config.password, redis_config.db, redis_config.max_connections)) {
            LOG_ERROR("初始化Redis连接池失败");
            return 1;
        }
        LOG_INFO("Redis连接池初始化成功");

        // 初始化Drogon服务器
        auto& drogon_app = drogon::app();

        // 设置服务器配置
        auto& app_config = config.getAppConfig();
        
        // 设置日志级别
        if (app_config.log_level == "trace") {
            drogon_app.setLogLevel(trantor::Logger::kTrace);
        } else if (app_config.log_level == "debug") {
            drogon_app.setLogLevel(trantor::Logger::kDebug);
        } else if (app_config.log_level == "info") {
            drogon_app.setLogLevel(trantor::Logger::kInfo);
        } else if (app_config.log_level == "warn") {
            drogon_app.setLogLevel(trantor::Logger::kWarn);
        } else if (app_config.log_level == "error") {
            drogon_app.setLogLevel(trantor::Logger::kError);
        } else if (app_config.log_level == "fatal") {
            drogon_app.setLogLevel(trantor::Logger::kFatal);
        } else {
            drogon_app.setLogLevel(trantor::Logger::kInfo);
        }
        
        drogon_app.setThreadNum(app_config.workers);

        // 启动服务器
        LOG_INFO("服务器启动，监听端口: {}, 工作线程数: {}", 
                 app_config.port, app_config.workers);

        drogon_app.run();

        // 关闭连接池
        db_pool.close();
        redis_pool.close();

        LOG_INFO("礼品卡管理系统关闭");

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "系统启动失败: " << e.what() << std::endl;
        return 1;
    }
}
