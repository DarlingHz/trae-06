#include "server/HttpServer.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <nlohmann/json.hpp>
#include "util/Utils.h"

namespace server {

// ThreadPool implementation
ThreadPool::ThreadPool(int num_threads) : stop_(false) {
    for (int i = 0; i < num_threads; ++i) {
        threads_.emplace_back([this]() {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    condition_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
                    if (stop_ && tasks_.empty()) {
                        return;
                    }
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for (std::thread& thread : threads_) {
        thread.join();
    }
}

// HttpServer implementation
HttpServer::HttpServer(int port, int num_threads, dao::UserDao& user_dao)
    : port_(port), server_fd_(-1), running_(false), thread_pool_(num_threads), user_dao_(user_dao) {
}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::start() {
    // 创建socket
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ == -1) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return;
    }

    // 设置socket选项，允许地址重用
    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "Failed to set socket options: " << strerror(errno) << std::endl;
        close(server_fd_);
        return;
    }

    // 绑定socket到指定端口
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) == -1) {
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        close(server_fd_);
        return;
    }

    // 开始监听连接
    if (listen(server_fd_, 10) == -1) {
        std::cerr << "Failed to listen on socket: " << strerror(errno) << std::endl;
        close(server_fd_);
        return;
    }

    running_ = true;
    std::cout << "Server started on port " << port_ << std::endl;

    // 启动主线程，接受客户端连接
    std::thread accept_thread([this]() {
        while (running_) {
            struct sockaddr_in client_address;
            socklen_t client_address_len = sizeof(client_address);

            // 接受客户端连接
            int client_fd = accept(server_fd_, (struct sockaddr*)&client_address, &client_address_len);
            if (client_fd == -1) {
                if (running_) {
                    std::cerr << "Failed to accept client connection: " << strerror(errno) << std::endl;
                }
                continue;
            }

            // 获取客户端IP地址
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
            std::cout << "Accepted connection from " << client_ip << std::endl;

            // 将客户端连接交给线程池处理
            thread_pool_.submit([this, client_fd]() {
                handleClient(client_fd);
            });
        }
    });

    // 等待主线程结束
    accept_thread.join();
}

void HttpServer::stop() {
    if (!running_) {
        return;
    }

    running_ = false;
    close(server_fd_);
    std::cout << "Server stopped" << std::endl;
}

void HttpServer::registerRoute(const std::string& method, const std::string& path,
    std::function<HttpResponse(const HttpRequest&, int)> handler) {
    routes_[method][path] = handler;
}

void HttpServer::registerRoute(const std::string& method, const std::string& path,
    std::function<nlohmann::json(const nlohmann::json&, int)> handler) {
    // 将json处理函数转换为HttpResponse处理函数
    auto http_handler = [handler](const HttpRequest& request, int user_id) -> HttpResponse {
        try {
            // 将HttpRequest转换为json
            nlohmann::json request_json;
            if (!request.body.empty()) {
                request_json = nlohmann::json::parse(request.body);
            }
            
            // 调用处理函数
            nlohmann::json response_json = handler(request_json, user_id);
            
            // 将json转换为HttpResponse
            HttpResponse response;
            response.status_code = 200;
            response.headers["Content-Type"] = "application/json";
            response.body = response_json.dump();
            
            return response;
        } catch (const nlohmann::json::parse_error& e) {
            // 处理JSON解析错误
            HttpResponse response;
            response.status_code = 400;
            response.headers["Content-Type"] = "application/json";
            response.body = nlohmann::json({{"code", 400}, {"message", "Invalid JSON format"}}).dump();
            
            return response;
        } catch (const std::exception& e) {
            // 处理其他异常
            HttpResponse response;
            response.status_code = 500;
            response.headers["Content-Type"] = "application/json";
            response.body = nlohmann::json({{"code", 500}, {"message", "Internal server error"}}).dump();
            
            return response;
        }
    };
    
    // 将转换后的处理函数添加到路由表中
    routes_[method][path] = http_handler;
}

void HttpServer::registerRoute(const std::string& method, const std::string& path,
    std::function<nlohmann::json(const nlohmann::json&)> handler) {
    // 将json处理函数转换为HttpResponse处理函数
    auto http_handler = [handler](const HttpRequest& request, int user_id) -> HttpResponse {
        try {
            // 将HttpRequest转换为json
            nlohmann::json request_json;
            if (!request.body.empty()) {
                request_json = nlohmann::json::parse(request.body);
            }
            
            // 调用处理函数
            nlohmann::json response_json = handler(request_json);
            
            // 将json转换为HttpResponse
            HttpResponse response;
            response.status_code = 200;
            response.headers["Content-Type"] = "application/json";
            response.body = response_json.dump();
            
            return response;
        } catch (const nlohmann::json::parse_error& e) {
            // 处理JSON解析错误
            HttpResponse response;
            response.status_code = 400;
            response.headers["Content-Type"] = "application/json";
            response.body = nlohmann::json({{"code", 400}, {"message", "Invalid JSON format"}}).dump();
            
            return response;
        } catch (const std::exception& e) {
            // 处理其他异常
            HttpResponse response;
            response.status_code = 500;
            response.headers["Content-Type"] = "application/json";
            response.body = nlohmann::json({{"code", 500}, {"message", "Internal server error"}}).dump();
            
            return response;
        }
    };
    
    // 将转换后的处理函数添加到路由表中
    routes_[method][path] = http_handler;
}

void HttpServer::handleClient(int client_fd) {
    const int buffer_size = 4096;
    char buffer[buffer_size];
    std::string request_str;

    // 设置超时时间为5秒
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == -1) {
        std::cerr << "Failed to set socket timeout: " << strerror(errno) << std::endl;
        close(client_fd);
        return;
    }

    // 读取客户端请求
    while (true) {
        ssize_t bytes_read = recv(client_fd, buffer, buffer_size - 1, 0);
        if (bytes_read == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 超时，结束读取
                break;
            } else {
                std::cerr << "Failed to read from client socket: " << strerror(errno) << std::endl;
                close(client_fd);
                return;
            }
        } else if (bytes_read == 0) {
            // 客户端关闭连接
            break;
        }

        // 将读取到的数据添加到请求字符串中
        buffer[bytes_read] = '\0';
        request_str += buffer;

        // 检查是否读取到完整的请求
        if (request_str.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    if (request_str.empty()) {
        close(client_fd);
        return;
    }

    // 解析HTTP请求
    HttpRequest request = parseRequest(request_str);

    // 查找对应的路由处理函数
    HttpResponse response;
    auto method_it = routes_.find(request.method);
    if (method_it != routes_.end()) {
        auto path_it = method_it->second.find(request.path);
        if (path_it != method_it->second.end()) {
            // 验证token（除了注册和登录接口）
            int user_id = -1;
            if (request.path != "/api/users/register" && request.path != "/api/users/login") {
                auto auth_header_it = request.headers.find("Authorization");
                if (auth_header_it == request.headers.end()) {
                    response = handleUnauthorized();
                } else {
                    std::string token = auth_header_it->second.substr(7); // 去掉"Bearer "前缀
                    user_id = authenticateToken(token);
                    if (user_id == -1) {
                        response = handleUnauthorized();
                    }
                }
            }

            // 如果token验证成功，调用路由处理函数
            if (user_id != -1 || request.path == "/api/users/register" || request.path == "/api/users/login") {
                try {
                    response = path_it->second(request, user_id);
                } catch (const std::exception& e) {
                    std::cerr << "Exception in route handler: " << e.what() << std::endl;
                    response = handleInternalServerError();
                }
            }
        } else {
            response = handleNotFound();
        }
    } else {
        response = handleNotFound();
    }

    // 生成HTTP响应字符串
    std::string response_str = generateResponse(response);

    // 发送响应给客户端
    ssize_t bytes_sent = send(client_fd, response_str.c_str(), response_str.size(), 0);
    if (bytes_sent == -1) {
        std::cerr << "Failed to send response to client: " << strerror(errno) << std::endl;
    }

    // 关闭客户端连接
    close(client_fd);
    std::cout << "Closed connection from client" << std::endl;
}

HttpRequest HttpServer::parseRequest(const std::string& request_str) const {
    HttpRequest request;
    std::istringstream request_stream(request_str);
    std::string line;

    // 解析请求行
    if (std::getline(request_stream, line)) {
        // 去掉行尾的\r

        size_t cr_pos = line.find('\r');
        if (cr_pos != std::string::npos) {
            line = line.substr(0, cr_pos);
        }

        std::istringstream line_stream(line);
        line_stream >> request.method >> request.path >> request.version;

        // 解析查询参数
        size_t query_pos = request.path.find('?');
        if (query_pos != std::string::npos) {
            request.query = request.path.substr(query_pos + 1);
            request.path = request.path.substr(0, query_pos);
        }
    }

    // 解析请求头
    while (std::getline(request_stream, line)) {
        // 去掉行尾的\r

        size_t cr_pos = line.find('\r');
        if (cr_pos != std::string::npos) {
            line = line.substr(0, cr_pos);
        }

        // 如果是空行，说明请求头结束
        if (line.empty()) {
            break;
        }

        // 解析请求头字段
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string header_name = line.substr(0, colon_pos);
            std::string header_value = line.substr(colon_pos + 1);

            // 去掉请求头值前后的空白字符
            header_value = util::string::trim(header_value);

            // 将请求头名称转换为小写
            header_name = util::string::toLower(header_name);

            request.headers[header_name] = header_value;
        }
    }

    // 解析请求体
    // 首先检查是否有Content-Length头
    auto content_length_it = request.headers.find("content-length");
    if (content_length_it != request.headers.end()) {
        try {
            int content_length = std::stoi(content_length_it->second);
            char buffer[content_length];
            request_stream.read(buffer, content_length);
            request.body = std::string(buffer, content_length);
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse content length: " << e.what() << std::endl;
        }
    }

    return request;
}

std::string HttpServer::generateResponse(const HttpResponse& response) const {
    std::ostringstream response_stream;

    // 生成响应行
    response_stream << "HTTP/1.1 " << response.status_code << " ";
    switch (response.status_code) {
        case 200:
            response_stream << "OK";
            break;
        case 201:
            response_stream << "Created";
            break;
        case 400:
            response_stream << "Bad Request";
            break;
        case 401:
            response_stream << "Unauthorized";
            break;
        case 404:
            response_stream << "Not Found";
            break;
        case 500:
            response_stream << "Internal Server Error";
            break;
        default:
            response_stream << "Unknown Status";
            break;
    }
    response_stream << "\r\n";

    // 生成响应头
    for (const auto& header : response.headers) {
        response_stream << header.first << ": " << header.second << "\r\n";
    }

    // 生成Content-Length头
    response_stream << "Content-Length: " << response.body.size() << "\r\n";

    // 结束响应头
    response_stream << "\r\n";

    // 生成响应体
    response_stream << response.body;

    return response_stream.str();
}

int HttpServer::authenticateToken(const std::string& token) const {
    // TODO: 实现token验证逻辑
    // 目前只是简单返回1，实际应用中应该从数据库或缓存中验证token
    return 1;
}

HttpResponse HttpServer::handleNotFound() const {
    HttpResponse response;
    response.status_code = 404;
    response.headers["Content-Type"] = "application/json";

    nlohmann::json response_json;
    response_json["code"] = 404;
    response_json["message"] = "Not Found";
    response_json["data"] = nullptr;

    response.body = util::json::toString(response_json);

    return response;
}

HttpResponse HttpServer::handleUnauthorized() const {
    HttpResponse response;
    response.status_code = 401;
    response.headers["Content-Type"] = "application/json";
    response.headers["WWW-Authenticate"] = "Bearer";

    json response_json;
    response_json["code"] = 401;
    response_json["message"] = "Unauthorized";
    response_json["data"] = nullptr;

    response.body = util::json::toString(response_json);

    return response;
}

HttpResponse HttpServer::handleInternalServerError() const {
    HttpResponse response;
    response.status_code = 500;
    response.headers["Content-Type"] = "application/json";

    json response_json;
    response_json["code"] = 500;
    response_json["message"] = "Internal Server Error";
    response_json["data"] = nullptr;

    response.body = util::json::toString(response_json);

    return response;
}

} // namespace server
