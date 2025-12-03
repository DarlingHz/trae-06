#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <memory>
#include <optional>
#include <ctime>
#include <map>
#include <set>

#include "models/user.h"

namespace auth {

// 权限枚举
enum class Permission {
    // 用户权限
    USER_CREATE,           // 创建用户
    USER_READ,             // 读取用户信息
    USER_UPDATE,           // 更新用户信息
    USER_DELETE,           // 删除用户
    
    // 公告权限
    ANNOUNCEMENT_CREATE,   // 创建公告
    ANNOUNCEMENT_READ,     // 读取公告
    ANNOUNCEMENT_UPDATE,   // 更新公告
    ANNOUNCEMENT_DELETE,   // 删除公告
    ANNOUNCEMENT_PUBLISH,  // 发布公告
    ANNOUNCEMENT_REVIEW,   // 审核公告
    
    // 阅读记录权限
    READ_RECEIPT_CREATE,   // 创建阅读记录
    READ_RECEIPT_READ,     // 读取阅读记录
    READ_RECEIPT_STATS,    // 查看阅读统计
    
    // 系统权限
    SYSTEM_ADMIN,          // 系统管理员
    SYSTEM_CONFIG,         // 系统配置
    SYSTEM_MONITOR         // 系统监控
};

// 将权限转换为字符串
std::string permission_to_string(Permission permission);

// 将字符串转换为权限
Permission string_to_permission(const std::string& str);

// 权限集合类型
using PermissionSet = std::set<Permission>;

// 角色枚举
enum class Role {
    GUEST,        // 访客
    USER,         // 普通用户
    DEPT_HEAD,    // 部门负责人
    HR,           // 人力资源
    ADMIN,        // 管理员
    SUPER_ADMIN   // 超级管理员
};

// 将角色转换为字符串
std::string role_to_string(Role role);

// 将字符串转换为角色
Role string_to_role(const std::string& str);

// 角色权限映射
PermissionSet get_permissions_for_role(Role role);

// JWT配置
struct JwtConfig {
    std::string secret_key;         // 密钥
    std::string issuer;             // 发行者
    std::string audience;           // 受众
    int access_token_expiry_hours;  // 访问令牌过期时间（小时）
    int refresh_token_expiry_days;  // 刷新令牌过期时间（天）
};

// JWT令牌信息
struct TokenInfo {
    std::string token;              // 令牌字符串
    std::time_t expires_at;         // 过期时间
    std::time_t issued_at;          // 签发时间
    std::string token_type;         // 令牌类型（Bearer等）
};

// 认证结果
struct AuthResult {
    bool success;                   // 是否认证成功
    std::string user_id;            // 用户ID
    std::string username;           // 用户名
    Role role;                      // 用户角色
    PermissionSet permissions;      // 用户权限
    std::string error_message;      // 错误信息
};

// 认证服务接口
class AuthService {
public:
    virtual ~AuthService() = default;
    
    // 初始化认证服务
    virtual void init(const JwtConfig& config) = 0;
    
    // 用户登录，生成访问令牌和刷新令牌
    virtual std::pair<TokenInfo, TokenInfo> login(const std::string& username, const std::string& password) = 0;
    
    // 验证访问令牌
    virtual AuthResult verify_token(const std::string& token) = 0;
    
    // 使用刷新令牌获取新的访问令牌
    virtual TokenInfo refresh_token(const std::string& refresh_token) = 0;
    
    // 用户登出
    virtual void logout(const std::string& refresh_token) = 0;
    
    // 检查用户是否有指定权限
    virtual bool has_permission(const std::string& user_id, Permission permission) = 0;
    
    // 检查用户是否有多个权限中的至少一个
    virtual bool has_any_permission(const std::string& user_id, const PermissionSet& permissions) = 0;
    
    // 检查用户是否有多个权限中的所有权限
    virtual bool has_all_permissions(const std::string& user_id, const PermissionSet& permissions) = 0;
    
    // 获取用户的所有权限
    virtual PermissionSet get_user_permissions(const std::string& user_id) = 0;
    
    // 生成密码重置令牌
    virtual std::string generate_password_reset_token(const std::string& user_id) = 0;
    
    // 验证密码重置令牌
    virtual bool verify_password_reset_token(const std::string& token, std::string& out_user_id) = 0;
};

// 创建认证服务实例
std::shared_ptr<AuthService> create_auth_service(const std::shared_ptr<services::UserService>& user_service);

// 从HTTP请求头中提取Authorization令牌
std::optional<std::string> extract_auth_token(const std::string& auth_header);

// 权限检查辅助函数
bool has_permission(const PermissionSet& user_permissions, Permission permission);
bool has_any_permission(const PermissionSet& user_permissions, const PermissionSet& required_permissions);
bool has_all_permissions(const PermissionSet& user_permissions, const PermissionSet& required_permissions);

// 权限验证异常
class PermissionDeniedException : public std::runtime_error {
public:
    explicit PermissionDeniedException(const std::string& message = "Permission denied")
        : std::runtime_error(message) {}
};

// 认证异常
class AuthenticationException : public std::runtime_error {
public:
    explicit AuthenticationException(const std::string& message = "Authentication failed")
        : std::runtime_error(message) {}
};

// 令牌过期异常
class TokenExpiredException : public std::runtime_error {
public:
    explicit TokenExpiredException(const std::string& message = "Token expired")
        : std::runtime_error(message) {}
};

} // namespace auth

#endif // AUTH_H
