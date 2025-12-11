#pragma once

#include <string>
#include <optional>
#include "models/User.h"
#include "models/Token.h"

namespace pet_hospital {

class UserDAO;
class TokenDAO;

class UserService {
public:
    UserService();
    ~UserService();

    // 用户注册
    std::optional<User> register_user(const std::string& email, 
                                        const std::string& password, 
                                        const std::string& name, 
                                        std::string& error_message);

    // 用户登录
    std::optional<Token> login_user(const std::string& email, 
                                      const std::string& password, 
                                      std::string& error_message);

    // 获取用户信息
    std::optional<User> get_user_info(int user_id, 
                                        std::string& error_message);

    // 更新用户信息
    bool update_user_info(int user_id, const std::string& name, 
                           const std::string& phone, 
                           std::string& error_message);

    // 删除用户
    bool delete_user(int user_id, std::string& error_message);

    // 验证token
    std::optional<User> validate_token(const std::string& token, 
                                         std::string& error_message);

private:
    // 密码哈希函数
    std::string hash_password(const std::string& password);

    // 生成随机token
    std::string generate_token();

    // 检查邮箱是否已存在
    bool is_email_exists(const std::string& email);

    std::unique_ptr<UserDAO> user_dao_;
    std::unique_ptr<TokenDAO> token_dao_;
};

} // namespace pet_hospital
