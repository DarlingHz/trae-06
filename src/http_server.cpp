#include "job_service/http_server.h"
#include "job_service/job.h"
#include "job_service/utils.h"
#include "job_service/logging.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <sstream>
#include <stdexcept>

namespace job_service {

using namespace std::chrono;

// HttpRequest 实现
HttpRequest::HttpRequest(const std::string& method, const std::string& path)
    : method_(method),
      path_(path),
      json_parsed_(false) {
}

void HttpRequest::set_header(const std::string& key, const std::string& value) {
    headers_[key] = value;
}

std::string HttpRequest::get_header(const std::string& key) const {
    auto it = headers_.find(key);
    if (it != headers_.end()) {
        return it->second;
    }
    return "";
}

void HttpRequest::set_body(const std::string& body) {
    body_ = body;
    parse_json_body();
}

const std::string& HttpRequest::get_body() const {
    return body_;
}

const nlohmann::json& HttpRequest::get_json_body() const {
    return json_body_;
}

const std::string& HttpRequest::get_method() const {
    return method_;
}

const std::string& HttpRequest::get_path() const {
    return path_;
}

void HttpRequest::set_path_params(const std::unordered_map<std::string, std::string>& params) {
    // 简化实现
}

std::string HttpRequest::get_path_param(const std::string& key) const {
    // 简化实现
    return "";
}

void HttpRequest::set_query_param(const std::string& key, const std::string& value) {
    query_params_[key] = value;
}

std::string HttpRequest::get_query_param(const std::string& key) const {
    auto it = query_params_.find(key);
    if (it != query_params_.end()) {
        return it->second;
    }
    return "";
}

bool HttpRequest::has_query_param(const std::string& key) const {
    return query_params_.find(key) != query_params_.end();
}

void HttpRequest::parse_query_params(const std::string& query_str) {
    if (query_str.empty()) {
        return;
    }
    
    auto pairs = utils::split(query_str, '&');
    for (const auto& pair : pairs) {
        auto eq_pos = pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = pair.substr(0, eq_pos);
            std::string value = utils::url_decode(pair.substr(eq_pos + 1));
            query_params_[key] = value;
        }
    }
}

bool HttpRequest::parse_json_body() {
    json_parsed_ = false;
    json_body_ = nlohmann::json();
    
    if (body_.empty()) {
        return true;
    }
    
    try {
        nlohmann::json json_data;
        std::istringstream ss(body_);
        ss >> json_data;
        json_body_ = json_data;
        json_parsed_ = true;
        return true;
    } catch (...) {
        return false;
    }
}

bool HttpRequest::parse(const std::string& raw_request) {
    std::istringstream ss(raw_request);
    std::string line;
    
    // 解析请求行
    if (!std::getline(ss, line)) {
        return false;
    }
    
    auto parts = utils::split(line, ' ');
    if (parts.size() < 2) {
        return false;
    }
    
    method_ = parts[0];
    std::string full_path = parts[1];
    
    // 解析路径和查询参数
    auto q_pos = full_path.find('?');
    if (q_pos != std::string::npos) {
        path_ = full_path.substr(0, q_pos);
        parse_query_params(full_path.substr(q_pos + 1));
    } else {
        path_ = full_path;
    }
    
    // 解析请求头
    while (std::getline(ss, line)) {
        line = utils::trim(line);
        if (line.empty()) {
            break;
        }
        
        auto colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = utils::trim(line.substr(0, colon_pos));
            std::string value = utils::trim(line.substr(colon_pos + 1));
            headers_[key] = value;
        }
    }
    
    // 解析请求体
    auto content_length_it = headers_.find("Content-Length");
    if (content_length_it != headers_.end()) {
        try {
            size_t content_length = std::stoul(content_length_it->second);
            body_.resize(content_length);
            ss.read(&body_[0], content_length);
            parse_json_body();
        } catch (...) {
            // 解析失败
        }
    }
    
    return true;
}

// HttpResponse 实现
HttpResponse::HttpResponse(int status_code)
    : status_code_(status_code),
      reason_phrase_(get_default_reason(status_code)) {
    set_header("Content-Type", "application/json");
}

std::string HttpResponse::get_default_reason(int status_code) {
    switch (status_code) {
        case 200: return "OK";
        case 201: return "Created";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default: return "Unknown Status";
    }
}

void HttpResponse::set_status_code(int status_code) {
    status_code_ = status_code;
    reason_phrase_ = get_default_reason(status_code);
}

int HttpResponse::get_status_code() const {
    return status_code_;
}

void HttpResponse::set_header(const std::string& key, const std::string& value) {
    headers_[key] = value;
}

void HttpResponse::set_json_body(const nlohmann::json& json) {
    body_ = json.dump();
    set_header("Content-Length", std::to_string(body_.size()));
}

void HttpResponse::set_string_body(const std::string& body, const std::string& content_type) {
    body_ = body;
    set_header("Content-Type", content_type);
    set_header("Content-Length", std::to_string(body_.size()));
}

std::string HttpResponse::to_string() const {
    std::ostringstream oss;
    
    // 状态行
    oss << "HTTP/1.1 " << status_code_ << " " << reason_phrase_ << "\r\n";
    
    // 响应头
    for (const auto& pair : headers_) {
        oss << pair.first << ": " << pair.second << "\r\n";
    }
    
    // 空行分隔头和体
    oss << "\r\n";
    
    // 响应体
    oss << body_;
    
    return oss.str();
}

HttpResponse HttpResponse::create_error(int status_code, const std::string& message) {
    HttpResponse response(status_code);
    nlohmann::json error_json;
    error_json["error"] = message;
    response.set_json_body(error_json);
    return response;
}

HttpResponse HttpResponse::create_success(const nlohmann::json& data) {
    HttpResponse response(200);
    response.set_json_body(data);
    return response;
}

// HttpServer 实现
HttpServer::HttpServer(int port, std::unique_ptr<TaskExecutor> task_executor, std::shared_ptr<Storage> storage)
    : port_(port),
      task_executor_(std::move(task_executor)),
      storage_(std::move(storage)),
      running_(false) {
    register_api_routes();
}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::start() {
    if (running_) {
        return false;
    }
    
    // 创建 socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        global_logger.error("Failed to create socket");
        return false;
    }
    
    // 设置 socket 选项
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        global_logger.error("Failed to set socket options");
        close(server_socket);
        return false;
    }
    
    // 绑定地址
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);
    
    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        global_logger.error("Failed to bind socket");
        close(server_socket);
        return false;
    }
    
    // 监听
    if (listen(server_socket, 10) < 0) {
        global_logger.error("Failed to listen on socket");
        close(server_socket);
        return false;
    }
    
    running_ = true;
    global_logger.info("HTTP server started on port ", port_);
    
    // 启动任务执行器
    task_executor_->start();
    
    // 接受连接的线程
    std::thread accept_thread([this, server_socket]() {
        while (running_) {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            
            int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
            
            if (client_socket >= 0 && running_) {
                std::thread(&HttpServer::handle_client, this, client_socket).detach();
            }
        }
        close(server_socket);
    });
    
    accept_thread.detach();
    return true;
}

void HttpServer::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    // 停止任务执行器
    if (task_executor_) {
        task_executor_->stop();
    }
    
    global_logger.info("HTTP server stopped");
}

bool HttpServer::is_running() const {
    return running_;
}

void HttpServer::handle_client(int client_socket) const {
    char buffer[4096] = {0};
    ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    
    if (bytes_read > 0) {
        std::string raw_request(buffer, bytes_read);
        HttpRequest request("", "");
        
        if (request.parse(raw_request)) {
            auto handler_opt = match_route(request);
            HttpResponse response(404);
            
            if (handler_opt) {
                try {
                    response = (*handler_opt)(request);
                } catch (const std::exception& e) {
                    response = HttpResponse::create_error(500, std::string("Internal server error: ") + e.what());
                    global_logger.error("Request handling error: ", e.what());
                } catch (...) {
                    response = HttpResponse::create_error(500, "Unknown internal server error");
                    global_logger.error("Unknown request handling error");
                }
            } else {
                response = HttpResponse::create_error(404, "Route not found");
            }
            
            std::string response_str = response.to_string();
            send(client_socket, response_str.c_str(), response_str.size(), 0);
        }
    }
    
    close(client_socket);
}

std::optional<RouteHandler> HttpServer::match_route(const HttpRequest& request) const {
    auto method_it = routes_.find(request.get_method());
    if (method_it == routes_.end()) {
        return std::nullopt;
    }
    
    auto& path_handlers = method_it->second;
    auto path_it = path_handlers.find(request.get_path());
    
    if (path_it != path_handlers.end()) {
        return path_it->second;
    }
    
    // 处理带参数的路径（简化实现）
    for (const auto& pair : path_handlers) {
        if (pair.first == "/api/jobs/{job_id}" && request.get_path().find("/api/jobs/") == 0) {
            return pair.second;
        }
        if (pair.first == "/api/jobs/{job_id}/cancel" && request.get_path().find("/api/jobs/") == 0) {
            return pair.second;
        }
    }
    
    return std::nullopt;
}

void HttpServer::register_api_routes() {
    routes_["POST"]["/api/jobs"] = std::bind(&HttpServer::handle_create_job, this, std::placeholders::_1);
    routes_["GET"]["/api/jobs"] = std::bind(&HttpServer::handle_list_jobs, this, std::placeholders::_1);
    routes_["GET"]["/api/jobs/{job_id}"] = std::bind(&HttpServer::handle_get_job, this, std::placeholders::_1);
    routes_["POST"]["/api/jobs/{job_id}/cancel"] = std::bind(&HttpServer::handle_cancel_job, this, std::placeholders::_1);
    routes_["GET"]["/health"] = std::bind(&HttpServer::handle_health, this, std::placeholders::_1);
}

HttpResponse HttpServer::handle_create_job(const HttpRequest& request) const {
    const auto& json_body = request.get_json_body();
    
    // 验证请求参数
    if (!utils::json_has_key(json_body, "type") || !utils::json_has_key(json_body, "payload")) {
        return HttpResponse::create_error(400, "Missing required fields: type and payload are required");
    }
    
    std::string type = json_body["type"].get<std::string>();
    
    // 检查任务类型是否支持
    auto factory = std::make_shared<TaskFactory>();
    if (!factory->has_task_type(type)) {
        return HttpResponse::create_error(400, std::string("Unknown task type: ") + type);
    }
    
    // 获取优先级
    int priority = 5;
    if (utils::json_has_key(json_body, "priority")) {
        priority = json_body["priority"].get<int>();
    }
    
    // 创建任务
    std::string job_id = utils::generate_job_id();
    auto job = std::make_shared<Job>(job_id, type, json_body["payload"], priority);
    
    // 提交任务
    if (!task_executor_->submit_job(job)) {
        return HttpResponse::create_error(500, "Failed to submit job");
    }
    
    // 返回响应
    nlohmann::json response_json;
    response_json["job_id"] = job_id;
    response_json["status"] = job_status_to_string(job->get_status());
    
    HttpResponse response(201);
    response.set_json_body(response_json);
    return response;
}

HttpResponse HttpServer::handle_get_job(const HttpRequest& request) const {
    // 从路径中提取 job_id
    std::string path = request.get_path();
    size_t pos = path.find("/api/jobs/") + 10;
    std::string job_id = path.substr(pos);
    
    // 检查是否有 cancel 后缀
    size_t cancel_pos = job_id.find("/cancel");
    if (cancel_pos != std::string::npos) {
        job_id = job_id.substr(0, cancel_pos);
    }
    
    // 从存储中获取任务
    auto job_opt = storage_->get_job(job_id);
    if (!job_opt) {
        return HttpResponse::create_error(404, "Job not found");
    }
    
    // 返回任务信息
    HttpResponse response(200);
    response.set_json_body((*job_opt)->to_json());
    return response;
}

HttpResponse HttpServer::handle_list_jobs(const HttpRequest& request) const {
    // 获取查询参数
    std::optional<JobStatus> status_opt;
    if (request.has_query_param("status")) {
        std::string status_str = request.get_query_param("status");
        status_opt = string_to_job_status(status_str);
        if (!status_opt) {
            return HttpResponse::create_error(400, "Invalid status value");
        }
    }
    
    std::optional<JobType> type_opt;
    if (request.has_query_param("type")) {
        type_opt = request.get_query_param("type");
    }
    
    size_t limit = 20;
    if (request.has_query_param("limit")) {
        try {
            limit = std::stoull(request.get_query_param("limit"));
            if (limit > 100) {
                limit = 100; // 限制最大返回数量
            }
        } catch (...) {
            return HttpResponse::create_error(400, "Invalid limit value");
        }
    }
    
    size_t offset = 0;
    if (request.has_query_param("offset")) {
        try {
            offset = std::stoull(request.get_query_param("offset"));
        } catch (...) {
            return HttpResponse::create_error(400, "Invalid offset value");
        }
    }
    
    // 查询任务
    auto jobs = storage_->get_jobs(status_opt, type_opt, limit, offset);
    
    // 转换为JSON
    nlohmann::json response_json = nlohmann::json(std::vector<nlohmann::json>());
    for (const auto& job : jobs) {
        response_json.push_back(job->to_json());
    }
    
    HttpResponse response(200);
    response.set_json_body(response_json);
    return response;
}

HttpResponse HttpServer::handle_cancel_job(const HttpRequest& request) const {
    // 从路径中提取 job_id
    std::string path = request.get_path();
    size_t pos = path.find("/api/jobs/") + 10;
    size_t cancel_pos = path.find("/cancel");
    if (cancel_pos == std::string::npos || cancel_pos <= pos) {
        return HttpResponse::create_error(400, "Invalid request path");
    }
    
    std::string job_id = path.substr(pos, cancel_pos - pos);
    
    // 从存储中获取任务
    auto job_opt = storage_->get_job(job_id);
    if (!job_opt) {
        return HttpResponse::create_error(404, "Job not found");
    }
    
    auto job = *job_opt;
    
    // 检查任务当前状态
    JobStatus current_status = job->get_status();
    
    if (current_status == JobStatus::CANCELED || current_status == JobStatus::DONE || current_status == JobStatus::FAILED) {
        nlohmann::json response_json;
        response_json["job_id"] = job_id;
        response_json["success"] = false;
        response_json["current_status"] = job_status_to_string(current_status);
        response_json["error"] = "Cannot cancel job in current status";
        
        HttpResponse response(400);
        response.set_json_body(response_json);
        return response;
    }
    
    // 请求取消
    job->request_cancel();
    
    if (current_status == JobStatus::QUEUED) {
        job->set_status(JobStatus::CANCELED);
    }
    
    // 更新存储
    storage_->update_job(job);
    
    // 返回响应
    nlohmann::json response_json;
    response_json["job_id"] = job_id;
    response_json["success"] = true;
    response_json["current_status"] = job_status_to_string(job->get_status());
    
    HttpResponse response(200);
    response.set_json_body(response_json);
    return response;
}

HttpResponse HttpServer::handle_health(const HttpRequest& request) const {
    nlohmann::json response_json;
    response_json["status"] = "healthy";
    response_json["timestamp"] = utils::time_to_iso_string(std::chrono::system_clock::now());
    
    HttpResponse response(200);
    response.set_json_body(response_json);
    return response;
}

} // namespace job_service
