#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include "../models/user.h"
#include "../repositories/user_repository.h"
#include "../utils/logger.h"

class UserService {
public:
    std::shared_ptr<User> createUser(const std::string& name, const std::string& email) {
        // 验证邮箱格式
        if (email.find('@') == std::string::npos) {
            throw std::runtime_error("Invalid email format");
        }

        User user;
        user.name = name;
        user.email = email;

        try {
            return UserRepository::getInstance().create(user);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create user: %s", e.what());
            throw;
        }
    }

    std::shared_ptr<User> getUserById(int id) {
        if (id <= 0) {
            throw std::runtime_error("Invalid user ID");
        }

        try {
            auto user = UserRepository::getInstance().findById(id);
            if (!user) {
                throw std::runtime_error("User not found");
            }
            return user;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get user by ID: %s", e.what());
            throw;
        }
    }

    std::shared_ptr<User> getUserByEmail(const std::string& email) {
        if (email.empty()) {
            throw std::runtime_error("Invalid email");
        }

        try {
            return UserRepository::getInstance().findByEmail(email);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get user by email: %s", e.what());
            throw;
        }
    }

    bool userExists(int id) {
        if (id <= 0) {
            return false;
        }

        try {
            return UserRepository::getInstance().findById(id) != nullptr;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to check user existence: %s", e.what());
            return false;
        }
    }

private:
    UserService(const UserService&) = delete;
    UserService& operator=(const UserService&) = delete;

public:
    static UserService& getInstance() {
        static UserService instance;
        return instance;
    }
};
