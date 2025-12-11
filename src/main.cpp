#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>

#include "config/Config.h"
#include "repository/DatabasePool.h"
#include "repository/UserRepository.h"
#include "repository/BookmarkRepository.h"
#include "service/UserService.h"
#include "service/BookmarkService.h"
#include "auth/JWT.h"
#include "http/Server.h"

int main(int argc, char* argv[]) {
    try {
        // 初始化数据库连接池
        auto db_pool = std::make_shared<repository::DatabasePool>("./bookmarks.db", 5);
        db_pool->initialize_tables();
        
        // 创建数据访问层实例
        auto user_repo = repository::create_user_repository(*db_pool);
        auto bookmark_repo = repository::create_bookmark_repository(*db_pool);
        
        // 创建JWT认证实例
        auto jwt = std::make_shared<auth::JWT>("your-secret-key-here");
        
        // 创建业务逻辑层实例
        auto user_service = std::make_shared<service::UserService>(std::move(user_repo), jwt);
        auto bookmark_service = std::make_shared<service::BookmarkService>(std::move(bookmark_repo));
        
        // 创建HTTP服务器实例
        http::Server server(user_service, bookmark_service, jwt, 8080);
        
        // 启动服务器
        std::cout << "Server is running on http://localhost:8080" << std::endl;
        server.start();
        
    } catch(const std::exception& e) {
        std::cerr << "Failed to start server: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
