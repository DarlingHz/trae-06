#ifndef USER_REPOSITORY_H
#define USER_REPOSITORY_H

#include "models/user.h"
#include <optional>

class UserRepository {
private:
    // 私有构造函数，实现单例模式
    UserRepository();
    UserRepository(const UserRepository&) = delete;
    UserRepository& operator=(const UserRepository&) = delete;

public:
    // 单例模式获取实例
    static UserRepository& getInstance();

    // 析构函数
    ~UserRepository();

    // 根据用户ID获取用户
    std::optional<User> getUserById(int id);

    // 根据邮箱获取用户
    std::optional<User> getUserByEmail(const std::string& email);

    // 检查用户名是否存在
    bool checkUsernameExists(const std::string& username);

    // 检查邮箱是否存在
    bool checkEmailExists(const std::string& email);

    // 创建新用户
    std::optional<User> createUser(const User& user);

    // 更新用户的问题数量
    bool updateQuestionCount(int user_id, int count);

    // 更新用户的回答数量
    bool updateAnswerCount(int user_id, int count);
};

#endif // USER_REPOSITORY_H
