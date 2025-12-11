#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include <httplib.h>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

namespace service {
    class UserService;
    class SessionService;
}

namespace controller {

class UserController {
public:
    UserController(
        std::shared_ptr<service::UserService> user_service,
        std::shared_ptr<service::SessionService> session_service);
    ~UserController() = default;

    // 禁用拷贝操作
    UserController(const UserController&) = delete;
    UserController& operator=(const UserController&) = delete;

    // 允许移动操作
    UserController(UserController&&) noexcept = default;
    UserController& operator=(UserController&&) noexcept = default;

    // 注册路由
    void registerRoutes(httplib::Server& server);

private:
    // 处理用户注册请求
    void handleUserRegister(const httplib::Request& req, httplib::Response& res);

    // 处理用户登录请求
    void handleUserLogin(const httplib::Request& req, httplib::Response& res);

    // 处理获取当前用户信息请求
    void handleGetUserInfo(const httplib::Request& req, httplib::Response& res);

    // 验证请求中的token
    std::optional<int> validateToken(const httplib::Request& req);

    // 发送统一格式的JSON响应
    void sendJsonResponse(httplib::Response& res, bool success, int code, const std::string& message, const nlohmann::json& data = nlohmann::json());

    // 服务层依赖
    std::shared_ptr<service::UserService> user_service_;
    std::shared_ptr<service::SessionService> session_service_;
};

} // namespace controller

#endif // USER_CONTROLLER_H