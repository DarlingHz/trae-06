#include "chat_archive/controller/UserController.h"
#include "chat_archive/Logger.h"
#include <sstream>

namespace chat_archive {
namespace controller {

void UserController::init_routes(httplib::Server& server) {
    // 注册路由
    server.Post("/api/users", [this](const httplib::Request& req, httplib::Response& res) {
        this->handle_create_user(req, res);
    });
    
    server.Get("/api/users", [this](const httplib::Request& req, httplib::Response& res) {
        this->handle_get_users(req, res);
    });
    
    server.Get("/api/users/(\\d+)", [this](const httplib::Request& req, httplib::Response& res) {
        this->handle_get_user(req, res);
    });
    
    CHAT_ARCHIVE_LOG_INFO("UserController routes initialized");
}

void UserController::handle_create_user(const httplib::Request& req, httplib::Response& res) {
    CHAT_ARCHIVE_LOG_INFO("Received POST request for /api/users");
    
    try {
        // 解析JSON请求体
        json request_body = json::parse(req.body);
        
        // 验证请求体中的字段
        if (!request_body.contains("name") || !request_body["name"].is_string()) {
            send_error_response(res, 400, "INVALID_REQUEST", "Missing or invalid 'name' field");
            return;
        }
        
        std::string name = request_body["name"];
        
        // 调用UserService创建用户
        auto user_id = user_service_.create_user(name);
        
        if (!user_id) {
            send_error_response(res, 400, "USER_CREATION_FAILED", "Failed to create user");
            return;
        }
        
        // 构建成功响应
        json response_data;
        response_data["id"] = *user_id;
        response_data["name"] = name;
        
        send_success_response(res, response_data);
        
        CHAT_ARCHIVE_LOG_INFO("User created successfully with ID: {}", *user_id);
        
    } catch (const json::parse_error& e) {
        CHAT_ARCHIVE_LOG_WARN("Invalid JSON format for create user request: {}", e.what());
        send_error_response(res, 400, "INVALID_JSON", "Invalid JSON format");
    } catch (const std::exception& e) {
        CHAT_ARCHIVE_LOG_ERROR("Error handling create user request: {}", e.what());
        send_error_response(res, 500, "INTERNAL_ERROR", "Internal server error");
    }
}

void UserController::handle_get_users(const httplib::Request& req, httplib::Response& res) {
    CHAT_ARCHIVE_LOG_INFO("Received GET request for /api/users");
    
    try {
        // 解析查询参数
        int limit = 100;
        int offset = 0;
        
        if (req.has_param("limit")) {
            std::string limit_str = req.get_param_value("limit");
            limit = std::stoi(limit_str);
        }
        
        if (req.has_param("offset")) {
            std::string offset_str = req.get_param_value("offset");
            offset = std::stoi(offset_str);
        }
        
        // 调用UserService获取用户列表
        auto users = user_service_.get_users(limit, offset);
        
        // 构建成功响应
        json response_data = json::array();
        
        for (const auto& user : users) {
            json user_data;
            user_data["id"] = user.get_id();
            user_data["name"] = user.get_name();
            std::time_t created_at_time = std::chrono::system_clock::to_time_t(user.get_created_at());
            std::tm created_at_tm = *std::localtime(&created_at_time);
            std::stringstream created_at_ss;
            created_at_ss << std::put_time(&created_at_tm, "%Y-%m-%d %H:%M:%S");
            user_data["created_at"] = created_at_ss.str();
            
            response_data.push_back(user_data);
        }
        
        send_success_response(res, response_data);
        
        CHAT_ARCHIVE_LOG_INFO("Users retrieved successfully: {} users", users.size());
        
    } catch (const std::invalid_argument& e) {
        CHAT_ARCHIVE_LOG_WARN("Invalid query parameters for get users request: {}", e.what());
        send_error_response(res, 400, "INVALID_PARAMETERS", "Invalid query parameters");
    } catch (const std::exception& e) {
        CHAT_ARCHIVE_LOG_ERROR("Error handling get users request: {}", e.what());
        send_error_response(res, 500, "INTERNAL_ERROR", "Internal server error");
    }
}

void UserController::handle_get_user(const httplib::Request& req, httplib::Response& res) {
    CHAT_ARCHIVE_LOG_INFO("Received GET request for /api/users/{}", std::string(req.matches[1]));
    
    try {
        // 解析URL参数中的用户ID
        int64_t user_id = std::stoll(req.matches[1]);
        
        // 调用UserService获取用户
        auto user = user_service_.get_user_by_id(user_id);
        
        if (!user) {
            send_error_response(res, 404, "USER_NOT_FOUND", "User not found");
            return;
        }
        
        // 构建成功响应
        json response_data;
        response_data["id"] = user->get_id();
        response_data["name"] = user->get_name();
        std::time_t created_at_time = std::chrono::system_clock::to_time_t(user->get_created_at());
        std::tm created_at_tm = *std::localtime(&created_at_time);
        std::stringstream created_at_ss;
        created_at_ss << std::put_time(&created_at_tm, "%Y-%m-%d %H:%M:%S");
        response_data["created_at"] = created_at_ss.str();
        
        send_success_response(res, response_data);
        
        CHAT_ARCHIVE_LOG_INFO("User retrieved successfully with ID: {}", user_id);
        
    } catch (const std::invalid_argument& e) {
        CHAT_ARCHIVE_LOG_WARN("Invalid user ID format: {}", std::string(req.matches[1]));
        send_error_response(res, 400, "INVALID_USER_ID", "Invalid user ID format");
    } catch (const std::exception& e) {
        CHAT_ARCHIVE_LOG_ERROR("Error handling get user request: {}", e.what());
        send_error_response(res, 500, "INTERNAL_ERROR", "Internal server error");
    }
}

void UserController::send_success_response(httplib::Response& res, const json& data) {
    json response;
    response["data"] = data;
    
    res.status = 200;
    res.set_content(response.dump(), "application/json");
}

void UserController::send_error_response(httplib::Response& res, int status_code, const std::string& error_code, const std::string& message) {
    json response;
    response["error_code"] = error_code;
    response["message"] = message;
    
    res.status = status_code;
    res.set_content(response.dump(), "application/json");
}

} // namespace controller
} // namespace chat_archive