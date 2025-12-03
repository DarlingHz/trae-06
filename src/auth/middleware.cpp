#include "middleware.h"
#include <cpprest/uri.h>
#include <sstream>
#include <algorithm>

namespace auth {

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

// AuthMiddleware 实现
AuthMiddleware::AuthMiddleware(std::shared_ptr<AuthService> auth_service)
    : auth_service_(std::move(auth_service)), enabled_(true) {
}

AuthMiddleware::~AuthMiddleware() {
}

RequestHandler AuthMiddleware::authenticate(Permission required_permission) const {
    return create_middleware(
        [required_permission](const AuthResult& result) {
            return result.user.has_permission(required_permission);
        },
        "Permission denied (missing required permission)"
    );
}

RequestHandler AuthMiddleware::authenticate_any(const PermissionSet& required_permissions) const {
    return create_middleware(
        [&required_permissions](const AuthResult& result) {
            const PermissionSet& user_permissions = result.user.get_permissions();
            
            for (const auto& required : required_permissions) {
                if (user_permissions.count(required)) {
                    return true;
                }
            }
            return false;
        },
        "Permission denied (missing any required permission)"
    );
}

RequestHandler AuthMiddleware::authenticate_all(const PermissionSet& required_permissions) const {
    return create_middleware(
        [&required_permissions](const AuthResult& result) {
            const PermissionSet& user_permissions = result.user.get_permissions();
            
            for (const auto& required : required_permissions) {
                if (!user_permissions.count(required)) {
                    return false;
                }
            }
            return true;
        },
        "Permission denied (missing all required permissions)"
    );
}

RequestHandler AuthMiddleware::authenticate_only() const {
    return create_middleware(
        [](const AuthResult& result) {
            return true; // 只需要认证，不需要特定权限
        },
        "Authentication failed"
    );
}

RequestHandler AuthMiddleware::public_access() const {
    return [](const RequestHandler& handler) {
        return [handler](http_request request) {
            // 不需要认证，直接处理请求
            handler(std::move(request));
        };
    };
}

std::optional<AuthResult> AuthMiddleware::get_auth_result(const http_request& request) const {
    if (!enabled_) {
        return std::nullopt;
    }
    
    return validate_request(request);
}

void AuthMiddleware::set_enabled(bool enabled) {
    enabled_ = enabled;
}

bool AuthMiddleware::is_enabled() const {
    return enabled_;
}

RequestHandler AuthMiddleware::create_middleware(
    std::function<bool(const AuthResult&)> permission_checker,
    const std::string& error_message
) const {
    return [this, permission_checker, error_message](const RequestHandler& handler) {
        return [this, permission_checker, error_message, handler](http_request request) {
            if (!this->enabled_) {
                // 权限检查已禁用，直接处理请求
                handler(std::move(request));
                return;
            }
            
            // 验证请求的认证信息
            auto auth_result = this->validate_request(request);
            
            if (!auth_result) {
                // 认证失败
                request.reply(create_auth_failure_response());
                return;
            }
            
            if (auth_result->status != AuthStatus::SUCCESS) {
                // 令牌状态错误
                switch (auth_result->status) {
                    case AuthStatus::EXPIRED:
                        request.reply(create_token_expired_response());
                        break;
                    case AuthStatus::INVALID_SIGNATURE:
                    case AuthStatus::INVALID_FORMAT:
                    case AuthStatus::INVALID_CLAIMS:
                        request.reply(create_validation_failed_response("Invalid token"));
                        break;
                    default:
                        request.reply(create_auth_failure_response());
                        break;
                }
                return;
            }
            
            // 检查权限
            if (!permission_checker(*auth_result)) {
                request.reply(create_permission_denied_response(error_message));
                return;
            }
            
            // 认证和权限检查通过，继续处理请求
            handler(std::move(request));
        };
    };
}

std::optional<AuthResult> AuthMiddleware::validate_request(const http_request& request) const {
    try {
        // 从Authorization头获取令牌
        const auto& headers = request.headers();
        
        auto auth_header_it = headers.find(U("Authorization"));
        if (auth_header_it == headers.end()) {
            return std::nullopt;
        }
        
        const auto& auth_header = auth_header_it->second;
        
        // 检查令牌前缀
        const utility::string_view bearer_prefix = U("Bearer ");
        if (auth_header.substr(0, bearer_prefix.size()) != bearer_prefix) {
            return std::nullopt;
        }
        
        // 提取令牌
        const auto token = auth_header.substr(bearer_prefix.size());
        const std::string token_str = utility::conversions::to_utf8string(token);
        
        // 验证令牌
        return auth_service_->validate_token(token_str);
        
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

// RoleMiddleware 实现
RoleMiddleware::RoleMiddleware(std::shared_ptr<AuthService> auth_service)
    : auth_service_(std::move(auth_service)) {
}

RequestHandler RoleMiddleware::require_role(Role required_role) const {
    return [this, required_role](const RequestHandler& handler) {
        return [this, required_role, handler](http_request request) {
            try {
                // 验证认证信息
                const auto& headers = request.headers();
                auto auth_header_it = headers.find(U("Authorization"));
                
                if (auth_header_it == headers.end()) {
                    request.reply(create_auth_failure_response());
                    return;
                }
                
                const auto& auth_header = auth_header_it->second;
                const utility::string_view bearer_prefix = U("Bearer ");
                
                if (auth_header.substr(0, bearer_prefix.size()) != bearer_prefix) {
                    request.reply(create_auth_failure_response());
                    return;
                }
                
                const auto token = auth_header.substr(bearer_prefix.size());
                const std::string token_str = utility::conversions::to_utf8string(token);
                
                auto auth_result = auth_service_->validate_token(token_str);
                
                if (!auth_result || auth_result->status != AuthStatus::SUCCESS) {
                    request.reply(create_auth_failure_response());
                    return;
                }
                
                // 检查角色
                if (auth_result->user.role != required_role) {
                    request.reply(create_permission_denied_response("Insufficient role"));
                    return;
                }
                
                // 角色检查通过，继续处理请求
                handler(std::move(request));
                
            } catch (const std::exception& e) {
                request.reply(create_auth_failure_response());
            }
        };
    };
}

RequestHandler RoleMiddleware::require_any_role(const std::vector<Role>& required_roles) const {
    return [this, required_roles](const RequestHandler& handler) {
        return [this, required_roles, handler](http_request request) {
            try {
                // 验证认证信息
                const auto& headers = request.headers();
                auto auth_header_it = headers.find(U("Authorization"));
                
                if (auth_header_it == headers.end()) {
                    request.reply(create_auth_failure_response());
                    return;
                }
                
                const auto& auth_header = auth_header_it->second;
                const utility::string_view bearer_prefix = U("Bearer ");
                
                if (auth_header.substr(0, bearer_prefix.size()) != bearer_prefix) {
                    request.reply(create_auth_failure_response());
                    return;
                }
                
                const auto token = auth_header.substr(bearer_prefix.size());
                const std::string token_str = utility::conversions::to_utf8string(token);
                
                auto auth_result = auth_service_->validate_token(token_str);
                
                if (!auth_result || auth_result->status != AuthStatus::SUCCESS) {
                    request.reply(create_auth_failure_response());
                    return;
                }
                
                // 检查角色
                bool has_required_role = false;
                const Role user_role = auth_result->user.role;
                
                for (const auto& role : required_roles) {
                    if (user_role == role) {
                        has_required_role = true;
                        break;
                    }
                }
                
                if (!has_required_role) {
                    request.reply(create_permission_denied_response("Insufficient role"));
                    return;
                }
                
                // 角色检查通过，继续处理请求
                handler(std::move(request));
                
            } catch (const std::exception& e) {
                request.reply(create_auth_failure_response());
            }
        };
    };
}

RequestHandler RoleMiddleware::require_all_roles(const std::vector<Role>& required_roles) const {
    return [this, required_roles](const RequestHandler& handler) {
        return [this, required_roles, handler](http_request request) {
            try {
                // 验证认证信息
                const auto& headers = request.headers();
                auto auth_header_it = headers.find(U("Authorization"));
                
                if (auth_header_it == headers.end()) {
                    request.reply(create_auth_failure_response());
                    return;
                }
                
                const auto& auth_header = auth_header_it->second;
                const utility::string_view bearer_prefix = U("Bearer ");
                
                if (auth_header.substr(0, bearer_prefix.size()) != bearer_prefix) {
                    request.reply(create_auth_failure_response());
                    return;
                }
                
                const auto token = auth_header.substr(bearer_prefix.size());
                const std::string token_str = utility::conversions::to_utf8string(token);
                
                auto auth_result = auth_service_->validate_token(token_str);
                
                if (!auth_result || auth_result->status != AuthStatus::SUCCESS) {
                    request.reply(create_auth_failure_response());
                    return;
                }
                
                // 检查角色
                const Role user_role = auth_result->user.role;
                
                for (const auto& role : required_roles) {
                    if (user_role != role) {
                        request.reply(create_permission_denied_response("Insufficient role"));
                        return;
                    }
                }
                
                // 角色检查通过，继续处理请求
                handler(std::move(request));
                
            } catch (const std::exception& e) {
                request.reply(create_auth_failure_response());
            }
        };
    };
}

// 权限检查辅助函数
std::function<void(http_request)> require_auth(
    const std::shared_ptr<AuthService>& auth_service,
    Permission required_permission,
    const RequestHandler& handler
) {
    AuthMiddleware middleware(auth_service);
    return middleware.authenticate(required_permission)(handler);
}

std::function<void(http_request)> require_auth_any(
    const std::shared_ptr<AuthService>& auth_service,
    const PermissionSet& required_permissions,
    const RequestHandler& handler
) {
    AuthMiddleware middleware(auth_service);
    return middleware.authenticate_any(required_permissions)(handler);
}

std::function<void(http_request)> require_auth_all(
    const std::shared_ptr<AuthService>& auth_service,
    const PermissionSet& required_permissions,
    const RequestHandler& handler
) {
    AuthMiddleware middleware(auth_service);
    return middleware.authenticate_all(required_permissions)(handler);
}

std::function<void(http_request)> require_auth_only(
    const std::shared_ptr<AuthService>& auth_service,
    const RequestHandler& handler
) {
    AuthMiddleware middleware(auth_service);
    return middleware.authenticate_only()(handler);
}

std::function<void(http_request)> public_access(
    const RequestHandler& handler
) {
    return [handler](http_request request) {
        handler(std::move(request));
    };
}

// 响应辅助函数
void add_auth_headers(http_response& response, const AuthResult* auth_result) {
    // 添加CORS相关头
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, PUT, DELETE, OPTIONS"));
    response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type, Authorization"));
    
    if (auth_result) {
        // 可以添加自定义头
        response.headers().add(U("X-Authenticated-User-ID"), utility::conversions::to_string_t(std::to_string(auth_result->user.id)));
        response.headers().add(U("X-Authenticated-User-Role"), utility::conversions::to_string_t(to_string(auth_result->user.role)));
    }
}

http_response create_auth_failure_response(const std::string& error_message) {
    http_response response(status_codes::Unauthorized);
    response.set_body(json::value::object({
        { U("success"), json::value::boolean(false) },
        { U("error"), json::value::string(utility::conversions::to_string_t(error_message)) },
        { U("code"), json::value::string(U("AUTH_FAILED")) }
    }));
    
    add_auth_headers(response);
    return response;
}

http_response create_permission_denied_response(const std::string& error_message) {
    http_response response(status_codes::Forbidden);
    response.set_body(json::value::object({
        { U("success"), json::value::boolean(false) },
        { U("error"), json::value::string(utility::conversions::to_string_t(error_message)) },
        { U("code"), json::value::string(U("PERMISSION_DENIED")) }
    }));
    
    add_auth_headers(response);
    return response;
}

http_response create_token_expired_response() {
    http_response response(status_codes::Unauthorized);
    response.set_body(json::value::object({
        { U("success"), json::value::boolean(false) },
        { U("error"), json::value::string(U("Token expired")) },
        { U("code"), json::value::string(U("TOKEN_EXPIRED")) }
    }));
    
    add_auth_headers(response);
    return response;
}

http_response create_validation_failed_response(const std::string& error_message) {
    http_response response(status_codes::BadRequest);
    response.set_body(json::value::object({
        { U("success"), json::value::boolean(false) },
        { U("error"), json::value::string(utility::conversions::to_string_t(error_message)) },
        { U("code"), json::value::string(U("VALIDATION_FAILED")) }
    }));
    
    add_auth_headers(response);
    return response;
}

} // namespace auth
