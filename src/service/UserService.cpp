#include "UserService.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <chrono>
#include <functional>
#include <random>
#include <sstream>
#include <iomanip>

namespace service {

// 注册新用户
std::optional<model::User> UserService::registerUser(const std::string& username, const std::string& password) {
    try {
        // 业务逻辑验证
        if (username.empty()) {
            spdlog::error("Username cannot be empty");
            return std::nullopt;
        }

        if (password.empty()) {
            spdlog::error("Password cannot be empty");
            return std::nullopt;
        }

        if (username.length() < 3 || username.length() > 20) {
            spdlog::error("Username must be between 3 and 20 characters");
            return std::nullopt;
        }

        if (password.length() < 6 || password.length() > 50) {
            spdlog::error("Password must be between 6 and 50 characters");
            return std::nullopt;
        }

        // 检查用户名是否已存在
        if (user_repository_->existsByUsername(username)) {
            spdlog::error("Username already exists: {}", username);
            return std::nullopt;
        }

        // 哈希密码
        std::string password_hash = hashPassword(password);

        // 创建用户对象
        model::User user;
        user.setUsername(username);
        user.setPasswordHash(password_hash);
        user.setCreatedAt(std::chrono::system_clock::now());

        // 保存用户到数据库
        int user_id = user_repository_->create(user);
        if (user_id <= 0) {
            spdlog::error("Failed to create user: {}", username);
            return std::nullopt;
        }

        // 设置用户ID
        user.setId(user_id);

        spdlog::info("User registered successfully: {}", username);
        return user;
    } catch (const std::exception& e) {
        spdlog::error("Error registering user: {}", e.what());
        return std::nullopt;
    }
}

// 验证用户密码
std::optional<model::User> UserService::authenticateUser(const std::string& username, const std::string& password) {
    try {
        // 业务逻辑验证
        if (username.empty() || password.empty()) {
            spdlog::error("Username or password cannot be empty");
            return std::nullopt;
        }

        // 查找用户
        std::optional<model::User> user = user_repository_->findByUsername(username);
        if (!user) {
            spdlog::error("User not found: {}", username);
            return std::nullopt;
        }

        // 验证密码哈希
        if (!verifyPassword(password, user->getPasswordHash())) {
            spdlog::error("Invalid password for user: {}", username);
            return std::nullopt;
        }

        spdlog::info("User authenticated successfully: {}", username);
        return user;
    } catch (const std::exception& e) {
        spdlog::error("Error authenticating user: {}", e.what());
        return std::nullopt;
    }
}

// 根据ID查找用户
std::optional<model::User> UserService::findUserById(int id) {
    try {
        if (id <= 0) {
            spdlog::error("Invalid user ID: {}", id);
            return std::nullopt;
        }

        std::optional<model::User> user = user_repository_->findById(id);
        if (!user) {
            spdlog::error("User not found: ID = {}", id);
            return std::nullopt;
        }

        return user;
    } catch (const std::exception& e) {
        spdlog::error("Error finding user by ID: {}", e.what());
        return std::nullopt;
    }
}

// 根据用户名查找用户
std::optional<model::User> UserService::findUserByUsername(const std::string& username) {
    try {
        if (username.empty()) {
            spdlog::error("Username cannot be empty");
            return std::nullopt;
        }

        std::optional<model::User> user = user_repository_->findByUsername(username);
        if (!user) {
            spdlog::error("User not found: {}", username);
            return std::nullopt;
        }

        return user;
    } catch (const std::exception& e) {
        spdlog::error("Error finding user by username: {}", e.what());
        return std::nullopt;
    }
}

// 检查用户名是否存在
bool UserService::existsByUsername(const std::string& username) {
    try {
        if (username.empty()) {
            spdlog::error("Username cannot be empty");
            return false;
        }

        return user_repository_->existsByUsername(username);
    } catch (const std::exception& e) {
        spdlog::error("Error checking if username exists: {}", e.what());
        return false;
    }
}

// 密码哈希函数
std::string UserService::hashPassword(const std::string& password) {
    // 为了简单起见，使用C++标准库中的hash函数
    // 在实际生产环境中，应该使用更安全的哈希算法，如SHA-256
    std::hash<std::string> hash_fn;
    size_t hash_value = hash_fn(password);

    // 将哈希值转换为十六进制字符串
    std::ostringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash_value;
    return ss.str();
}

// 验证密码哈希
bool UserService::verifyPassword(const std::string& password, const std::string& password_hash) {
    // 计算输入密码的哈希值
    std::string input_hash = hashPassword(password);

    // 比较两个哈希值
    return input_hash == password_hash;
}

} // namespace service