#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <cpprest/http_listener.h>
#include "auth/auth.h"

namespace auth {

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

// HTTP请求处理函数类型
using RequestHandler = std::function<void(http_request)>;

// 权限检查中间件类
class AuthMiddleware {
public:
    // 构造函数
    explicit AuthMiddleware(std::shared_ptr<AuthService> auth_service);
    
    // 析构函数
    ~AuthMiddleware();
    
    // 中间件函数：检查认证和权限
    // 返回包装后的请求处理函数
    RequestHandler authenticate(Permission required_permission = Permission::ANNOUNCEMENT_READ) const;
    
    // 中间件函数：检查多个权限中的至少一个
    RequestHandler authenticate_any(const PermissionSet& required_permissions) const;
    
    // 中间件函数：检查多个权限中的所有权限
    RequestHandler authenticate_all(const PermissionSet& required_permissions) const;
    
    // 中间件函数：只检查认证（不检查特定权限）
    RequestHandler authenticate_only() const;
    
    // 中间件函数：公开访问（不需要认证）
    RequestHandler public_access() const;
    
    // 从请求中获取认证结果
    std::optional<AuthResult> get_auth_result(const http_request& request) const;
    
    // 设置是否启用权限检查
    void set_enabled(bool enabled);
    
    // 获取是否启用权限检查
    bool is_enabled() const;
    
private:
    std::shared_ptr<AuthService> auth_service_;
    bool enabled_;
    
    // 通用权限检查函数
    RequestHandler create_middleware(
        std::function<bool(const AuthResult&)> permission_checker,
        const std::string& error_message = "Permission denied"
    ) const;
    
    // 验证请求的认证信息
    std::optional<AuthResult> validate_request(const http_request& request) const;
};

// 角色中间件：检查用户是否有指定角色
class RoleMiddleware {
public:
    explicit RoleMiddleware(std::shared_ptr<AuthService> auth_service);
    
    // 检查单个角色
    RequestHandler require_role(Role required_role) const;
    
    // 检查多个角色中的至少一个
    RequestHandler require_any_role(const std::vector<Role>& required_roles) const;
    
    // 检查多个角色中的所有角色
    RequestHandler require_all_roles(const std::vector<Role>& required_roles) const;
    
private:
    std::shared_ptr<AuthService> auth_service_;
};

// 权限检查辅助函数
std::function<void(http_request)> require_auth(
    const std::shared_ptr<AuthService>& auth_service,
    Permission required_permission,
    const RequestHandler& handler
);

std::function<void(http_request)> require_auth_any(
    const std::shared_ptr<AuthService>& auth_service,
    const PermissionSet& required_permissions,
    const RequestHandler& handler
);

std::function<void(http_request)> require_auth_all(
    const std::shared_ptr<AuthService>& auth_service,
    const PermissionSet& required_permissions,
    const RequestHandler& handler
);

std::function<void(http_request)> require_auth_only(
    const std::shared_ptr<AuthService>& auth_service,
    const RequestHandler& handler
);

std::function<void(http_request)> public_access(
    const RequestHandler& handler
);

// 向响应添加认证相关的头信息
void add_auth_headers(http_response& response, const AuthResult* auth_result = nullptr);

// 创建认证失败的响应
http_response create_auth_failure_response(const std::string& error_message = "Authentication failed");

// 创建权限不足的响应
http_response create_permission_denied_response(const std::string& error_message = "Permission denied");

// 创建令牌过期的响应
http_response create_token_expired_response();

// 创建验证失败的响应
http_response create_validation_failed_response(const std::string& error_message = "Validation failed");

} // namespace auth

#endif // MIDDLEWARE_H
