#include "services/user_service.h"
#include <cpprest/json.h>
#include <stdexcept>
#include <regex>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace services {

using namespace web;

UserService::UserService(std::shared_ptr<repositories::UserRepository> user_repository)
    : user_repository_(std::move(user_repository)) {
    if (!user_repository_) {
        throw std::invalid_argument("user_repository cannot be null");
    }
}

UserService::~UserService() {
}

std::optional<models::User> UserService::create_user(const std::string& name, const std::string& email, 
                                                     const std::string& department, models::User::Role role,
                                                     const std::string& password) {
    if (name.empty()) {
        throw std::invalid_argument("name cannot be empty");
    }
    
    if (email.empty() || !validate_email(email)) {
        throw std::invalid_argument("invalid email format");
    }
    
    if (department.empty()) {
        throw std::invalid_argument("department cannot be empty");
    }
    
    if (!validate_password_strength(password)) {
        throw std::invalid_argument("password is too weak");
    }
    
    // 检查邮箱是否已存在
    if (user_repository_->find_by_email(email)) {
        throw std::runtime_error("email already exists");
    }
    
    std::string password_hash = hash_password(password);
    std::time_t now = std::time(nullptr);
    
    models::User user(0, name, email, department, role, password_hash, now, now, models::User::Status::ACTIVE);
    
    int user_id = user_repository_->create(user);
    if (user_id <= 0) {
        return std::nullopt;
    }
    
    return user_repository_->find_by_id(user_id);
}

std::optional<models::User> UserService::get_user_by_id(int user_id) {
    if (user_id <= 0) {
        throw std::invalid_argument("user_id must be positive");
    }
    
    return user_repository_->find_by_id(user_id);
}

std::optional<models::User> UserService::get_user_by_email(const std::string& email) {
    if (email.empty()) {
        throw std::invalid_argument("email cannot be empty");
    }
    
    return user_repository_->find_by_email(email);
}

std::vector<models::User> UserService::get_users_by_department(const std::string& department) {
    if (department.empty()) {
        throw std::invalid_argument("department cannot be empty");
    }
    
    return user_repository_->find_by_department(department);
}

std::vector<models::User> UserService::get_all_users() {
    return user_repository_->find_all();
}

bool UserService::update_user(int user_id, const std::optional<std::string>& name,
                              const std::optional<std::string>& email,
                              const std::optional<std::string>& department,
                              const std::optional<models::User::Role>& role,
                              const std::optional<models::User::Status>& status) {
    if (user_id <= 0) {
        throw std::invalid_argument("user_id must be positive");
    }
    
    auto existing_user = user_repository_->find_by_id(user_id);
    if (!existing_user) {
        return false;
    }
    
    // 检查邮箱是否已被其他用户使用
    if (email && *email != existing_user->get_email()) {
        if (user_repository_->find_by_email(*email)) {
            throw std::runtime_error("email already exists");
        }
    }
    
    models::User updated_user = *existing_user;
    
    if (name) updated_user.set_name(*name);
    if (email) updated_user.set_email(*email);
    if (department) updated_user.set_department(*department);
    if (role) updated_user.set_role(*role);
    if (status) updated_user.set_status(*status);
    
    updated_user.set_updated_at(std::time(nullptr));
    
    return user_repository_->update(updated_user);
}

bool UserService::update_user_password(int user_id, const std::string& new_password) {
    if (user_id <= 0) {
        throw std::invalid_argument("user_id must be positive");
    }
    
    if (!validate_password_strength(new_password)) {
        throw std::invalid_argument("password is too weak");
    }
    
    auto existing_user = user_repository_->find_by_id(user_id);
    if (!existing_user) {
        return false;
    }
    
    std::string password_hash = hash_password(new_password);
    
    models::User updated_user = *existing_user;
    updated_user.set_password_hash(password_hash);
    updated_user.set_updated_at(std::time(nullptr));
    
    return user_repository_->update(updated_user);
}

bool UserService::delete_user(int user_id) {
    if (user_id <= 0) {
        throw std::invalid_argument("user_id must be positive");
    }
    
    return user_repository_->delete_by_id(user_id);
}

bool UserService::verify_password(int user_id, const std::string& password) const {
    if (user_id <= 0) {
        throw std::invalid_argument("user_id must be positive");
    }
    
    if (password.empty()) {
        return false;
    }
    
    auto user = user_repository_->find_by_id(user_id);
    if (!user) {
        return false;
    }
    
    const std::string& stored_hash = user->get_password_hash();
    
    // 从存储的哈希中提取盐
    if (stored_hash.size() < 16 + 64) { // salt(16 bytes) + hash(64 bytes in hex) = 80 characters
        return false;
    }
    
    std::string salt_hex = stored_hash.substr(0, 16);
    std::string target_hash = stored_hash.substr(16);
    
    // 将盐的十六进制字符串转换为二进制
    std::string salt(salt_hex.size() / 2, 0);
    for (size_t i = 0; i < salt.size(); ++i) {
        int x;
        std::istringstream iss(salt_hex.substr(i*2, 2));
        iss >> std::hex >> x;
        salt[i] = static_cast<char>(x);
    }
    
    // 计算密码哈希
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    
    if (!PKCS5_PBKDF2_HMAC(password.c_str(), password.size(),
                           reinterpret_cast<const unsigned char*>(salt.c_str()), salt.size(),
                           100000, EVP_sha256(), EVP_MAX_MD_SIZE, hash, &hash_len)) {
        return false;
    }
    
    // 将哈希转换为十六进制字符串
    std::ostringstream oss;
    for (unsigned int i = 0; i < hash_len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    std::string computed_hash = oss.str();
    
    return computed_hash == target_hash;
}

bool UserService::validate_email(const std::string& email) {
    static const std::regex email_regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return std::regex_match(email, email_regex);
}

bool UserService::validate_password_strength(const std::string& password) {
    // 密码至少8个字符，包含大小写字母和数字
    if (password.size() < 8) {
        return false;
    }
    
    bool has_lower = false, has_upper = false, has_digit = false;
    
    for (char c : password) {
        if (islower(c)) has_lower = true;
        else if (isupper(c)) has_upper = true;
        else if (isdigit(c)) has_digit = true;
        
        if (has_lower && has_upper && has_digit) {
            return true;
        }
    }
    
    return false;
}

std::string UserService::hash_password(const std::string& password) const {
    // 生成随机盐（16字节）
    unsigned char salt[16];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        throw std::runtime_error("failed to generate salt");
    }
    
    // 计算PBKDF2哈希
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    
    if (!PKCS5_PBKDF2_HMAC(password.c_str(), password.size(),
                           salt, sizeof(salt), 100000, EVP_sha256(), EVP_MAX_MD_SIZE, hash, &hash_len)) {
        throw std::runtime_error("failed to hash password");
    }
    
    // 将盐和哈希转换为十六进制字符串存储
    std::ostringstream oss;
    
    // 存储盐（16字节，32个十六进制字符）
    for (int i = 0; i < sizeof(salt); ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(salt[i]);
    }
    
    // 存储哈希
    for (unsigned int i = 0; i < hash_len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return oss.str();
}

} // namespace services
