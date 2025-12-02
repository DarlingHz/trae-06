#ifndef USER_H
#define USER_H

#include <string>
#include <nlohmann/json.hpp>

class User {
public:
    // 构造函数
    User() = default;
    User(int id, const std::string& username, const std::string& nickname, const std::string& email,
         const std::string& password_hash, const std::string& role, const std::string& status,
         const std::string& created_at, const std::string& updated_at);
    
    // 获取用户ID
    int getId() const;
    
    // 设置用户ID
    void setId(int id);
    
    // 获取用户名
    std::string getUsername() const;
    
    // 设置用户名
    void setUsername(const std::string& username);
    
    // 获取昵称
    std::string getNickname() const;
    
    // 设置昵称
    void setNickname(const std::string& nickname);
    
    // 获取邮箱
    std::string getEmail() const;
    
    // 设置邮箱
    void setEmail(const std::string& email);
    
    // 获取密码哈希
    std::string getPasswordHash() const;
    
    // 设置密码哈希
    void setPasswordHash(const std::string& password_hash);
    
    // 获取角色类型
    std::string getRole() const;
    
    // 设置角色类型
    void setRole(const std::string& role);
    
    // 获取用户状态
    std::string getStatus() const;
    
    // 设置用户状态
    void setStatus(const std::string& status);
    
    // 获取创建时间
    std::string getCreatedAt() const;
    
    // 设置创建时间
    void setCreatedAt(const std::string& created_at);
    
    // 获取更新时间
    std::string getUpdatedAt() const;
    
    // 设置更新时间
    void setUpdatedAt(const std::string& updated_at);
    
    // 转换为JSON对象
    nlohmann::json toJson() const;
    
    // 从JSON对象转换
    static User fromJson(const nlohmann::json& json_obj);
    
private:
    int id_ = 0;
    std::string username_;
    std::string nickname_;
    std::string email_;
    std::string password_hash_;
    std::string role_ = "reader";
    std::string status_ = "active";
    std::string created_at_;
    std::string updated_at_;
};

#endif // USER_H