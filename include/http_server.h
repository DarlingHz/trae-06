#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include <map>
#include <atomic>
#include "controllers/user_controller.h"
#include "utils/json.h"
#include "utils/logger.h"

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};

struct HttpResponse {
    int status_code = 200;
    std::string status_message = "OK";
    std::map<std::string, std::string> headers;
    std::string body;
};

class HttpServer {
private:
    int port_;
    std::atomic<bool> running_;
    int socket_fd_;
    UserController& user_controller_;
    Json& json_;
    Logger& logger_;

    HttpServer(int port) : port_(port), running_(false), socket_fd_(-1), user_controller_(UserController::getInstance()), json_(Json::getInstance()), logger_(Logger::getInstance()) {}
    ~HttpServer();

    // 处理客户端连接
    void handleClient(int client_fd, const std::string& client_ip);

    // 解析HTTP请求
    HttpRequest parseRequest(const std::string& request);

    // 处理HTTP请求并生成响应
    HttpResponse handleRequest(const HttpRequest& request);

    // 发送HTTP响应
    void sendResponse(int client_fd, const HttpResponse& response);

public:
    static HttpServer& getInstance(int port = 8080);

    // 启动服务器
    bool start();

    // 停止服务器
    void stop();
};

#endif // HTTP_SERVER_H
