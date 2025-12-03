#ifndef USER_SERVICE_H
#define USER_SERVICE_H

#include "models/user.h"
#include <string>

class UserService {
private:
    // 私有构造函数，实现单例模式
    UserService();
    UserService(const UserService&) = delete;
    UserService& operator=(const UserService&) = delete;

public:
    // 单例模式获取实例
    static UserService& getInstance();

    // 析构函数
    ~UserService();

    // 注册新用户
    User registerUser(const std::string& username, const std::string& email, const std::string& password);

    // 用户登录
    std::string loginUser(const std::string& email, const std::string& password);

    // 根据ID获取用户
    User getUserById(int id);

    // 根据邮箱获取用户
    User getUserByEmail(const std::string& email);

    // 验证JWT令牌
    bool verifyToken(const std::string& token);

    // 从令牌中获取用户ID
    int getUserIdFromToken(const std::string& token);
};

#endif // USER_SERVICE_H
