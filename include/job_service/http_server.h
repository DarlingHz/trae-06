#ifndef JOB_SERVICE_HTTP_SERVER_H
#define JOB_SERVICE_HTTP_SERVER_H

#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>
#include "task_executor.h"
#include "storage.h"

namespace job_service {

// HTTP请求类
class HttpRequest {
private:
    std::string method_;
    std::string path_;
    std::unordered_map<std::string, std::string> headers_;
    std::unordered_map<std::string, std::string> query_params_;
    std::string body_;
    nlohmann::json json_body_;
    bool json_parsed_;
    
    // 解析查询参数
    void parse_query_params(const std::string& query_str);
    
    // 解析JSON请求体
    bool parse_json_body();

public:
    HttpRequest(const std::string& method, const std::string& path);
    
    // 设置请求头
    void set_header(const std::string& key, const std::string& value);
    
    // 获取请求头
    std::string get_header(const std::string& key) const;
    
    // 设置请求体
    void set_body(const std::string& body);
    
    // 获取请求体
    const std::string& get_body() const;
    
    // 获取JSON请求体
    const nlohmann::json& get_json_body() const;
    
    // 获取方法
    const std::string& get_method() const;
    
    // 获取路径
    const std::string& get_path() const;
    
    // 设置路径参数
    void set_path_params(const std::unordered_map<std::string, std::string>& params);
    
    // 获取路径参数
    std::string get_path_param(const std::string& key) const;
    
    // 设置查询参数
    void set_query_param(const std::string& key, const std::string& value);
    
    // 获取查询参数
    std::string get_query_param(const std::string& key) const;
    
    // 检查是否有查询参数
    bool has_query_param(const std::string& key) const;
    
    // 解析完整请求
    bool parse(const std::string& raw_request);
};

// HTTP响应类
class HttpResponse {
private:
    int status_code_;
    std::string reason_phrase_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
    
    // 默认响应短语
    static std::string get_default_reason(int status_code);

public:
    explicit HttpResponse(int status_code = 200);
    
    // 设置状态码
    void set_status_code(int status_code);
    
    // 获取状态码
    int get_status_code() const;
    
    // 设置响应头
    void set_header(const std::string& key, const std::string& value);
    
    // 设置JSON响应体
    void set_json_body(const nlohmann::json& json);
    
    // 设置字符串响应体
    void set_string_body(const std::string& body, const std::string& content_type = "text/plain");
    
    // 转换为字符串
    std::string to_string() const;
    
    // 创建错误响应
    static HttpResponse create_error(int status_code, const std::string& message);
    
    // 创建成功响应
    static HttpResponse create_success(const nlohmann::json& data);
};

// HTTP路由处理函数类型
typedef std::function<HttpResponse(const HttpRequest&)> RouteHandler;

// HTTP服务器类
class HttpServer {
private:
    int port_;
    std::unique_ptr<TaskExecutor> task_executor_;
    std::shared_ptr<Storage> storage_;
    std::unordered_map<std::string, std::unordered_map<std::string, RouteHandler>> routes_;
    std::atomic<bool> running_;
    
    // 处理客户端连接
    void handle_client(int client_socket) const;
    
    // 解析请求路由
    std::optional<RouteHandler> match_route(const HttpRequest& request) const;
    
    // 注册API路由
    void register_api_routes();
    
    // API处理函数
    HttpResponse handle_create_job(const HttpRequest& request) const;
    HttpResponse handle_get_job(const HttpRequest& request) const;
    HttpResponse handle_list_jobs(const HttpRequest& request) const;
    HttpResponse handle_cancel_job(const HttpRequest& request) const;
    HttpResponse handle_health(const HttpRequest& request) const;

public:
    HttpServer(int port, std::unique_ptr<TaskExecutor> task_executor, std::shared_ptr<Storage> storage);
    
    ~HttpServer();
    
    // 启动服务器
    bool start();
    
    // 停止服务器
    void stop();
    
    // 检查服务器是否运行
    bool is_running() const;
};

} // namespace job_service

#endif // JOB_SERVICE_HTTP_SERVER_H
