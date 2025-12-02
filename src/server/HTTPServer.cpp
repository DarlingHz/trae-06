#include "server/HTTPServer.h"
#include <iostream>
#include <cstring>
#include <regex>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace pet_hospital {

// ThreadPool implementation
class ThreadPool {
public:
    ThreadPool(size_t num_threads)
        : stop(false)
    {
        for (size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) {
                            return;
                        }
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& thread : threads) {
            thread.join();
        }
    }

    template<typename F, typename... Args>
    void enqueue(F&& f, Args&&... args)
    {
        std::function<void()> task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.push(task);
        }
        condition.notify_one();
    }

private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// HTTPServer::Impl implementation
class HTTPServer::Impl {
public:
    Impl(int port)
        : port(port), server_fd(-1), running(false), thread_pool(4)
    {
    }

    ~Impl()
    {
        stop();
    }

    bool start(std::string& error_message)
    {
        // 创建socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            error_message = "socket creation failed: " + std::string(strerror(errno));
            return false;
        }

        // 设置socket选项，允许端口重用
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                       &opt, sizeof(opt))) {
            error_message = "setsockopt failed: " + std::string(strerror(errno));
            close(server_fd);
            return false;
        }

        // 绑定socket到指定端口
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            error_message = "bind failed: " + std::string(strerror(errno));
            close(server_fd);
            return false;
        }

        // 开始监听连接
        if (listen(server_fd, 10) < 0) {
            error_message = "listen failed: " + std::string(strerror(errno));
            close(server_fd);
            return false;
        }

        running = true;
        std::cout << "Server started on port " << port << std::endl;

        // 启动接受连接的线程
        accept_thread = std::thread(&Impl::accept_connections, this);

        return true;
    }

    void stop()
    {
        if (!running) {
            return;
        }

        running = false;
        close(server_fd);

        if (accept_thread.joinable()) {
            accept_thread.join();
        }

        std::cout << "Server stopped" << std::endl;
    }

    void register_route(const std::string& method, const std::string& path, const HTTPHandler& handler)
    {
        Route route;
        route.method = method;
        route.path = path;
        route.handler = handler;

        // 将路径转换为正则表达式，支持路径参数
        std::string regex_str = "^";
        size_t pos = 0;
        while (pos < path.size()) {
            size_t start = path.find(':', pos);
            if (start == std::string::npos) {
                regex_str += std::regex_replace(path.substr(pos), std::regex("[.+*?^${}()|\\[\\]]"), "\\$&");

































































































































                break;
            }

            regex_str += std::regex_replace(path.substr(pos, start - pos), std::regex("[.+*?^${}()|\\[\\]]"), "\\$&");;
























































            size_t end = path.find('/', start);
            if (end == std::string::npos) {
                end = path.size();
            }

            std::string param_name = path.substr(start + 1, end - start - 1);
            route.param_names.push_back(param_name);
            regex_str += "([^/]+)";

            pos = end;
        }
        regex_str += "$";

        route.path_regex = std::regex(regex_str);

        std::lock_guard<std::mutex> lock(routes_mutex);
        routes.push_back(route);
    }

private:
    // 路由结构体
    struct Route {
        std::string method;
        std::string path;
        std::regex path_regex;
        std::vector<std::string> param_names;
        HTTPHandler handler;
    };

    void accept_connections()
    {
        while (running) {
            sockaddr_in client_address;
            socklen_t client_address_len = sizeof(client_address);

            // 接受新连接
            int client_fd = accept(server_fd, (struct sockaddr*)&client_address, &client_address_len);
            if (client_fd < 0) {
                if (running) {
                    std::cerr << "accept failed: " << strerror(errno) << std::endl;
                }
                continue;
            }

            // 将客户端连接处理任务提交到线程池
            thread_pool.enqueue([this, client_fd] { handle_client(client_fd); });
        }
    }

    void handle_client(int client_fd)
    {
        char buffer[8192];
        memset(buffer, 0, sizeof(buffer));

        // 读取客户端请求
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read < 0) {
            std::cerr << "read failed: " << strerror(errno) << std::endl;
            close(client_fd);
            return;
        }

        // 解析HTTP请求
        HTTPRequest request;
        if (!parse_request(buffer, bytes_read, request)) {
            send_bad_request(client_fd);
            close(client_fd);
            return;
        }

        // 处理HTTP请求
        handle_request(request, client_fd);

        // 关闭客户端连接
        close(client_fd);
    }

    bool parse_request(const char* buffer, size_t buffer_size, HTTPRequest& request)
    {
        std::stringstream ss(std::string(buffer, buffer_size));
        std::string line;

        // 解析请求行
        if (!std::getline(ss, line)) {
            return false;
        }

        // 移除行尾的回车符
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::stringstream request_line_ss(line);
        if (!(request_line_ss >> request.method >> request.path)) {
            return false;
        }

        // 解析查询参数
        size_t query_pos = request.path.find('?');
        if (query_pos != std::string::npos) {
            request.query = request.path.substr(query_pos + 1);
            request.path = request.path.substr(0, query_pos);
        }

        // 解析请求头
        while (std::getline(ss, line)) {
            // 移除行尾的回车符
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            // 空行表示请求头结束
            if (line.empty()) {
                break;
            }

            size_t colon_pos = line.find(':');
            if (colon_pos == std::string::npos) {
                return false;
            }

            std::string header_name = line.substr(0, colon_pos);
            std::string header_value = line.substr(colon_pos + 1);

            // 移除header_value开头的空格
            size_t first_non_space = header_value.find_first_not_of(' ');
            if (first_non_space != std::string::npos) {
                header_value = header_value.substr(first_non_space);
            }

            request.headers[header_name] = header_value;
        }

        // 解析请求体（如果有）
        if (request.headers.count("Content-Length") > 0) {
            try {
                size_t content_length = std::stoul(request.headers["Content-Length"]);
                std::string body;
                body.resize(content_length);

                size_t bytes_read = 0;
                char ch;
                while (bytes_read < content_length && ss.get(ch)) {
                    body[bytes_read++] = ch;
                }

                if (bytes_read == content_length) {
                    request.body = body;
                } else {
                    return false;
                }
            } catch (const std::exception& e) {
                return false;
            }
        }

        return true;
    }

    void handle_request(const HTTPRequest& request, int client_fd)
    {
        // 查找匹配的路由
        std::lock_guard<std::mutex> lock(routes_mutex);
        for (const Route& route : routes) {
            if (route.method == request.method) {
                std::smatch match;
                if (std::regex_match(request.path, match, route.path_regex)) {
                    // 提取路径参数
                    HTTPRequest request_with_params = request;
                    for (size_t i = 0; i < route.param_names.size() && i + 1 < match.size(); ++i) {
                        request_with_params.params[route.param_names[i]] = match[i + 1].str();
                    }

                    // 调用路由处理函数
                    HTTPResponse response = route.handler(request_with_params);

                    // 发送响应
                    send_response(response, client_fd);
                    return;
                }
            }
        }

        // 如果没有找到匹配的路由，返回404 Not Found
        send_not_found(client_fd);
    }

    void send_response(const HTTPResponse& response, int client_fd)
    {
        std::stringstream ss;

        // 响应行
        ss << "HTTP/1.1 " << response.status_code << " " << response.status_message << "\r\n";

        // 响应头
        for (const auto& header : response.headers) {
            ss << header.first << ": " << header.second << "\r\n";
        }

        // 如果响应体不为空，添加Content-Length头
        if (!response.body.empty()) {
            ss << "Content-Length: " << response.body.size() << "\r\n";
        }

        // 空行分隔响应头和响应体
        ss << "\r\n";

        // 响应体
        ss << response.body;

        // 发送响应
        std::string response_str = ss.str();
        send(client_fd, response_str.c_str(), response_str.size(), 0);
    }

    void send_bad_request(int client_fd)
    {
        HTTPResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.body = "<html><body><h1>400 Bad Request</h1></body></html>";
        send_response(response, client_fd);
    }

    void send_not_found(int client_fd)
    {
        HTTPResponse response;
        response.status_code = 404;
        response.status_message = "Not Found";
        response.body = "<html><body><h1>404 Not Found</h1></body></html>";
        send_response(response, client_fd);
    }

    int port;
    int server_fd;
    bool running;
    std::thread accept_thread;
    ThreadPool thread_pool;
    std::vector<Route> routes;
    std::mutex routes_mutex;
};

// HTTPServer implementation
HTTPServer::HTTPServer(int port)
    : impl_(new Impl(port))
{
}

HTTPServer::~HTTPServer()
{
}

bool HTTPServer::start(std::string& error_message)
{
    return impl_->start(error_message);
}

void HTTPServer::stop()
{
    impl_->stop();
}

void HTTPServer::register_route(const std::string& method, const std::string& path, const HTTPHandler& handler)
{
    impl_->register_route(method, path, handler);
}

} // namespace pet_hospital