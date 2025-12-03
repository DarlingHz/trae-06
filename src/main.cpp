#include <iostream>
#include <memory>
#include <csignal>
#include <atomic>

#include "repository/user_repository.h"
#include "repository/announcement_repository.h"
#include "repository/read_receipt_repository.h"
#include "services/user_service.h"
#include "services/announcement_service.h"
#include "services/read_receipt_service.h"
#include "cache/cache_manager.h"
#include "auth/auth.h"
#include "auth/middleware.h"
#include "controller/user_controller.h"
#include "controller/announcement_controller.h"
#include "http/router.h"

std::atomic<bool> g_running(true);

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nReceived termination signal. Shutting down server..." << std::endl;
        g_running = false;
    }
}

int main(int argc, char* argv[]) {
    try {
        // 注册信号处理
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
        
        std::cout << "Starting announcement system..." << std::endl;
        
        // 1. 初始化数据库
        const std::string db_path = "announcements.db";
        std::cout << "Connecting to database: " << db_path << std::endl;
        
        // 创建数据库表
        UserRepository::create_table(db_path);
        AnnouncementRepository::create_table(db_path);
        ReadReceiptRepository::create_table(db_path);
        
        std::cout << "Database tables initialized successfully" << std::endl;
        
        // 2. 创建 repository 实例
        auto user_repo = std::make_shared<UserRepository>(db_path);
        auto announcement_repo = std::make_shared<AnnouncementRepository>(db_path);
        auto read_receipt_repo = std::make_shared<ReadReceiptRepository>(db_path);
        
        // 3. 创建缓存管理器
        auto cache_manager = std::make_shared<CacheManager>();
        
        // 4. 创建 service 实例
        auto user_service = std::make_shared<UserService>(user_repo, cache_manager);
        auto announcement_service = std::make_shared<AnnouncementService>(announcement_repo, cache_manager);
        auto read_receipt_service = std::make_shared<ReadReceiptService>(
            read_receipt_repo, announcement_repo, user_repo, cache_manager
        );
        
        // 5. 创建认证服务
        JWTConfig jwt_config;
        jwt_config.secret_key = "your-secret-key-change-in-production";
        jwt_config.access_token_expiry = 3600;  // 1 hour
        jwt_config.refresh_token_expiry = 86400 * 7;  // 7 days
        
        auto auth_service = std::make_shared<JwtAuthService>(jwt_config, user_service);
        
        // 6. 创建认证中间件
        auto auth_middleware = std::make_shared<AuthMiddleware>(auth_service);
        
        // 7. 创建控制器
        auto user_controller = std::make_shared<UserController>(user_service, auth_service);
        auto announcement_controller = std::make_shared<AnnouncementController>(
            announcement_service, read_receipt_service, auth_service
        );
        
        // 8. 创建路由器并启动服务器
        const uint16_t port = 3000;
        Router router("http://localhost", port);
        
        router.set_auth_middleware(auth_middleware);
        router.set_user_controller(user_controller);
        router.set_announcement_controller(announcement_controller);
        
        router.register_all_routes();
        router.start().wait();
        
        std::cout << "\nAnnouncement system is running on http://localhost:" << port << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        
        // 等待终止信号
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 停止服务器
        router.stop().wait();
        
        std::cout << "Announcement system has been shut down successfully" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Failed to start announcement system: " << e.what() << std::endl;
        return 1;
    }
}
