#ifndef USER_DAO_H
#define USER_DAO_H

#include <memory>
#include <vector>
#include "../model/User.h"
#include "../util/DatabaseConnectionPool.h"

class UserDAO {
public:
    // 构造函数
    UserDAO() = default;
    
    // 析构函数
    ~UserDAO() = default;
    
    // 注册用户
    bool registerUser(const User& user);
    
    // 根据用户名查询用户
    std::shared_ptr<User> getUserByUsername(const std::string& username);
    
    // 根据邮箱查询用户
    std::shared_ptr<User> getUserByEmail(const std::string& email);
    
    // 根据用户ID查询用户
    std::shared_ptr<User> getUserById(int user_id);
    
    // 更新用户信息
    bool updateUser(const User& user);
    
    // 更新用户密码
    bool updateUserPassword(int user_id, const std::string& new_password_hash);
    
    // 获取所有用户（管理员功能）
    std::vector<std::shared_ptr<User>> getAllUsers(int page = 1, int page_size = 10);
    
    // 获取用户总数（管理员功能）
    int getUserCount();
    
    // 禁用/启用用户（管理员功能）
    bool toggleUserStatus(int user_id, const std::string& status);
    
private:
    // 从数据库结果集中创建User对象
    std::shared_ptr<User> createUserFromResult(const Row& row);
};

#endif // USER_DAO_H