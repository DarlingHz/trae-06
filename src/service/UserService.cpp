#include "service/UserService.h"
#include <stdexcept>
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

namespace service {

UserService::UserService(repository::UserRepository& user_repository)
    : user_repository_(user_repository) {
}

model::User UserService::registerUser(const std::string& username, const std::string& password) {
    // 验证参数
    if (username.empty()) {
        throw ServiceException("Username cannot be empty");
    }
    if (password.empty()) {
        throw ServiceException("Password cannot be empty");
    }

    // 检查用户名是否已存在
    auto existing_user = user_repository_.findUserByUsername(username);
    if (existing_user) {
        throw ServiceException("Username already exists");
    }

    // 对密码进行哈希
    std::string hashed_password = hashPassword(password);

    // 创建用户
    model::User user;
    user.set_username(username);
    user.set_password_hash(hashed_password);

    try {
        return user_repository_.createUser(user.username(), user.password_hash());
    } catch (const repository::DatabaseException& e) {
        throw ServiceException("Failed to create user: " + std::string(e.what()));
    }
}

std::string UserService::loginUser(const std::string& username, const std::string& password) {
    // 验证参数
    if (username.empty()) {
        throw ServiceException("Username cannot be empty");
    }
    if (password.empty()) {
        throw ServiceException("Password cannot be empty");
    }

    // 查找用户
    auto user = user_repository_.findUserByUsername(username);
    if (!user) {
        throw ServiceException("Invalid username or password");
    }

    // 验证密码
    if (!verifyPassword(password, user->password_hash())) {
        throw ServiceException("Invalid username or password");
    }

    // 生成token
    std::string token = generateToken();

    // 保存token到用户ID的映射
    { // 加锁保护token_user_map_
        std::lock_guard<std::mutex> lock(token_map_mutex_);
        token_user_map_[token] = user->id();
    }

    return token;
}

std::optional<int> UserService::validateToken(const std::string& token) {
    if (token.empty()) {
        return std::nullopt;
    }

    { // 加锁保护token_user_map_
        std::lock_guard<std::mutex> lock(token_map_mutex_);
        auto it = token_user_map_.find(token);
        if (it != token_user_map_.end()) {
            return it->second;
        }
    }

    return std::nullopt;
}

void UserService::logoutUser(const std::string& token) {
    if (token.empty()) {
        return;
    }

    { // 加锁保护token_user_map_
        std::lock_guard<std::mutex> lock(token_map_mutex_);
        token_user_map_.erase(token);
    }
}

model::User UserService::getUserById(int user_id) {
    auto user = user_repository_.findUserById(user_id);
    if (!user) {
        throw ServiceException("User not found");
    }
    return *user;
}

std::string UserService::generateToken() {
    // 使用随机数生成器生成token
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 255);

    std::string token;
    token.reserve(32);

    for (int i = 0; i < 32; ++i) {
        token += static_cast<char>(dis(gen));
    }

    // 将token转换为十六进制字符串
    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (char c : token) {
        ss << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
    }

    return ss.str();
}

std::string UserService::hashPassword(const std::string& password) {
    // 使用SHA-256对密码进行哈希
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.size());
    SHA256_Final(hash, &sha256);

    // 将哈希结果转换为十六进制字符串
    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::setw(2) << static_cast<int>(hash[i]);
    }

    return ss.str();
}

bool UserService::verifyPassword(const std::string& password, const std::string& hashed_password) {
    // 对输入的密码进行哈希，然后与存储的哈希值进行比较
    std::string input_hash = hashPassword(password);
    return input_hash == hashed_password;
}

} // namespace service