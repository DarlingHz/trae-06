#include "UserService.h"
#include "../dao/UserDAO.h"
#include "../util/Logger.h"
#include "../util/Config.h"
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <jwt-cpp/jwt.h>
#include <stdexcept>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>

// SHA-256 哈希函数（使用 OpenSSL 3.0 EVP API）
std::string sha256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
    
    if (md_ctx == nullptr) {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }
    
    if (EVP_DigestInit_ex(md_ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(md_ctx);
        throw std::runtime_error("Failed to initialize EVP_DigestInit_ex");
    }
    
    if (EVP_DigestUpdate(md_ctx, input.c_str(), input.length()) != 1) {
        EVP_MD_CTX_free(md_ctx);
        throw std::runtime_error("Failed to update EVP_DigestUpdate");
    }
    
    unsigned int hash_length;
    if (EVP_DigestFinal_ex(md_ctx, hash, &hash_length) != 1) {
        EVP_MD_CTX_free(md_ctx);
        throw std::runtime_error("Failed to finalize EVP_DigestFinal_ex");
    }
    
    EVP_MD_CTX_free(md_ctx);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return ss.str();
}

bool UserService::registerUser(const User& user, const std::string& password) {
    try {
        // 检查用户名是否存在
        if (checkUsernameExists(user.getUsername())) {
            Logger::error("Username already exists: " + user.getUsername());
            return false;
        }
        
        // 检查邮箱是否存在
        if (checkEmailExists(user.getEmail())) {
            Logger::error("Email already exists: " + user.getEmail());
            return false;
        }
        
        // 加密密码
        std::string hashed_password = sha256(password);
        
        // 创建UserDAO对象
        UserDAO user_dao;
        
        // 注册用户
        User new_user = user;
        new_user.setPasswordHash(hashed_password);
        bool result = user_dao.registerUser(new_user);
        
        if (result) {
            Logger::info("User registered successfully: " + user.getUsername());
        } else {
            Logger::error("Failed to register user: " + user.getUsername());
        }
        
        return result;
    } catch (const std::exception& e) {
        Logger::error("Failed to register user: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<User> UserService::login(const std::string& username, const std::string& password) {
    try {
        // 创建UserDAO对象
        UserDAO user_dao;
        
        // 根据用户名查询用户
        std::shared_ptr<User> user = user_dao.getUserByUsername(username);
        if (!user) {
            Logger::error("User not found: " + username);
            return nullptr;
        }
        
        // 检查用户状态
        if (user->getStatus() != "active") {
            Logger::error("User is not active: " + username);
            return nullptr;
        }
        
        // 验证密码
        std::string hashed_password = sha256(password);
        if (hashed_password != user->getPasswordHash()) {
            Logger::error("Invalid password for user: " + username);
            return nullptr;
        }
        
        Logger::info("User logged in successfully: " + username);
        return user;
    } catch (const std::exception& e) {
        Logger::error("Failed to login user: " + std::string(e.what()));
        return nullptr;
    }
}

bool UserService::updateUserInfo(const User& user) {
    try {
        // 创建UserDAO对象
        UserDAO user_dao;
        
        // 更新用户信息
        bool result = user_dao.updateUser(user);
        
        if (result) {
            Logger::info("User info updated successfully: " + user.getUsername());
        } else {
            Logger::error("Failed to update user info: " + user.getUsername());
        }
        
        return result;
    } catch (const std::exception& e) {
        Logger::error("Failed to update user info: " + std::string(e.what()));
        return false;
    }
}

bool UserService::updateUserPassword(int user_id, const std::string& old_password, const std::string& new_password) {
    try {
        // 创建UserDAO对象
        UserDAO user_dao;
        
        // 根据用户ID查询用户
        std::shared_ptr<User> user = user_dao.getUserById(user_id);
        if (!user) {
            Logger::error("User not found: " + std::to_string(user_id));
            return false;
        }
        
        // 验证旧密码
        std::string hashed_old_password = sha256(old_password);
        if (hashed_old_password != user->getPasswordHash()) {
            Logger::error("Invalid old password for user: " + std::to_string(user_id));
            return false;
        }
        
        // 加密新密码
        std::string hashed_password = sha256(new_password);
        
        // 更新用户密码
        bool result = user_dao.updateUserPassword(user_id, hashed_password);
        
        if (result) {
            Logger::info("User password updated successfully: " + std::to_string(user_id));
        } else {
            Logger::error("Failed to update user password: " + std::to_string(user_id));
        }
        
        return result;
    } catch (const std::exception& e) {
        Logger::error("Failed to update user password: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<User> UserService::getUserInfo(int user_id) {
    try {
        // 创建UserDAO对象
        UserDAO user_dao;
        
        // 根据用户ID查询用户
        std::shared_ptr<User> user = user_dao.getUserById(user_id);
        if (!user) {
            Logger::error("User not found: " + std::to_string(user_id));
            return nullptr;
        }
        
        Logger::info("User info retrieved successfully: " + std::to_string(user_id));
        return user;
    } catch (const std::exception& e) {
        Logger::error("Failed to get user info: " + std::string(e.what()));
        return nullptr;
    }
}

std::vector<std::shared_ptr<User>> UserService::getAllUsers(int page, int page_size) {
    try {
        // 创建UserDAO对象
        UserDAO user_dao;
        
        // 获取所有用户
        std::vector<std::shared_ptr<User>> users = user_dao.getAllUsers(page, page_size);
        
        Logger::info("All users retrieved successfully, page: " + std::to_string(page) + ", page size: " + std::to_string(page_size));
        return users;
    } catch (const std::exception& e) {
        Logger::error("Failed to get all users: " + std::string(e.what()));
        return std::vector<std::shared_ptr<User>>();
    }
}

int UserService::getUserCount() {
    try {
        // 创建UserDAO对象
        UserDAO user_dao;
        
        // 获取用户总数
        int count = user_dao.getUserCount();
        
        Logger::info("User count retrieved successfully: " + std::to_string(count));
        return count;
    } catch (const std::exception& e) {
        Logger::error("Failed to get user count: " + std::string(e.what()));
        return 0;
    }
}

bool UserService::toggleUserStatus(int user_id, const std::string& status) {
    try {
        // 创建UserDAO对象
        UserDAO user_dao;
        
        // 切换用户状态
        bool result = user_dao.toggleUserStatus(user_id, status);
        
        if (result) {
            Logger::info("User status toggled successfully: " + std::to_string(user_id) + ", status: " + status);
        } else {
            Logger::error("Failed to toggle user status: " + std::to_string(user_id) + ", status: " + status);
        }
        
        return result;
    } catch (const std::exception& e) {
        Logger::error("Failed to toggle user status: " + std::string(e.what()));
        return false;
    }
}

bool UserService::checkUsernameExists(const std::string& username) {
    try {
        // 创建UserDAO对象
        UserDAO user_dao;
        
        // 根据用户名查询用户
        std::shared_ptr<User> user = user_dao.getUserByUsername(username);
        
        return user != nullptr;
    } catch (const std::exception& e) {
        Logger::error("Failed to check username exists: " + std::string(e.what()));
        return false;
    }
}

bool UserService::checkEmailExists(const std::string& email) {
    try {
        // 创建UserDAO对象
        UserDAO user_dao;
        
        // 根据邮箱查询用户
        std::shared_ptr<User> user = user_dao.getUserByEmail(email);
        
        return user != nullptr;
    } catch (const std::exception& e) {
        Logger::error("Failed to check email exists: " + std::string(e.what()));
        return false;
    }
}

std::string UserService::generateJWTToken(const User& user) {
    try {
        // 获取JWT配置
        std::string secret_key = Config::getJwtSecret();
        int expires_in = Config::getJwtExpiresIn();
        
        // 创建JWT Token
        auto token = jwt::create()
            .set_issuer("library_service")
            .set_subject(std::to_string(user.getId()))
            .set_audience("library_users")
            .set_issued_at(std::chrono::system_clock::now())
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(expires_in))
            .set_payload_claim("username", jwt::claim(user.getUsername()))
            .set_payload_claim("role", jwt::claim(user.getRole()))
            .sign(jwt::algorithm::hs256{secret_key});
        
        Logger::info("JWT Token generated successfully for user: " + user.getUsername());
        return token;
    } catch (const std::exception& e) {
        Logger::error("Failed to generate JWT Token: " + std::string(e.what()));
        return "";
    }
}

int UserService::verifyJWTToken(const std::string& token) {
    try {
        // 获取JWT配置
        std::string secret_key = Config::getJwtSecret();
        
        // 验证JWT Token
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret_key})
            .with_issuer("library_service")
            .with_audience("library_users");
        
        verifier.verify(decoded);
        
        // 获取用户ID
        int user_id = std::stoi(decoded.get_subject());
        
        Logger::info("JWT Token verified successfully for user: " + std::to_string(user_id));
        return user_id;
    } catch (const std::exception& e) {
        Logger::error("Failed to verify JWT Token: " + std::string(e.what()));
        return -1;
    }
}