#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace pet_hospital {

// HTTP请求结构体
struct HTTPRequest {
    std::string method; // HTTP方法（GET、POST、PUT、DELETE等）
    std::string path; // 请求路径
    std::string query; // 查询参数
    std::string body; // 请求体
    std::unordered_map<std::string, std::string> headers; // 请求头
    std::unordered_map<std::string, std::string> params; // 路径参数
};

// HTTP响应结构体
struct HTTPResponse {
    int status_code; // 状态码
    std::string status_message; // 状态消息
    std::string body; // 响应体
    std::unordered_map<std::string, std::string> headers; // 响应头

    // 构造函数
    HTTPResponse() : status_code(200), status_message("OK") {
        headers["Content-Type"] = "application/json; charset=utf-8";
        headers["Access-Control-Allow-Origin"] = "*";
        headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
        headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
    }
};

// HTTP请求处理函数类型
typedef std::function<HTTPResponse(const HTTPRequest&)> HTTPHandler;

// HTTP服务器类
class HTTPServer {
public:
    HTTPServer(int port = 8080);
    ~HTTPServer();

    // 启动服务器
    bool start(std::string& error_message);

    // 停止服务器
    void stop();

    // 注册路由
    void register_route(const std::string& method, const std::string& path, const HTTPHandler& handler);

private:
    // 服务器实现类（Pimpl惯用法）
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace pet_hospital
