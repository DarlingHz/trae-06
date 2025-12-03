#include "http_server.h"
#include "controllers/user_controller.h"
#include "utils/json.h"
#include "utils/logger.h"
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

HttpServer::HttpServer(int port) : port_(port), running_(false), socket_fd_(-1), user_controller_(UserController::getInstance()), json_(Json::getInstance()), logger_(Logger::getInstance()) {
}

HttpServer::~HttpServer() {
    if (running_) {
        stop();
    }
}

HttpServer& HttpServer::getInstance(int port) {
    static HttpServer instance(port);
    return instance;
}

bool HttpServer::start() {
    // 创建套接字
    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ < 0) {
        logger_.log(Logger::LogLevel::ERROR, "Failed to create socket: " + std::string(strerror(errno)));
        return false;
    }

    // 设置套接字选项，允许地址重用
    int opt = 1;
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        logger_.log(Logger::LogLevel::ERROR, "Failed to set socket options: " + std::string(strerror(errno)));
        close(socket_fd_);
        return false;
    }

    // 绑定地址和端口
    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    if (bind(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        logger_.log(Logger::LogLevel::ERROR, "Failed to bind socket: " + std::string(strerror(errno)));
        close(socket_fd_);
        return false;
    }

    // 监听连接
    if (listen(socket_fd_, 10) < 0) {
        logger_.log(Logger::LogLevel::ERROR, "Failed to listen on socket: " + std::string(strerror(errno)));
        close(socket_fd_);
        return false;
    }

    running_ = true;
    logger_.log(Logger::LogLevel::INFO, "HTTP server started on port " + std::to_string(port_));

    // 处理客户端连接的循环
    while (running_) {
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        // 接受客户端连接
        int client_fd = accept(socket_fd_, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            if (running_) {
                logger_.log(Logger::LogLevel::ERROR, "Failed to accept client connection: " + std::string(strerror(errno)));
            }
            continue;
        }

        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        logger_.log(Logger::LogLevel::INFO, "New client connection from " + client_ip);

        // 处理客户端请求
        handleClient(client_fd, client_ip);

        // 关闭客户端连接
        close(client_fd);
        logger_.log(Logger::LogLevel::INFO, "Client connection closed: " + client_ip);
    }

    return true;
}

void HttpServer::stop() {
    running_ = false;
    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
    logger_.log(Logger::LogLevel::INFO, "HTTP server stopped");
}

void HttpServer::handleClient(int client_fd, const std::string& client_ip) {
    try {
        char buffer[4096];
        std::memset(buffer, 0, sizeof(buffer));

        // 读取客户端请求
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            if (bytes_read < 0) {
                logger_.log(Logger::LogLevel::ERROR, "Failed to read client request: " + std::string(strerror(errno)));
            }
            return;
        }

        std::string request(buffer, bytes_read);
        logger_.log(Logger::LogLevel::DEBUG, "Received request from " + client_ip + ":\n" + request);

        // 解析请求
        HttpRequest http_request = parseRequest(request);

        // 处理请求并生成响应
        HttpResponse http_response = handleRequest(http_request);

        // 发送响应
        sendResponse(client_fd, http_response);
    } catch (const std::exception& e) {
        logger_.log(Logger::LogLevel::ERROR, "Error handling client request: " + std::string(e.what()));
        // 发送500错误响应
        HttpResponse error_response;
        error_response.status_code = 500;
        error_response.status_message = "Internal Server Error";
        error_response.headers["Content-Type"] = "application/json";
        error_response.body = json_.createErrorResponse(500, "Internal server error");
        sendResponse(client_fd, error_response);
    }
}

HttpRequest HttpServer::parseRequest(const std::string& request) {
    HttpRequest http_request;
    std::istringstream request_stream(request);
    std::string line;

    // 解析请求行
    if (std::getline(request_stream, line)) {
        std::istringstream line_stream(line);
        line_stream >> http_request.method >> http_request.path >> http_request.version;
    }

    // 解析请求头
    while (std::getline(request_stream, line) && line != "\r" && !line.empty()) {
        size_t colon_pos = line.find(":");
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            // 去除value两端的空格
            value.erase(value.find_last_not_of(" \t\r") + 1);
            value.erase(0, value.find_first_not_of(" \t\r"));
            http_request.headers[key] = value;
        }
    }

    // 解析请求体
    if (http_request.headers.find("Content-Length") != http_request.headers.end()) {
        int content_length = std::stoi(http_request.headers["Content-Length"]);
        std::vector<char> body_buffer(content_length);
        request_stream.read(body_buffer.data(), content_length);
        http_request.body = std::string(body_buffer.data(), content_length);
    }

    return http_request;
}

HttpResponse HttpServer::handleRequest(const HttpRequest& request) {
    HttpResponse response;
    response.headers["Content-Type"] = "application/json";
    response.headers["Access-Control-Allow-Origin"] = "*";
    response.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
    response.headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";

    // 处理OPTIONS预检请求
    if (request.method == "OPTIONS") {
        response.status_code = 200;
        response.status_message = "OK";
        return response;
    }

    // 用户注册接口
    if (request.method == "POST" && request.path == "/api/auth/register") {
        response.body = user_controller_.registerUser(request.body);
        // 解析响应体获取状态码
        try {
            auto data = json_.deserialize(response.body);
            response.status_code = std::any_cast<int>(data["code"]);
            response.status_message = response.status_code == 201 ? "Created" : "Bad Request";
        } catch (const std::exception& e) {
            response.status_code = 500;
            response.status_message = "Internal Server Error";
        }
        return response;
    }

    // 用户登录接口
    if (request.method == "POST" && request.path == "/api/auth/login") {
        response.body = user_controller_.loginUser(request.body);
        try {
            auto data = json_.deserialize(response.body);
            response.status_code = std::any_cast<int>(data["code"]);
            response.status_message = response.status_code == 200 ? "OK" : "Unauthorized";
        } catch (const std::exception& e) {
            response.status_code = 500;
            response.status_message = "Internal Server Error";
        }
        return response;
    }

    // 获取用户信息接口
    if (request.method == "GET" && request.path == "/api/user/info") {
        // 验证Authorization头
        if (request.headers.find("Authorization") == request.headers.end()) {
            response.status_code = 401;
            response.status_message = "Unauthorized";
            response.body = json_.createErrorResponse(401, "Missing Authorization header");
            return response;
        }

        std::string auth_header = request.headers["Authorization"];
        // 提取Bearer令牌
        if (auth_header.substr(0, 7) != "Bearer ") {
            response.status_code = 401;
            response.status_message = "Unauthorized";
            response.body = json_.createErrorResponse(401, "Invalid Authorization header format");
            return response;
        }

        std::string token = auth_header.substr(7);
        response.body = user_controller_.getUserInfo(token);
        try {
            auto data = json_.deserialize(response.body);
            response.status_code = std::any_cast<int>(data["code"]);
            response.status_message = response.status_code == 200 ? "OK" : "Unauthorized";
        } catch (const std::exception& e) {
            response.status_code = 500;
            response.status_message = "Internal Server Error";
        }
        return response;
    }

    // 404 Not Found
    response.status_code = 404;
    response.status_message = "Not Found";
    response.body = json_.createErrorResponse(404, "Endpoint not found");
    return response;
}

void HttpServer::sendResponse(int client_fd, const HttpResponse& response) {
    try {
        std::ostringstream response_stream;

        // 状态行
        response_stream << "HTTP/1.1 " << response.status_code << " " << response.status_message << "\r\n";

        // 响应头
        for (const auto& header : response.headers) {
            response_stream << header.first << ": " << header.second << "\r\n";
        }

        // Content-Length头
        response_stream << "Content-Length: " << response.body.size() << "\r\n";

        // 空行分隔头和体
        response_stream << "\r\n";

        // 响应体
        response_stream << response.body;

        std::string response_str = response_stream.str();

        // 发送响应
        ssize_t bytes_sent = send(client_fd, response_str.c_str(), response_str.size(), 0);
        if (bytes_sent < 0) {
            logger_.log(Logger::LogLevel::ERROR, "Failed to send response: " + std::string(strerror(errno)));
        } else {
            logger_.log(Logger::LogLevel::DEBUG, "Sent response: " + std::to_string(bytes_sent) + " bytes");
        }
    } catch (const std::exception& e) {
        logger_.log(Logger::LogLevel::ERROR, "Error sending response: " + std::string(e.what()));
    }
}
