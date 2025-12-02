#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include "server/HttpServer.h"
#include "controller/UserController.h"
#include "controller/SnippetController.h"
#include "service/UserService.h"
#include "service/SnippetService.h"
#include "repository/UserRepository.h"
#include "repository/SnippetRepository.h"

int main(int argc, char* argv[]) {
    try {
        // 配置数据库连接
        std::string db_path = "./snippets.db";

        // 创建 UserRepository 和 SnippetRepository 实例
        auto user_repo = std::make_shared<repository::UserRepository>(db_path);
        auto snippet_repo = std::make_shared<repository::SnippetRepository>(db_path);

        // 创建 UserService 和 SnippetService 实例
        auto user_service = std::make_shared<service::UserService>(*user_repo);
        auto snippet_service = std::make_shared<service::SnippetService>(*snippet_repo);

        // 创建 HttpServer 实例
        std::string address = "0.0.0.0";
        unsigned short port = 8080;
        boost::asio::io_context io_context;
        auto http_server = std::make_shared<server::HttpServer>(io_context, address, std::to_string(port));

        // 创建 UserController 和 SnippetController 实例
        auto user_controller = std::make_shared<controller::UserController>(user_service, http_server);
        auto snippet_controller = std::make_shared<controller::SnippetController>(snippet_service, user_service, http_server);

        // 注册所有 API 端点
        user_controller->registerEndpoints();
        snippet_controller->registerEndpoints();

        // 启动 HTTP 服务器
        std::cout << "Starting server on http://" << address << ":" << port << std::endl;
        http_server->start();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error starting server: " << e.what() << std::endl;
        return 1;
    }
}