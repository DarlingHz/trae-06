#include "config.h"
#include "database.h"
#include "lru_cache.h"
#include "service.h"
#include "router.h"
#include <httplib.h>
#include <iostream>
#include <memory>
#include <string>

int main() {
    try {
        // 1. 加载配置文件
        std::cout << "Loading configuration..." << std::endl;
        Config config;
        config.load("config/config.json");

        // 2. 初始化数据库连接
        std::cout << "Initializing database..." << std::endl;
        std::shared_ptr<Database> database = std::make_shared<Database>(config.getDbPath());

        // 3. 创建LRU缓存
        std::cout << "Creating LRU cache..." << std::endl;
        using CacheKey = std::pair<int, int>; // (document_id, version_number)
        using CacheValue = DocumentVersion; // DocumentVersion object
        std::shared_ptr<LRUCache<CacheKey, CacheValue>> cache = 
            std::shared_ptr<LRUCache<CacheKey, CacheValue>>(new LRUCache<CacheKey, CacheValue>(static_cast<size_t>(config.getCacheCapacity())));

        // 4. 初始化业务逻辑层
        std::cout << "Initializing service layer..." << std::endl;
        std::shared_ptr<Service> service = std::make_shared<Service>(database, cache);

        // 5. 初始化路由层
        std::cout << "Initializing router..." << std::endl;
        Router router(service);

        // 6. 启动HTTP服务器
        std::cout << "Starting HTTP server..." << std::endl;
        httplib::Server server;
        router.init(server);

        // 设置多线程
        server.set_mount_point("/", "./");
        // 注意：httplib库的线程池大小是在编译时通过CPPHTTPLIB_THREAD_POOL_COUNT宏定义的
        // server.set_max_threads(config.getMaxThreads()); // 此方法可能不存在或已更改

        std::cout << "Server is running on port " << config.getPort() << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;

        if (!server.listen("0.0.0.0", config.getPort())) {
            std::cerr << "Failed to start server on port " << config.getPort() << std::endl;
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}