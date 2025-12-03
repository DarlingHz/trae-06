#pragma once

#include <memory>
#include <string>
#include <optional>

#include "models/user.h"
#include "repositories/user_repository.h"

namespace services {

class UserService {
public:
    UserService(std::shared_ptr<repositories::UserRepository> user_repository);
    ~UserService();
    
    UserService(const UserService&) = delete;
    UserService& operator=(const UserService&) = delete;
    
    UserService(UserService&&) = default;
    UserService& operator=(UserService&&) = default;
    
    // 创建用户（仅管理员）
    std::optional<models::User> create_user(const std::string& name, const std::string& email, 
                                           const std::string& department, models::User::Role role,
                                           const std::string& password);
    
    // 按ID查找用户
    std::optional<models::User> get_user_by_id(int user_id);
    
    // 按邮箱查找用户
    std::optional<models::User> get_user_by_email(const std::string& email);
    
    // 按部门查找用户
    std::vector<models::User> get_users_by_department(const std::string& department);
    
    // 获取所有用户
    std::vector<models::User> get_all_users();
    
    // 更新用户信息
    bool update_user(int user_id, const std::optional<std::string>& name = std::nullopt,
                     const std::optional<std::string>& email = std::nullopt,
                     const std::optional<std::string>& department = std::nullopt,
                     const std::optional<models::User::Role>& role = std::nullopt,
                     const std::optional<models::User::Status>& status = std::nullopt);
    
    // 更新用户密码
    bool update_user_password(int user_id, const std::string& new_password);
    
    // 删除用户（软删除）
    bool delete_user(int user_id);
    
    // 验证用户密码
    bool verify_password(int user_id, const std::string& password) const;
    
    // 验证邮箱格式
    static bool validate_email(const std::string& email);
    
    // 验证密码强度
    static bool validate_password_strength(const std::string& password);
    
private:
    std::shared_ptr<repositories::UserRepository> user_repository_;
    
    // 密码哈希
    std::string hash_password(const std::string& password) const;
};

} // namespace services
