#include "chat_archive/service/UserService.h"
#include "chat_archive/Logger.h"
#include <regex>

namespace chat_archive {
namespace service {

std::optional<int64_t> UserService::create_user(const std::string& name) {
    // 验证用户名
    if (!validate_username(name)) {
        CHAT_ARCHIVE_LOG_WARN("Invalid username: {}", name);
        return std::nullopt;
    }
    
    // 检查用户是否已存在
    auto existing_user = user_dao_.get_user_by_name(name);
    if (existing_user) {
        CHAT_ARCHIVE_LOG_WARN("User already exists with name: {}", name);
        return std::nullopt;
    }
    
    // 创建用户
    auto user_id = user_dao_.create_user(name);
    if (!user_id) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to create user with name: {}", name);
        return std::nullopt;
    }
    
    CHAT_ARCHIVE_LOG_INFO("Successfully created user with ID: {}, name: {}", *user_id, name);
    return user_id;
}

std::optional<model::User> UserService::get_user_by_id(int64_t id) {
    if (id <= 0) {
        CHAT_ARCHIVE_LOG_WARN("Invalid user ID: {}", id);
        return std::nullopt;
    }
    
    auto user = user_dao_.get_user_by_id(id);
    if (!user) {
        CHAT_ARCHIVE_LOG_DEBUG("User not found with ID: {}", id);
        return std::nullopt;
    }
    
    return user;
}

std::optional<model::User> UserService::get_user_by_name(const std::string& name) {
    if (!validate_username(name)) {
        CHAT_ARCHIVE_LOG_WARN("Invalid username: {}", name);
        return std::nullopt;
    }
    
    auto user = user_dao_.get_user_by_name(name);
    if (!user) {
        CHAT_ARCHIVE_LOG_DEBUG("User not found with name: {}", name);
        return std::nullopt;
    }
    
    return user;
}

std::vector<model::User> UserService::get_users(int limit, int offset) {
    // 验证分页参数
    if (limit <= 0) {
        limit = 100;
    } else if (limit > 1000) {
        limit = 1000; // 限制最大返回数量
    }
    
    if (offset < 0) {
        offset = 0;
    }
    
    auto users = user_dao_.get_users(limit, offset);
    return users;
}

int64_t UserService::get_total_users() {
    return user_dao_.get_total_users();
}

bool UserService::validate_username(const std::string& name) {
    // 用户名长度检查
    if (name.empty() || name.length() > 50) {
        CHAT_ARCHIVE_LOG_WARN("Username length is invalid: {}", name);
        return false;
    }
    
    // 用户名格式检查（只允许字母、数字、下划线和连字符）
    std::regex username_regex("^[a-zA-Z0-9_-]+$");
    if (!std::regex_match(name, username_regex)) {
        CHAT_ARCHIVE_LOG_WARN("Username format is invalid: {}", name);
        return false;
    }
    
    return true;
}

} // namespace service
} // namespace chat_archive