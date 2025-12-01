#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <nlohmann/json.hpp>
#include "dao/UserDao.h"

namespace server {

// HTTP请求结构
struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::string query;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

// HTTP响应结构
struct HttpResponse {
    int status_code;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

// 线程池类
class ThreadPool {
public:
    ThreadPool(int num_threads);
    ~ThreadPool();

    // 提交任务
    template<typename F, typename... Args>
    void submit(F&& f, Args&&... args) {
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.push(std::move(task));
        }
        condition_.notify_one();
    }

private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
};

// HTTP服务器类
class HttpServer {
public:
    HttpServer(int port, int num_threads, dao::UserDao& user_dao);
    ~HttpServer();

    // 启动服务器
    void start();
    // 停止服务器
    void stop();

    // 注册路由（接受返回HttpResponse的处理函数）
    void registerRoute(const std::string& method, const std::string& path,
        std::function<HttpResponse(const HttpRequest&, int)> handler);
    
    // 注册路由（接受返回json的处理函数，自动转换为HttpResponse）
    void registerRoute(const std::string& method, const std::string& path,
        std::function<nlohmann::json(const nlohmann::json&, int)> handler);
    
    // 注册路由（接受返回json且不需要user_id的处理函数，自动转换为HttpResponse）
    void registerRoute(const std::string& method, const std::string& path,
        std::function<nlohmann::json(const nlohmann::json&)> handler);

private:
    int port_;
    int server_fd_;
    std::atomic<bool> running_;
    ThreadPool thread_pool_;
    dao::UserDao& user_dao_;

    // 路由表类型定义
    typedef std::function<HttpResponse(const HttpRequest&, int)> RouteHandler;
    typedef std::unordered_map<std::string, RouteHandler> PathHandlerMap;
    typedef std::unordered_map<std::string, PathHandlerMap> MethodHandlerMap;
    
    // 路由表
    MethodHandlerMap routes_;

    // 处理客户端连接
    void handleClient(int client_fd);
    // 解析HTTP请求
    HttpRequest parseRequest(const std::string& request_str) const;
    // 生成HTTP响应
    std::string generateResponse(const HttpResponse& response) const;
    // 验证token并获取用户ID
    int authenticateToken(const std::string& token) const;
    // 处理404错误
    HttpResponse handleNotFound() const;
    // 处理401错误
    HttpResponse handleUnauthorized() const;
    // 处理500错误
    HttpResponse handleInternalServerError() const;
};

} // namespace server

#endif // HTTPSERVER_H
