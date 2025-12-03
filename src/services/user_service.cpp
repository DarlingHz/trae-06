#include "services/user_service.h"
#include "repositories/user_repository.h"
#include "utils/hash.h"
#include "utils/jwt.h"
#include "utils/logger.h"
#include "utils/config.h"
#include <stdexcept>

UserService::UserService() {}

UserService::~UserService() {}

UserService& UserService::getInstance() {
    static UserService instance;
    return instance;
}

User UserService::registerUser(const std::string& username, const std::string& email, const std::string& password) {
    // 检查用户名是否存在
    if (UserRepository::getInstance().checkUsernameExists(username)) {
        throw std::runtime_error("Username already exists");
    }

    // 检查邮箱是否存在
    if (UserRepository::getInstance().checkEmailExists(email)) {
        throw std::runtime_error("Email already exists");
    }

    // 生成盐值
    std::string salt = Hash::generateSalt();
    
    // 对密码进行哈希
    std::string password_hash = Hash::hashPassword(password, salt);
    
    // 创建用户
    User user(username, email, password_hash, salt);
    
    auto created_user = UserRepository::getInstance().createUser(user);
    
    if (!created_user) {
        throw std::runtime_error("Failed to create user");
    }

    LOG_INFO("User registered: " << username << " (ID: " << created_user->getId() << ")");
    return *created_user;
}

std::string UserService::loginUser(const std::string& email, const std::string& password) {
    // 根据邮箱获取用户
    auto user = UserRepository::getInstance().getUserByEmail(email);
    
    if (!user) {
        LOG_WARNING("Login attempt failed: User not found with email: " << email);
        throw std::runtime_error("Invalid email or password");
    }

    // 验证密码
    if (!Hash::verifyPassword(password, user->getPasswordHash(), user->getSalt())) {
        LOG_WARNING("Login attempt failed: Invalid password for user: " << email);
        throw std::runtime_error("Invalid email or password");
    }

    // 生成JWT令牌
    std::string token = JWT::generateToken(user->getId(), user->getUsername());
    
    LOG_INFO("User logged in: " << user->getUsername() << " (ID: " << user->getId() << ")");
    return token;
}

User UserService::getUserById(int id) {
    auto user = UserRepository::getInstance().getUserById(id);
    
    if (!user) {
        throw std::runtime_error("User not found");
    }

    return *user;
}

User UserService::getUserByEmail(const std::string& email) {
    auto user = UserRepository::getInstance().getUserByEmail(email);
    
    if (!user) {
        throw std::runtime_error("User not found");
    }

    return *user;
}

bool UserService::verifyToken(const std::string& token) {
    return JWT::verifyToken(token);
}

int UserService::getUserIdFromToken(const std::string& token) {
    return JWT::getUserIdFromToken(token);
}
