#include "chat_archive/Config.h"
#include "chat_archive/Logger.h"
#include "chat_archive/Database.h"
#include "chat_archive/dao/UserDAO.h"
#include "chat_archive/dao/ConversationDAO.h"
#include "chat_archive/dao/MessageDAO.h"
#include "chat_archive/service/UserService.h"
#include "chat_archive/service/ConversationService.h"
#include "chat_archive/service/MessageService.h"
#include "chat_archive/service/StatsService.h"
#include "chat_archive/controller/UserController.h"
#include "chat_archive/controller/ConversationController.h"
#include "chat_archive/controller/MessageController.h"
#include "chat_archive/controller/StatsController.h"
#include <httplib.h>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        // 1. 加载配置
        chat_archive::Config& config = chat_archive::Config::get();
        config.load();
        
        // 2. 初始化日志
        chat_archive::Logger::init(config.get_log_level());
        CHAT_ARCHIVE_LOG_INFO("Chat Archive Server starting...");
        
        // 3. 打印配置信息
        CHAT_ARCHIVE_LOG_INFO("Configuration:");
        CHAT_ARCHIVE_LOG_INFO("  Server Port: {}", config.get_port());
        CHAT_ARCHIVE_LOG_INFO("  Database Path: {}", config.get_db_path());
        CHAT_ARCHIVE_LOG_INFO("  Log Level: {}", config.get_log_level());
        
        // 5. 初始化数据访问对象
        chat_archive::dao::UserDAO user_dao;
        chat_archive::dao::ConversationDAO conversation_dao;
        chat_archive::dao::MessageDAO message_dao;
        
        // 6. 初始化业务服务
        chat_archive::service::UserService user_service;
        chat_archive::service::ConversationService conversation_service;
        chat_archive::service::MessageService message_service;
        chat_archive::service::StatsService stats_service;
        
        // 7. 初始化控制器
        chat_archive::controller::UserController user_controller(user_service);
        chat_archive::controller::ConversationController conversation_controller(conversation_service);
        chat_archive::controller::MessageController message_controller(message_service);
        chat_archive::controller::StatsController stats_controller(stats_service);
        
        // 8. 创建HTTP服务器
        httplib::Server server;
        
        // 9. 注册路由
        user_controller.init_routes(server);
        conversation_controller.init_routes(server);
        message_controller.init_routes(server);
        stats_controller.init_routes(server);
        
        // 10. 启动服务器
        int port = config.get_port();
        CHAT_ARCHIVE_LOG_INFO("Server starting on port {}", port);
        
        if (!server.listen("0.0.0.0", port)) {
            CHAT_ARCHIVE_LOG_ERROR("Failed to start server on port {}", port);
            return 1;
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    }
}