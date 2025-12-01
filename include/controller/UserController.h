#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H

#include <string>
#include <nlohmann/json.hpp>
#include "dao/UserDao.h"

using json = nlohmann::json;

namespace controller {

class UserController {
public:
    UserController(dao::UserDao& user_dao);
    ~UserController();

    // 处理用户注册请求
    json handleRegister(const json& request) const;
    // 处理用户登录请求
    json handleLogin(const json& request) const;

private:
    dao::UserDao& user_dao_;

    // 生成随机token
    std::string generateToken() const;
    // 计算密码哈希
    std::string hashPassword(const std::string& password) const;
};

} // namespace controller

#endif // USERCONTROLLER_H
