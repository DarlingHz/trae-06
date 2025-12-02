#ifndef USER_SERVICE_H
#define USER_SERVICE_H

#include "service/ServiceException.h"
#include "repository/UserRepository.h"
#include "model/User.h"
#include <string>
#include <optional>
#include <unordered_map>
#include <mutex>

namespace service {

class UserService {
public:
    UserService(repository::UserRepository& user_repository);
    ~UserService() = default;

    // 禁止拷贝构造和赋值运算符
    UserService(const UserService&) = delete;
    UserService& operator=(const UserService&) = delete;

    // 禁止移动构造和赋值运算符，因为 std::mutex 是不可移动的
    UserService(UserService&&) noexcept = delete;
    UserService& operator=(UserService&&) noexcept = delete;

    /**
     * @brief 用户注册
     * @param username 用户名
     * @param password 密码
     * @return 注册成功的用户对象
     * @throws ServiceException 如果用户名已存在或参数不合法
     */
    model::User registerUser(const std::string& username, const std::string& password);

    /**
     * @brief 用户登录
     * @param username 用户名
     * @param password 密码
     * @return 登录成功的token
     * @throws ServiceException 如果用户名或密码错误
     */
    std::string loginUser(const std::string& username, const std::string& password);

    /**
     * @brief 验证token是否有效
     * @param token token字符串
     * @return 如果token有效，返回对应的用户ID；否则返回std::nullopt
     */
    std::optional<int> validateToken(const std::string& token);

    /**
     * @brief 退出登录
     * @param token token字符串
     */
    void logoutUser(const std::string& token);

    /**
     * @brief 根据用户ID获取用户信息
     * @param user_id 用户ID
     * @return 用户对象
     * @throws ServiceException 如果用户不存在
     */
    model::User getUserById(int user_id);

private:
    /**
     * @brief 生成随机token
     * @return 生成的token字符串
     */
    std::string generateToken();

    /**
     * @brief 对密码进行哈希
     * @param password 原始密码
     * @return 哈希后的密码
     */
    std::string hashPassword(const std::string& password);

    /**
     * @brief 验证密码是否正确
     * @param password 原始密码
     * @param hashed_password 哈希后的密码
     * @return 如果密码正确返回true，否则返回false
     */
    bool verifyPassword(const std::string& password, const std::string& hashed_password);

private:
    repository::UserRepository& user_repository_;
    std::unordered_map<std::string, int> token_user_map_; // token到用户ID的映射
    std::mutex token_map_mutex_; // 保护token_user_map_的互斥锁
};

} // namespace service

#endif // USER_SERVICE_H