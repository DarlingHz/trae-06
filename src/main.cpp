#include <iostream>
#include <string>
#include <cstdlib>

#include "db/db_pool.h"
#include "services/user_service.h"
#include "services/device_service.h"
#include "services/warranty_service.h"
#include "services/service_center_service.h"
#include "services/repair_service.h"
#include "services/cache_service.h"
#include "controllers/http_server.h"
#include "utils/logger.h"
#include "utils/config.h"

int main(int argc, char* argv[]) {
    try {
        // 初始化日志系统
        Logger::init(Logger::Level::INFO, true);
        LOG_INFO("Starting Device Warranty Management System...");

        // 加载配置
        Config& config = Config::getInstance();
        
        // 初始化数据库连接池
        DBPool& dbPool = DBPool::getInstance();
        dbPool.init(
            config.get("DB_HOST", "localhost"),
            config.get("DB_PORT", 3306),
            config.get("DB_USER", "root"),
            config.get("DB_PASSWORD", ""),
            config.get("DB_NAME", "warranty_db"),
            std::stoi(config.get("DB_POOL_SIZE", "5"))
        );
        LOG_INFO("Database connection pool initialized successfully");

        // 初始化Redis缓存
        if (config.get("REDIS_ENABLED", "false") == "true") {
            CacheService& cacheService = CacheService::getInstance();
            bool initSuccess = cacheService.init(
                config.get("REDIS_HOST", "localhost"),
                std::stoi(config.get("REDIS_PORT", "6379")),
                config.get("REDIS_PASSWORD", ""),
                std::stoi(config.get("REDIS_DB", "0"))
            );
            
            if (initSuccess) {
                LOG_INFO("Redis cache initialized successfully");
            } else {
                LOG_WARNING("Redis cache initialization failed");
            }
        }

        // 初始化HTTP服务器
        HttpServer server;
        int port = std::stoi(config.get("HTTP_PORT", "8080"));
        server.init(port);
        
        LOG_INFO("Server started successfully on port %d", port);
        
        // 保持服务器运行
        std::cout << "Press Enter to stop the server..." << std::endl;
        std::cin.get();
        
        LOG_INFO("Shutting down server...");
        return 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Server initialization failed: %s", e.what());
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
