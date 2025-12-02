#ifndef USER_SERVICE_H
#define USER_SERVICE_H

#include <string>
#include <optional>
#include "../model/User.h"
#include "../repository/UserRepository.h"

namespace service {

// UserService接口，定义用户相关的业务逻辑
class UserService {
public:
    virtual ~UserService() = default;

    // 构造函数，依赖于UserRepository接口
    explicit UserService(std::unique_ptr<repository::UserRepository> user_repository)
        : user_repository_(std::move(user_repository)) {}

    // 注册新用户
    // 参数：username - 用户名
    //      password - 密码
    // 返回：成功则返回创建的用户对象，失败则返回std::nullopt
    // 失败原因可能包括：用户名已存在、参数无效等
    std::optional<model::User> registerUser(const std::string& username, const std::string& password);

    // 验证用户密码
    // 参数：username - 用户名
    //      password - 密码
    // 返回：成功则返回用户对象，失败则返回std::nullopt
    std::optional<model::User> authenticateUser(const std::string& username, const std::string& password);

    // 根据ID查找用户
    // 参数：id - 用户ID
    // 返回：成功则返回用户对象，失败则返回std::nullopt
    std::optional<model::User> findUserById(int id);

    // 根据用户名查找用户
    // 参数：username - 用户名
    // 返回：成功则返回用户对象，失败则返回std::nullopt
    std::optional<model::User> findUserByUsername(const std::string& username);

    // 检查用户名是否存在
    // 参数：username - 用户名
    // 返回：存在则返回true，否则返回false
    bool existsByUsername(const std::string& username);

protected:
    // 密码哈希函数
    // 参数：password - 密码
    // 返回：哈希后的密码字符串
    virtual std::string hashPassword(const std::string& password);

    // 验证密码哈希
    // 参数：password - 原始密码
    //      password_hash - 哈希后的密码
    // 返回：验证成功则返回true，否则返回false
    virtual bool verifyPassword(const std::string& password, const std::string& password_hash);

private:
    // 依赖注入的UserRepository接口
    std::unique_ptr<repository::UserRepository> user_repository_;
};

} // namespace service

#endif // USER_SERVICE_H