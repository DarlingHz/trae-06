#ifndef USER_SERVICE_HPP
#define USER_SERVICE_HPP

#include <string>
#include <nlohmann/json.hpp>
#include "database.hpp"

using json = nlohmann::json;

class UserService {
public:
    UserService() : db_(Database::getInstance()) {}

    // 用户注册
    json registerUser(const json& user_data);

    // 用户登录
    json loginUser(const json& login_data);

    // 验证 token
    int verifyToken(const std::string& token);

    // 生成 token
    std::string generateToken(int user_id);

private:
    Database& db_;

    // 检查邮箱是否已存在
    bool emailExists(const std::string& email);

    // 获取用户 ID 对应的 token
    std::string getTokenForUserId(int user_id);

    // 存储 token
    bool storeToken(int user_id, const std::string& token);

    // 生成随机字符串
    std::string generateRandomString(int length);
};

#endif // USER_SERVICE_HPP
