#include "http_server.h"
#include "log.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <sstream>
#include <regex>

namespace recruitment {

// HTTP方法转换为字符串
std::string httpMethodToString(HttpMethod method) {
    switch (method) {
        case HttpMethod::GET: return "GET";
        case HttpMethod::POST: return "POST";
        case HttpMethod::PUT: return "PUT";
        case HttpMethod::DELETE: return "DELETE";
        case HttpMethod::PATCH: return "PATCH";
        case HttpMethod::HEAD: return "HEAD";
        case HttpMethod::OPTIONS: return "OPTIONS";
        default: return "UNKNOWN";
    }
}

// ThreadPool 实现

ThreadPool::ThreadPool(int thread_count) : stop_(false) {
    if (thread_count <= 0) {
        thread_count = std::thread::hardware_concurrency();
        if (thread_count <= 0) {
            thread_count = 4;
        }
    }

    LOG_INFO("Creating thread pool with " + std::to_string(thread_count) + " threads");

    for (int i = 0; i < thread_count; ++i) {
        workers_.emplace_back(&ThreadPool::worker, this);
    }
}

ThreadPool::~ThreadPool() {
    { 
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }

    condition_.notify_all();

    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    LOG_INFO("Thread pool destroyed");
}

void ThreadPool::worker() {
    LOG_DEBUG("Worker thread started: " + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())));

    while (true) {
        std::function<void()> task;

        { 
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });

            if (stop_ && tasks_.empty()) {
                LOG_DEBUG("Worker thread stopping: " + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())));
                return;
            }

            task = std::move(tasks_.front());
            tasks_.pop();
        }

        try {
            task();
        } catch (const std::exception& e) {
            LOG_ERROR("Exception in worker thread: " + std::string(e.what()));
        } catch (...) {
            LOG_ERROR("Unknown exception in worker thread");
        }
    }
}

// HttpServer 实现

HttpServer::HttpServer(int port, int thread_count) 
    : port_(port), 
      server_socket_(-1), 
      running_(false), 
      thread_pool_(std::make_shared<ThreadPool>(thread_count)) {
}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::start() {
    if (running_) {
        LOG_WARN("HttpServer is already running");
        return true;
    }

    // 创建服务器套接字
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ == -1) {
        LOG_ERROR("Failed to create server socket: " + std::string(strerror(errno)));
        return false;
    }

    // 设置套接字选项，允许地址重用
    int optval = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        LOG_ERROR("Failed to set socket option SO_REUSEADDR: " + std::string(strerror(errno)));
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }

    // 绑定套接字到地址和端口
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        LOG_ERROR("Failed to bind server socket: " + std::string(strerror(errno)));
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }

    // 开始监听连接
    if (listen(server_socket_, SOMAXCONN) == -1) {
        LOG_ERROR("Failed to listen on server socket: " + std::string(strerror(errno)));
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }

    running_ = true;

    // 启动接受连接线程
    accept_thread_ = std::thread(&HttpServer::handleClient, this, server_socket_);

    LOG_INFO("HttpServer started successfully, listening on port " + std::to_string(port_));

    return true;
}

void HttpServer::stop() {
    if (!running_) {
        return;
    }

    running_ = false;

    // 关闭服务器套接字
    if (server_socket_ != -1) {
        close(server_socket_);
        server_socket_ = -1;
    }

    // 等待接受连接线程结束
    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }

    LOG_INFO("HttpServer stopped successfully");
}

void HttpServer::registerRoute(HttpMethod method, const std::string& path, const HttpHandler& handler) {
    std::unique_lock<std::mutex> lock(routes_mutex_);
    routes_[method][path] = handler;
    LOG_DEBUG("Registered route: " + httpMethodToString(method) + " " + path);
}

void HttpServer::handleClient(int client_socket) {
    LOG_DEBUG("Client connection accepted: " + std::to_string(client_socket));

    char buffer[4096];
    std::string request_data;

    while (running_) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_read == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 非阻塞模式下没有数据可读，继续等待
                continue;
            } else {
                LOG_ERROR("Failed to read from client socket: " + std::string(strerror(errno)));
                break;
            }
        } else if (bytes_read == 0) {
            // 客户端关闭了连接
            LOG_DEBUG("Client connection closed: " + std::to_string(client_socket));
            break;
        }

        request_data.append(buffer, bytes_read);

        // 检查是否收到完整的请求
        if (request_data.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    if (!request_data.empty()) {
        try {
            // 解析HTTP请求
            HttpRequest request = parseRequest(request_data);
            LOG_DEBUG("Received request: " + httpMethodToString(request.method) + " " + request.path);

            // 查找路由处理函数
            std::optional<HttpHandler> handler = findRoute(request.method, request.path);

            HttpResponse response;

            if (handler) {
                // 执行路由处理函数
                response = (*handler)(request);
            } else {
                // 路由未找到
                response.status_code = 404;
                response.body = R"({
                    "error": "Not Found",
                    "message": "The requested resource could not be found"
                })";
                LOG_WARN("Route not found: " + httpMethodToString(request.method) + " " + request.path);
            }

            // 格式化并发送响应
            std::string response_data = formatResponse(response);
            send(client_socket, response_data.c_str(), response_data.size(), 0);

            LOG_DEBUG("Sent response: " + std::to_string(response.status_code) + " " + std::to_string(response.body.size()) + " bytes");

        } catch (const std::exception& e) {
            LOG_ERROR("Exception while handling client request: " + std::string(e.what()));

            // 发送500内部服务器错误响应
            HttpResponse response(500, R"({
                "error": "Internal Server Error",
                "message": "An unexpected error occurred while processing your request"
            })");
            std::string response_data = formatResponse(response);
            send(client_socket, response_data.c_str(), response_data.size(), 0);
        } catch (...) {
            LOG_ERROR("Unknown exception while handling client request");

            // 发送500内部服务器错误响应
            HttpResponse response(500, R"({
                "error": "Internal Server Error",
                "message": "An unexpected error occurred while processing your request"
            })");
            std::string response_data = formatResponse(response);
            send(client_socket, response_data.c_str(), response_data.size(), 0);
        }
    }

    // 关闭客户端套接字
    close(client_socket);
}

HttpRequest HttpServer::parseRequest(const std::string& request_data) {
    HttpRequest request;
    std::istringstream request_stream(request_data);
    std::string line;

    // 解析请求行
    if (std::getline(request_stream, line)) {
        // 移除行尾的\r

        size_t cr_pos = line.find('\r');
        if (cr_pos != std::string::npos) {
            line = line.substr(0, cr_pos);
        }

        std::istringstream line_stream(line);
        std::string method_str;
        std::string path_str;
        std::string http_version;

        line_stream >> method_str >> path_str >> http_version;

        // 解析请求方法
        if (method_str == "GET") {
            request.method = HttpMethod::GET;
        } else if (method_str == "POST") {
            request.method = HttpMethod::POST;
        } else if (method_str == "PUT") {
            request.method = HttpMethod::PUT;
        } else if (method_str == "DELETE") {
            request.method = HttpMethod::DELETE;
        } else if (method_str == "PATCH") {
            request.method = HttpMethod::PATCH;
        } else if (method_str == "HEAD") {
            request.method = HttpMethod::HEAD;
        } else if (method_str == "OPTIONS") {
            request.method = HttpMethod::OPTIONS;
        }

        // 解析路径和查询参数
        size_t query_pos = path_str.find('?');
        if (query_pos != std::string::npos) {
            request.path = path_str.substr(0, query_pos);

            // 解析查询参数
            std::string query_str = path_str.substr(query_pos + 1);
            std::istringstream query_stream(query_str);
            std::string param;

            while (std::getline(query_stream, param, '&')) {
                size_t equal_pos = param.find('=');
                if (equal_pos != std::string::npos) {
                    std::string key = param.substr(0, equal_pos);
                    std::string value = param.substr(equal_pos + 1);
                    request.query_params[key] = value;
                } else {
                    request.query_params[param] = "";
                }
            }
        } else {
            request.path = path_str;
        }
    }

    // 解析请求头部
    while (std::getline(request_stream, line)) {
        // 移除行尾的\r

        size_t cr_pos = line.find('\r');
        if (cr_pos != std::string::npos) {
            line = line.substr(0, cr_pos);
        }

        // 空行表示头部结束
        if (line.empty()) {
            break;
        }

        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);

            // 移除值前面的空格
            size_t space_pos = value.find_first_not_of(' ');
            if (space_pos != std::string::npos) {
                value = value.substr(space_pos);
            }

            request.headers[key] = value;
        }
    }

    // 解析请求体
    if (request.headers.count("Content-Length") > 0) {
        try {
            size_t content_length = std::stoul(request.headers["Content-Length"]);
            std::string body;
            body.reserve(content_length);

            char buffer[4096];
            while (body.size() < content_length) {
                size_t bytes_to_read = std::min(sizeof(buffer), content_length - body.size());
                size_t bytes_read = request_stream.readsome(buffer, bytes_to_read);

                if (bytes_read == 0) {
                    break;
                }

                body.append(buffer, bytes_read);
            }

            request.body = body;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse Content-Length header: " + std::string(e.what()));
        }
    }

    return request;
}

// HTTP状态码转换为字符串
std::string getHttpStatusCodeText(int status_code) {
    switch (status_code) {
        case 200: return "OK";
        case 201: return "Created";
        case 202: return "Accepted";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 406: return "Not Acceptable";
        case 415: return "Unsupported Media Type";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        default: return "Unknown";
    }
}

std::string HttpServer::formatResponse(const HttpResponse& response) {
    std::ostringstream response_stream;

    // 响应行
    response_stream << "HTTP/1.1 " << response.status_code << " " << getHttpStatusCodeText(response.status_code) << "\r\n";

    // 响应头部
    for (const auto& header : response.headers) {
        response_stream << header.first << ": " << header.second << "\r\n";
    }

    // 内容长度头部（如果未设置）
    if (response.headers.count("Content-Length") == 0 && !response.body.empty()) {
        response_stream << "Content-Length: " << response.body.size() << "\r\n";
    }

    // 空行分隔头部和体
    response_stream << "\r\n";

    // 响应体
    if (!response.body.empty()) {
        response_stream << response.body;
    }

    return response_stream.str();
}

std::optional<HttpHandler> HttpServer::findRoute(HttpMethod method, const std::string& path) {
    std::unique_lock<std::mutex> lock(routes_mutex_);

    auto method_it = routes_.find(method);
    if (method_it == routes_.end()) {
        return std::nullopt;
    }

    auto path_it = method_it->second.find(path);
    if (path_it != method_it->second.end()) {
        return path_it->second;
    }

    // 尝试匹配带参数的路由（如 /api/jobs/:id）
    for (const auto& route : method_it->second) {
        const std::string& route_path = route.first;
        std::regex route_regex;

        try {
            // 将路由路径转换为正则表达式
            std::string regex_pattern = "^";
            size_t pos = 0;

            while (pos < route_path.size()) {
                size_t colon_pos = route_path.find(':', pos);
                size_t slash_pos = route_path.find('/', pos);

                if (colon_pos != std::string::npos && (slash_pos == std::string::npos || colon_pos < slash_pos)) {
                    // 匹配参数前的部分
                    regex_pattern += std::regex_replace(route_path.substr(pos, colon_pos - pos), std::regex("[.+*?^${}()|[\\]\\\\]"), "\\\\$&");

                    // 匹配参数（直到下一个斜杠或字符串结束）
                    regex_pattern += "([^/]+)";

                    pos = route_path.find('/', colon_pos);
                    if (pos == std::string::npos) {
                        pos = route_path.size();
                    }
                } else {
                    // 匹配普通部分
                    size_t end_pos = (slash_pos == std::string::npos) ? route_path.size() : slash_pos;
                    regex_pattern += std::regex_replace(route_path.substr(pos, end_pos - pos), std::regex("[.+*?^${}()|[\\]\\\\]"), "\\\\$&");

                    if (slash_pos != std::string::npos) {
                        regex_pattern += "/";
                        pos = slash_pos + 1;
                    } else {
                        pos = route_path.size();
                    }
                }
            }

            regex_pattern += "$";
            route_regex = std::regex(regex_pattern);
        } catch (const std::regex_error& e) {
            LOG_ERROR("Failed to create regex for route: " + route_path + ", error: " + std::string(e.what()));
            continue;
        }

        // 检查请求路径是否匹配路由正则表达式
        if (std::regex_match(path, route_regex)) {
            return route.second;
        }
    }

    return std::nullopt;
}

std::string HttpServer::httpMethodToString(HttpMethod method) {
    switch (method) {
        case HttpMethod::GET: return "GET";
        case HttpMethod::POST: return "POST";
        case HttpMethod::PUT: return "PUT";
        case HttpMethod::DELETE: return "DELETE";
        case HttpMethod::PATCH: return "PATCH";
        case HttpMethod::HEAD: return "HEAD";
        case HttpMethod::OPTIONS: return "OPTIONS";
        default: return "UNKNOWN";
    }
}

std::string HttpServer::getHttpStatusCodeText(int status_code) {
    switch (status_code) {
        case 200: return "OK";
        case 201: return "Created";
        case 202: return "Accepted";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 406: return "Not Acceptable";
        case 415: return "Unsupported Media Type";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        default: return "Unknown";
    }
}

} // namespace recruitment
