#include "server/HTTPServer.h"
#include <unistd.h>
#include "server/RouteRegistrar.h"
#include "logging/Logging.h"
#include "config/Config.h"
#include "dao/BaseDAO.h"
#include "database/Database.h"

#include <iostream>
#include <signal.h>
#include <atomic>

using namespace pet_hospital;

std::atomic<bool> g_server_running(false);

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        LOG_INFO("Received termination signal, stopping server...");
        g_server_running = false;
    }
}

int main() {
    try {
        // 读取配置文件
        Config config("config.json");
        if (!config.load()) {
            std::cerr << "Failed to load configuration file" << std::endl;
            return 1;
        }

        // 初始化日志模块
        if (!g_logger.init(config)) {
            std::cerr << "Failed to initialize logging system" << std::endl;
            return 1;
        }
        LOG_INFO("Pet Hospital Server starting...");

        // 初始化数据库连接
        std::string db_path = config.get_database_connection_string();
        std::shared_ptr<Database> database = std::make_shared<Database>();
        if (!database->init(db_path)) {
            LOG_ERROR("Failed to initialize database");
            return 1;
        }

        // 初始化HTTP服务器
        int port = config.get_server_port();
        HTTPServer server(port);

        // 注册所有路由
        RouteRegistrar route_registrar(server);
        route_registrar.register_all_routes();

        // 设置信号处理
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);

        // 启动服务器
        LOG_INFO("Server starting on port " + std::to_string(port));
        g_server_running = true;
        std::string error_message;
        if (!server.start(error_message)) {
            LOG_ERROR("Failed to start server: " + error_message);
            return 1;
        }

        // 等待服务器停止
        while (g_server_running) {
            usleep(100000); // 100 milliseconds
        }

        // 停止服务器
        server.stop();
        LOG_INFO("Server stopped successfully");

        // 关闭数据库连接
        database->close();

        return 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Server failed to start: " + std::string(e.what()));
        std::cerr << "Server failed to start: " << e.what() << std::endl;
        return 1;
    }
}
