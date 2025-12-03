#include "http_server.h"
#include "utils/config.h"
#include "utils/logger.h"
#include "models/user.h"
#include "models/question.h"
#include "models/answer.h"
#include "models/like.h"
#include "models/tag.h"

int main(int argc, char* argv[]) {
    try {
        // 加载配置
        Config& config = Config::getInstance();

        // 初始化日志
        Logger& logger = Logger::getInstance();
        logger.setLogLevel(Logger::LogLevel::INFO);
        logger.enableFileLogging("server.log");

        logger.log(Logger::LogLevel::INFO, "Server starting...");

        // 初始化数据库表
        logger.log(Logger::LogLevel::INFO, "Initializing database tables...");
        if (!User::createTable()) {
            logger.log(Logger::LogLevel::ERROR, "Failed to create users table");
            return 1;
        }
        if (!Question::createTable()) {
            logger.log(Logger::LogLevel::ERROR, "Failed to create questions table");
            return 1;
        }
        if (!Answer::createTable()) {
            logger.log(Logger::LogLevel::ERROR, "Failed to create answers table");
            return 1;
        }
        if (!Like::createTable()) {
            logger.log(Logger::LogLevel::ERROR, "Failed to create likes table");
            return 1;
        }
        if (!Tag::createTable()) {
            logger.log(Logger::LogLevel::ERROR, "Failed to create tags table");
            return 1;
        }
        logger.log(Logger::LogLevel::INFO, "Database tables initialized successfully");

        // 获取端口配置
        int port = config.getInt("server.port", 8080);

        // 启动HTTP服务器
        HttpServer& server = HttpServer::getInstance(port);
        logger.log(Logger::LogLevel::INFO, "Starting HTTP server on port " + std::to_string(port));

        if (!server.start()) {
            logger.log(Logger::LogLevel::ERROR, "Failed to start HTTP server");
            return 1;
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Server initialization failed: " << e.what() << std::endl;
        return 1;
    }
}
