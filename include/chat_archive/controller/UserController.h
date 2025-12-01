#pragma once

#include "chat_archive/service/UserService.h"
#include <httplib.h>
#include <nlohmann/json.hpp>

namespace chat_archive {
namespace controller {

using json = nlohmann::json;

// 用户控制器类
class UserController {
public:
    explicit UserController(service::UserService& user_service)
        : user_service_(user_service) {}
    ~UserController() = default;
    
    // 初始化路由
    void init_routes(httplib::Server& server);
    
private:
    // 处理创建用户请求
    void handle_create_user(const httplib::Request& req, httplib::Response& res);
    
    // 处理获取用户列表请求
    void handle_get_users(const httplib::Request& req, httplib::Response& res);
    
    // 处理获取单个用户请求
    void handle_get_user(const httplib::Request& req, httplib::Response& res);
    
    // 发送成功响应
    void send_success_response(httplib::Response& res, const json& data);
    
    // 发送错误响应
    void send_error_response(httplib::Response& res, int status_code, const std::string& error_code, const std::string& message);
    
    service::UserService& user_service_;
};

} // namespace controller
} // namespace chat_archive