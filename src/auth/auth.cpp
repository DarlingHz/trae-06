#include "auth/auth.h"
#include "services/user_service.h"
#include <cpprest/json.h>
#include <chrono>
#include <ctime>
#include <map>
#include <set>
#include <stdexcept>
#include <sstream>
#include <algorithm>

// 注意：实际项目中应使用专门的JWT库（如jwt-cpp），此处为简化示例

namespace auth {

using namespace std;
using namespace std::chrono;
using namespace web;
using namespace models;

std::string permission_to_string(Permission permission) {
    switch (permission) {
        case Permission::USER_CREATE: return "user.create";
        case Permission::USER_READ: return "user.read";
        case Permission::USER_UPDATE: return "user.update";
        case Permission::USER_DELETE: return "user.delete";
        case Permission::ANNOUNCEMENT_CREATE: return "announcement.create";
        case Permission::ANNOUNCEMENT_READ: return "announcement.read";
        case Permission::ANNOUNCEMENT_UPDATE: return "announcement.update";
        case Permission::ANNOUNCEMENT_DELETE: return "announcement.delete";
        case Permission::ANNOUNCEMENT_PUBLISH: return "announcement.publish";
        case Permission::ANNOUNCEMENT_REVIEW: return "announcement.review";
        case Permission::READ_RECEIPT_CREATE: return "read_receipt.create";
        case Permission::READ_RECEIPT_READ: return "read_receipt.read";
        case Permission::READ_RECEIPT_STATS: return "read_receipt.stats";
        case Permission::SYSTEM_ADMIN: return "system.admin";
        case Permission::SYSTEM_CONFIG: return "system.config";
        case Permission::SYSTEM_MONITOR: return "system.monitor";
        default: return "unknown";
    }
}

Permission string_to_permission(const std::string& str) {
    if (str == "user.create") return Permission::USER_CREATE;
    if (str == "user.read") return Permission::USER_READ;
    if (str == "user.update") return Permission::USER_UPDATE;
    if (str == "user.delete") return Permission::USER_DELETE;
    if (str == "announcement.create") return Permission::ANNOUNCEMENT_CREATE;
    if (str == "announcement.read") return Permission::ANNOUNCEMENT_READ;
    if (str == "announcement.update") return Permission::ANNOUNCEMENT_UPDATE;
    if (str == "announcement.delete") return Permission::ANNOUNCEMENT_DELETE;
    if (str == "announcement.publish") return Permission::ANNOUNCEMENT_PUBLISH;
    if (str == "announcement.review") return Permission::ANNOUNCEMENT_REVIEW;
    if (str == "read_receipt.create") return Permission::READ_RECEIPT_CREATE;
    if (str == "read_receipt.read") return Permission::READ_RECEIPT_READ;
    if (str == "read_receipt.stats") return Permission::READ_RECEIPT_STATS;
    if (str == "system.admin") return Permission::SYSTEM_ADMIN;
    if (str == "system.config") return Permission::SYSTEM_CONFIG;
    if (str == "system.monitor") return Permission::SYSTEM_MONITOR;
    
    throw std::invalid_argument("Invalid permission string: " + str);
}

std::string role_to_string(Role role) {
    switch (role) {
        case Role::GUEST: return "guest";
        case Role::USER: return "user";
        case Role::DEPT_HEAD: return "dept_head";
        case Role::HR: return "hr";
        case Role::ADMIN: return "admin";
        case Role::SUPER_ADMIN: return "super_admin";
        default: return "unknown";
    }
}

Role string_to_role(const std::string& str) {
    if (str == "guest") return Role::GUEST;
    if (str == "user") return Role::USER;
    if (str == "dept_head") return Role::DEPT_HEAD;
    if (str == "hr") return Role::HR;
    if (str == "admin") return Role::ADMIN;
    if (str == "super_admin") return Role::SUPER_ADMIN;
    
    throw std::invalid_argument("Invalid role string: " + str);
}

PermissionSet get_permissions_for_role(Role role) {
    PermissionSet permissions;
    
    switch (role) {
        case Role::GUEST:
            permissions.insert(Permission::ANNOUNCEMENT_READ);
            break;
            
        case Role::USER:
            permissions.insert(Permission::ANNOUNCEMENT_READ);
            permissions.insert(Permission::READ_RECEIPT_CREATE);
            permissions.insert(Permission::READ_RECEIPT_READ);
            permissions.insert(Permission::USER_READ);
            permissions.insert(Permission::USER_UPDATE);
            break;
            
        case Role::DEPT_HEAD:
            // 包含普通用户的所有权限
            permissions.insert(Permission::ANNOUNCEMENT_READ);
            permissions.insert(Permission::READ_RECEIPT_CREATE);
            permissions.insert(Permission::READ_RECEIPT_READ);
            permissions.insert(Permission::USER_READ);
            permissions.insert(Permission::USER_UPDATE);
            // 额外权限
            permissions.insert(Permission::ANNOUNCEMENT_CREATE);
            permissions.insert(Permission::ANNOUNCEMENT_PUBLISH);
            permissions.insert(Permission::READ_RECEIPT_STATS);
            break;
            
        case Role::HR:
            // 包含普通用户的所有权限
            permissions.insert(Permission::ANNOUNCEMENT_READ);
            permissions.insert(Permission::READ_RECEIPT_CREATE);
            permissions.insert(Permission::READ_RECEIPT_READ);
            permissions.insert(Permission::USER_READ);
            permissions.insert(Permission::USER_UPDATE);
            // 额外权限
            permissions.insert(Permission::USER_CREATE);
            permissions.insert(Permission::USER_UPDATE);
            permissions.insert(Permission::ANNOUNCEMENT_CREATE);
            permissions.insert(Permission::ANNOUNCEMENT_PUBLISH);
            permissions.insert(Permission::ANNOUNCEMENT_REVIEW);
            permissions.insert(Permission::READ_RECEIPT_STATS);
            break;
            
        case Role::ADMIN:
            // 包含HR的所有权限
            permissions.insert(Permission::USER_CREATE);
            permissions.insert(Permission::USER_READ);
            permissions.insert(Permission::USER_UPDATE);
            permissions.insert(Permission::ANNOUNCEMENT_CREATE);
            permissions.insert(Permission::ANNOUNCEMENT_READ);
            permissions.insert(Permission::ANNOUNCEMENT_PUBLISH);
            permissions.insert(Permission::ANNOUNCEMENT_REVIEW);
            permissions.insert(Permission::READ_RECEIPT_CREATE);
            permissions.insert(Permission::READ_RECEIPT_READ);
            permissions.insert(Permission::READ_RECEIPT_STATS);
            // 额外权限
            permissions.insert(Permission::USER_DELETE);
            permissions.insert(Permission::ANNOUNCEMENT_DELETE);
            permissions.insert(Permission::SYSTEM_CONFIG);
            permissions.insert(Permission::SYSTEM_MONITOR);
            break;
            
        case Role::SUPER_ADMIN:
            // 所有权限
            permissions.insert(Permission::USER_CREATE);
            permissions.insert(Permission::USER_READ);
            permissions.insert(Permission::USER_UPDATE);
            permissions.insert(Permission::USER_DELETE);
            permissions.insert(Permission::ANNOUNCEMENT_CREATE);
            permissions.insert(Permission::ANNOUNCEMENT_READ);
            permissions.insert(Permission::ANNOUNCEMENT_UPDATE);
            permissions.insert(Permission::ANNOUNCEMENT_DELETE);
            permissions.insert(Permission::ANNOUNCEMENT_PUBLISH);
            permissions.insert(Permission::ANNOUNCEMENT_REVIEW);
            permissions.insert(Permission::READ_RECEIPT_CREATE);
            permissions.insert(Permission::READ_RECEIPT_READ);
            permissions.insert(Permission::READ_RECEIPT_STATS);
            permissions.insert(Permission::SYSTEM_ADMIN);
            permissions.insert(Permission::SYSTEM_CONFIG);
            permissions.insert(Permission::SYSTEM_MONITOR);
            break;
    }
    
    return permissions;
}

bool has_permission(const PermissionSet& user_permissions, Permission permission) {
    return user_permissions.count(permission) > 0;
}

bool has_any_permission(const PermissionSet& user_permissions, const PermissionSet& required_permissions) {
    for (Permission p : required_permissions) {
        if (user_permissions.count(p) > 0) {
            return true;
        }
    }
    return false;
}

bool has_all_permissions(const PermissionSet& user_permissions, const PermissionSet& required_permissions) {
    for (Permission p : required_permissions) {
        if (user_permissions.count(p) == 0) {
            return false;
        }
    }
    return true;
}

std::optional<std::string> extract_auth_token(const std::string& auth_header) {
    if (auth_header.size() < 8) { // "Bearer " is 7 characters
        return std::nullopt;
    }
    
    std::string prefix = auth_header.substr(0, 7);
    if (prefix != "Bearer ") {
        return std::nullopt;
    }
    
    return auth_header.substr(7);
}

// JWT工具类（简化实现）
class JwtUtils {
public:
    static std::string encode(const json::value& payload, const std::string& secret_key) {
        // 实际实现应该使用HS256等算法进行签名
        // 此处为简化示例，实际项目中请使用真实的JWT库
        
        std::string header = R"({"alg":"HS256","typ":"JWT"})";
        std::string payload_str = payload.serialize();
        
        std::ostringstream oss;
        oss << encode_base64url(header) << "." << encode_base64url(payload_str);
        std::string signature_base = oss.str();
        
        std::string signature = generate_signature(signature_base, secret_key);
        
        return signature_base << "." << encode_base64url(signature);
    }
    
    static std::optional<json::value> decode(const std::string& token, const std::string& secret_key) {
        // 实际实现应该验证签名和过期时间
        // 此处为简化示例
        
        size_t pos1 = token.find(".");
        size_t pos2 = token.find(".", pos1 + 1);
        
        if (pos1 == std::string::npos || pos2 == std::string::npos) {
            return std::nullopt;
        }
        
        try {
            std::string payload_str = decode_base64url(token.substr(pos1 + 1, pos2 - pos1 - 1));
            return json::value::parse(payload_str);
        } catch (...) {
            return std::nullopt;
        }
    }
    
private:
    static std::string encode_base64url(const std::string& input) {
        // 简化的Base64URL编码实现
        // 实际项目中应使用标准库或专门的编码库
        return input;
    }
    
    static std::string decode_base64url(const std::string& input) {
        // 简化的Base64URL解码实现
        return input;
    }
    
    static std::string generate_signature(const std::string& input, const std::string& secret_key) {
        // 简化的签名实现
        // 实际项目中应使用HMAC-SHA256等算法
        return input + "|" + secret_key;
    }
};

// 认证服务实现类
class JwtAuthService : public AuthService {
public:
    JwtAuthService(std::shared_ptr<services::UserService> user_service)
        : user_service_(std::move(user_service)) {
        if (!user_service_) {
            throw std::invalid_argument("user_service cannot be null");
        }
    }
    
    void init(const JwtConfig& config) override {
        config_ = config;
    }
    
    std::pair<TokenInfo, TokenInfo> login(const std::string& username, const std::string& password) override {
        // 验证用户凭证
        auto user = user_service_->get_user_by_username(username);
        
        if (!user || !user_service_->verify_password(*user, password)) {
            throw AuthenticationException("Invalid username or password");
        }
        
        if (user->get_status() != User::Status::ACTIVE) {
            throw AuthenticationException("User is not active");
        }
        
        // 生成访问令牌和刷新令牌
        TokenInfo access_token = generate_access_token(user);
        TokenInfo refresh_token = generate_refresh_token(user);
        
        // 存储刷新令牌（实际项目中应存储到数据库）
        store_refresh_token(refresh_token.token, user->get_id());
        
        return {access_token, refresh_token};
    }
    
    AuthResult verify_token(const std::string& token) override {
        AuthResult result;
        result.success = false;
        
        try {
            // 解析JWT令牌
            auto payload_opt = JwtUtils::decode(token, config_.secret_key);
            if (!payload_opt) {
                result.error_message = "Invalid token format";
                return result;
            }
            
            json::value& payload = *payload_opt;
            
            // 验证令牌类型
            if (!payload.has_field(U("token_type")) || 
                utility::conversions::to_utf8string(payload[U("token_type")].as_string()) != "access") {
                result.error_message = "Invalid token type";
                return result;
            }
            
            // 验证过期时间
            std::time_t now = time(nullptr);
            if (payload.has_field(U("exp"))) {
                std::time_t exp = payload[U("exp")].as_integer();
                if (exp < now) {
                    throw TokenExpiredException();
                }
            }
            
            // 获取用户信息
            std::string user_id = utility::conversions::to_utf8string(payload[U("user_id")].as_string());
            std::string username = utility::conversions::to_utf8string(payload[U("username")].as_string());
            std::string role_str = utility::conversions::to_utf8string(payload[U("role")].as_string());
            
            Role role = string_to_role(role_str);
            PermissionSet permissions = get_permissions_for_role(role);
            
            result.success = true;
            result.user_id = user_id;
            result.username = username;
            result.role = role;
            result.permissions = permissions;
            
        } catch (const TokenExpiredException& e) {
            result.error_message = e.what();
        } catch (const std::exception& e) {
            result.error_message = std::string("Token verification failed: ") + e.what();
        }
        
        return result;
    }
    
    TokenInfo refresh_token(const std::string& refresh_token) override {
        // 验证刷新令牌
        std::string user_id;
        if (!verify_refresh_token(refresh_token, user_id)) {
            throw AuthenticationException("Invalid refresh token");
        }
        
        // 获取用户信息
        auto user = user_service_->get_user_by_id(std::stoi(user_id));
        if (!user) {
            throw AuthenticationException("User not found");
        }
        
        // 生成新的访问令牌
        return generate_access_token(user);
    }
    
    void logout(const std::string& refresh_token) override {
        // 移除刷新令牌
        remove_refresh_token(refresh_token);
    }
    
    bool has_permission(const std::string& user_id, Permission permission) override {
        auto user = user_service_->get_user_by_id(std::stoi(user_id));
        if (!user) {
            return false;
        }
        
        Role role = string_to_role(user->get_role());
        PermissionSet permissions = get_permissions_for_role(role);
        
        return permissions.count(permission) > 0;
    }
    
    bool has_any_permission(const std::string& user_id, const PermissionSet& permissions) override {
        auto user = user_service_->get_user_by_id(std::stoi(user_id));
        if (!user) {
            return false;
        }
        
        Role role = string_to_role(user->get_role());
        PermissionSet user_perms = get_permissions_for_role(role);
        
        for (Permission p : permissions) {
            if (user_perms.count(p) > 0) {
                return true;
            }
        }
        
        return false;
    }
    
    bool has_all_permissions(const std::string& user_id, const PermissionSet& permissions) override {
        auto user = user_service_->get_user_by_id(std::stoi(user_id));
        if (!user) {
            return false;
        }
        
        Role role = string_to_role(user->get_role());
        PermissionSet user_perms = get_permissions_for_role(role);
        
        for (Permission p : permissions) {
            if (user_perms.count(p) == 0) {
                return false;
            }
        }
        
        return true;
    }
    
    PermissionSet get_user_permissions(const std::string& user_id) override {
        auto user = user_service_->get_user_by_id(std::stoi(user_id));
        if (!user) {
            return PermissionSet();
        }
        
        Role role = string_to_role(user->get_role());
        return get_permissions_for_role(role);
    }
    
    std::string generate_password_reset_token(const std::string& user_id) override {
        // 生成密码重置令牌（简化实现）
        json::value payload;
        payload[U("user_id")] = json::value::string(utility::conversions::to_string_t(user_id));
        payload[U("token_type")] = json::value::string(U("password_reset"));
        payload[U("exp")] = json::value::number(static_cast<int64_t>(time(nullptr) + 3600)); // 1小时过期
        
        return JwtUtils::encode(payload, config_.secret_key + "_reset");
    }
    
    bool verify_password_reset_token(const std::string& token, std::string& out_user_id) override {
        try {
            auto payload_opt = JwtUtils::decode(token, config_.secret_key + "_reset");
            if (!payload_opt) {
                return false;
            }
            
            json::value& payload = *payload_opt;
            
            // 验证令牌类型
            if (!payload.has_field(U("token_type")) || 
                utility::conversions::to_utf8string(payload[U("token_type")].as_string()) != "password_reset") {
                return false;
            }
            
            // 验证过期时间
            std::time_t now = time(nullptr);
            if (payload.has_field(U("exp"))) {
                std::time_t exp = payload[U("exp")].as_integer();
                if (exp < now) {
                    return false;
                }
            }
            
            // 获取用户ID
            out_user_id = utility::conversions::to_utf8string(payload[U("user_id")].as_string());
            
            return true;
            
        } catch (...) {
            return false;
        }
    }
    
private:
    std::shared_ptr<services::UserService> user_service_;
    JwtConfig config_;
    std::map<std::string, std::string> refresh_tokens_; // 实际项目中应存储到数据库
    
    TokenInfo generate_access_token(const std::shared_ptr<User>& user) {
        std::time_t now = time(nullptr);
        std::time_t expires_at = now + config_.access_token_expiry_hours * 3600;
        
        json::value payload;
        payload[U("iss")] = json::value::string(utility::conversions::to_string_t(config_.issuer));
        payload[U("aud")] = json::value::string(utility::conversions::to_string_t(config_.audience));
        payload[U("sub")] = json::value::string(utility::conversions::to_string_t(std::to_string(user->get_id())));
        payload[U("user_id")] = json::value::string(utility::conversions::to_string_t(std::to_string(user->get_id())));
        payload[U("username")] = json::value::string(utility::conversions::to_string_t(user->get_username()));
        payload[U("email")] = json::value::string(utility::conversions::to_string_t(user->get_email()));
        payload[U("role")] = json::value::string(utility::conversions::to_string_t(user->get_role()));
        payload[U("dept_id")] = json::value::string(utility::conversions::to_string_t(user->get_dept_id()));
        payload[U("token_type")] = json::value::string(U("access"));
        payload[U("iat")] = json::value::number(static_cast<int64_t>(now));
        payload[U("exp")] = json::value::number(static_cast<int64_t>(expires_at));
        
        TokenInfo token_info;
        token_info.token = JwtUtils::encode(payload, config_.secret_key);
        token_info.expires_at = expires_at;
        token_info.issued_at = now;
        token_info.token_type = "Bearer";
        
        return token_info;
    }
    
    TokenInfo generate_refresh_token(const std::shared_ptr<User>& user) {
        std::time_t now = time(nullptr);
        std::time_t expires_at = now + config_.refresh_token_expiry_days * 24 * 3600;
        
        json::value payload;
        payload[U("user_id")] = json::value::string(utility::conversions::to_string_t(std::to_string(user->get_id())));
        payload[U("token_type")] = json::value::string(U("refresh"));
        payload[U("iat")] = json::value::number(static_cast<int64_t>(now));
        payload[U("exp")] = json::value::number(static_cast<int64_t>(expires_at));
        
        TokenInfo token_info;
        token_info.token = JwtUtils::encode(payload, config_.secret_key + "_refresh");
        token_info.expires_at = expires_at;
        token_info.issued_at = now;
        token_info.token_type = "Bearer";
        
        return token_info;
    }
    
    void store_refresh_token(const std::string& token, const std::string& user_id) {
        // 实际项目中应存储到数据库，并设置过期时间
        refresh_tokens_[token] = user_id;
    }
    
    void remove_refresh_token(const std::string& token) {
        refresh_tokens_.erase(token);
    }
    
    bool verify_refresh_token(const std::string& token, std::string& out_user_id) {
        try {
            auto payload_opt = JwtUtils::decode(token, config_.secret_key + "_refresh");
            if (!payload_opt) {
                return false;
            }
            
            json::value& payload = *payload_opt;
            
            // 验证令牌类型
            if (!payload.has_field(U("token_type")) || 
                utility::conversions::to_utf8string(payload[U("token_type")].as_string()) != "refresh") {
                return false;
            }
            
            // 验证过期时间
            std::time_t now = time(nullptr);
            if (payload.has_field(U("exp"))) {
                std::time_t exp = payload[U("exp")].as_integer();
                if (exp < now) {
                    return false;
                }
            }
            
            // 验证存储的令牌
            auto it = refresh_tokens_.find(token);
            if (it == refresh_tokens_.end()) {
                return false;
            }
            
            out_user_id = it->second;
            
            return true;
            
        } catch (...) {
            return false;
        }
    }
};

std::shared_ptr<AuthService> create_auth_service(const std::shared_ptr<services::UserService>& user_service) {
    return std::make_shared<JwtAuthService>(user_service);
}

} // namespace auth
