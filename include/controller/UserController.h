#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include "server/HttpServer.h"
#include "service/UserService.h"
#include <memory>

namespace controller {

class UserController {
public:
    UserController(std::shared_ptr<service::UserService> user_service, 
                   std::shared_ptr<server::HttpServer> http_server);
    ~UserController() = default;

    // 禁止拷贝构造函数和赋值运算符
    UserController(const UserController&) = delete;
    UserController& operator=(const UserController&) = delete;

    // 允许移动构造函数和赋值运算符
    UserController(UserController&&) noexcept = default;
    UserController& operator=(UserController&&) noexcept = default;

    /**
     * @brief 注册用户相关的API端点
     */
    void registerEndpoints();

private:
    /**
     * @brief 处理用户注册请求
     * @param request HTTP请求
     * @param response HTTP响应
     */
    void handleRegister(const server::http::request<server::http::string_body>& request, 
                        server::http::response<server::http::string_body>& response);

    /**
     * @brief 处理用户登录请求
     * @param request HTTP请求
     * @param response HTTP响应
     */
    void handleLogin(const server::http::request<server::http::string_body>& request, 
                     server::http::response<server::http::string_body>& response);

    /**
     * @brief 处理用户退出登录请求
     * @param request HTTP请求
     * @param response HTTP响应
     */
    void handleLogout(const server::http::request<server::http::string_body>& request, 
                      server::http::response<server::http::string_body>& response);

private:
    std::shared_ptr<service::UserService> user_service_;
    std::shared_ptr<server::HttpServer> http_server_;
};

} // namespace controller

#endif // USER_CONTROLLER_H