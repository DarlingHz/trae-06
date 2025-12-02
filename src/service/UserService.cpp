#include "service/UserService.h"
#include "dao/UserDAO.h"
#include "dao/TokenDAO.h"
#include "logging/Logging.h"
#include <random>
#include <chrono>
#include <algorithm>
#include <cctype>

namespace pet_hospital {

UserService::UserService() {
    user_dao_ = std::make_unique<UserDAO>();
    token_dao_ = std::make_unique<TokenDAO>();
}

UserService::~UserService() {
}

std::optional<User> UserService::register_user(const std::string& email, 
                                        const std::string& password, 
                                        const std::string& name, 
                                        std::string& error_message) {
    try {
        // 验证邮箱格式
        if (email.empty() || email.find("@") == std::string::npos) {
            error_message = "Invalid email format";
            LOG_ERROR("User registration failed: " + error_message);
            return std::nullopt;
        }

        // 验证密码长度
        if (password.empty() || password.length() < 6) {
            error_message = "Password must be at least 6 characters";
            LOG_ERROR("User registration failed: " + error_message);
            return std::nullopt;
        }

        // 验证姓名
        if (name.empty()) {
            error_message = "Name cannot be empty";
            LOG_ERROR("User registration failed: " + error_message);
            return std::nullopt;
        }

        // 检查邮箱是否已存在
        if (is_email_exists(email)) {
            error_message = "Email already exists";
            LOG_ERROR("User registration failed: " + error_message);
            return std::nullopt;
        }

        // 创建用户对象
        User user;
        user.set_email(email);
        user.set_password_hash(hash_password(password));
        user.set_name(name);

        // 保存用户到数据库
        if (!user_dao_->create_user(user)) {
            error_message = "Failed to create user";
            LOG_ERROR("User registration failed: " + error_message);
            return std::nullopt;
        }

        // 获取创建的用户信息
        auto created_user = user_dao_->get_user_by_email(email);
        if (!created_user) {
            error_message = "Failed to retrieve created user";
            LOG_ERROR("User registration failed: " + error_message);
            return std::nullopt;
        }

        LOG_INFO("User registered successfully: " + email);
        return created_user;
    } catch (const std::exception& e) {
        error_message = "Failed to register user: " + std::string(e.what());
        LOG_ERROR("User registration failed: " + error_message);
        return std::nullopt;
    }
}

std::optional<Token> UserService::login_user(const std::string& email, 
                                      const std::string& password, 
                                      std::string& error_message) {
    try {
        // 验证邮箱和密码是否为空
        if (email.empty() || password.empty()) {
            error_message = "Email and password cannot be empty";
            LOG_ERROR("User login failed: " + error_message);
            return std::nullopt;
        }

        // 根据邮箱获取用户信息
        auto user = user_dao_->get_user_by_email(email);
        if (!user) {
            error_message = "Invalid email or password";
            LOG_ERROR("User login failed: " + error_message);
            return std::nullopt;
        }

        // 验证密码
        std::string hashed_password = hash_password(password);
        if (hashed_password != user->get_password_hash()) {
            error_message = "Invalid email or password";
            LOG_ERROR("User login failed: " + error_message);
            return std::nullopt;
        }

        // 生成新的token
        std::string token = generate_token();

        // 创建token对象
        Token token_obj;
        token_obj.set_user_id(user->get_id());
        token_obj.set_token(token);

        // 保存token到数据库
        if (!token_dao_->create_token(token_obj)) {
            error_message = "Failed to create token";
            LOG_ERROR("User login failed: " + error_message);
            return std::nullopt;
        }

        // 获取创建的token信息
        auto created_token = token_dao_->get_token_by_value(token);
        if (!created_token) {
            error_message = "Failed to retrieve created token";
            LOG_ERROR("User login failed: " + error_message);
            return std::nullopt;
        }

        LOG_INFO("User logged in successfully: " + email);
        return created_token;
    } catch (const std::exception& e) {
        error_message = "Failed to login user: " + std::string(e.what());
        LOG_ERROR("User login failed: " + error_message);
        return std::nullopt;
    }
}

std::optional<User> UserService::get_user_info(int user_id, 
                                        std::string& error_message) {
    try {
        // 验证用户ID
        if (user_id <= 0) {
            error_message = "Invalid user ID";
            LOG_ERROR("Get user info failed: " + error_message);
            return std::nullopt;
        }

        // 根据用户ID获取用户信息
        auto user = user_dao_->get_user_by_id(user_id);
        if (!user) {
            error_message = "User not found";
            LOG_ERROR("Get user info failed: " + error_message);
            return std::nullopt;
        }

        LOG_INFO("Get user info successfully: " + std::to_string(user_id));
        return user;
    } catch (const std::exception& e) {
        error_message = "Failed to get user info: " + std::string(e.what());
        LOG_ERROR("Get user info failed: " + error_message);
        return std::nullopt;
    }
}

bool UserService::update_user_info(int user_id, const std::string& name, 
                           const std::string& phone, 
                           std::string& error_message) {
    try {
        // 验证用户ID
        if (user_id <= 0) {
            error_message = "Invalid user ID";
            LOG_ERROR("Update user info failed: " + error_message);
            return false;
        }

        // 根据用户ID获取用户信息
        auto user = user_dao_->get_user_by_id(user_id);
        if (!user) {
            error_message = "User not found";
            LOG_ERROR("Update user info failed: " + error_message);
            return false;
        }

        // 更新用户信息
        if (!name.empty()) {
            user->set_name(name);
        }
        if (!phone.empty()) {
            user->set_phone(phone);
        }

        // 保存更新后的用户信息
        if (!user_dao_->update_user(*user)) {
            error_message = "Failed to update user info";
            LOG_ERROR("Update user info failed: " + error_message);
            return false;
        }

        LOG_INFO("Update user info successfully: " + std::to_string(user_id));
        return true;
    } catch (const std::exception& e) {
        error_message = "Failed to update user info: " + std::string(e.what());
        LOG_ERROR("Update user info failed: " + error_message);
        return false;
    }
}

bool UserService::delete_user(int user_id, std::string& error_message) {
    try {
        // 验证用户ID
        if (user_id <= 0) {
            error_message = "Invalid user ID";
            LOG_ERROR("Delete user failed: " + error_message);
            return false;
        }

        // 根据用户ID获取用户信息
        auto user = user_dao_->get_user_by_id(user_id);
        if (!user) {
            error_message = "User not found";
            LOG_ERROR("Delete user failed: " + error_message);
            return false;
        }

        // 删除用户
        if (!user_dao_->delete_user(user_id)) {
            error_message = "Failed to delete user";
            LOG_ERROR("Delete user failed: " + error_message);
            return false;
        }

        LOG_INFO("Delete user successfully: " + std::to_string(user_id));
        return true;
    } catch (const std::exception& e) {
        error_message = "Failed to delete user: " + std::string(e.what());
        LOG_ERROR("Delete user failed: " + error_message);
        return false;
    }
}

std::optional<User> UserService::validate_token(const std::string& token, 
                                         std::string& error_message) {
    try {
        // 验证token是否为空
        if (token.empty()) {
            error_message = "Token cannot be empty";
            LOG_ERROR("Validate token failed: " + error_message);
            return std::nullopt;
        }

        // 根据token获取token信息
        auto token_obj = token_dao_->get_token_by_value(token);
        if (!token_obj) {
            error_message = "Invalid token";
            LOG_ERROR("Validate token failed: " + error_message);
            return std::nullopt;
        }

        // 检查token是否过期
        auto now = std::chrono::system_clock::now();
        auto token_expiry = std::chrono::system_clock::from_time_t(std::stoll(token_obj->get_expires_at()));
        if (now > token_expiry) {
            error_message = "Token expired";
            LOG_ERROR("Validate token failed: " + error_message);
            return std::nullopt;
        }

        // 根据用户ID获取用户信息
        auto user = user_dao_->get_user_by_id(token_obj->get_user_id());
        if (!user) {
            error_message = "User not found";
            LOG_ERROR("Validate token failed: " + error_message);
            return std::nullopt;
        }

        LOG_INFO("Validate token successfully: " + token);
        return user;
    } catch (const std::exception& e) {
        error_message = "Failed to validate token: " + std::string(e.what());
        LOG_ERROR("Validate token failed: " + error_message);
        return std::nullopt;
    }
}

std::string UserService::hash_password(const std::string& password) {
    // 简单的密码哈希函数（实际应用中应该使用更安全的哈希算法）
    std::string hash = "";
    for (char c : password) {
        hash += std::to_string(static_cast<int>(c));
    }
    return hash;
}

std::string UserService::generate_token() {
    // 生成随机token
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, chars.size() - 1);

    std::string token;
    for (int i = 0; i < 32; ++i) {
        token += chars[distribution(generator)];
    }

    return token;
}

bool UserService::is_email_exists(const std::string& email) {
    auto user = user_dao_->get_user_by_email(email);
    return user.has_value();
}

} // namespace pet_hospital
