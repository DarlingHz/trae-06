#include "chat_archive/dao/UserDAO.h"
#include "chat_archive/Logger.h"
#include <sstream>
#include <iomanip>

namespace chat_archive {
namespace dao {

std::optional<int64_t> UserDAO::create_user(const std::string& name) {
    const std::string sql = "INSERT INTO users (name) VALUES (?);";
    
    auto conn = get_connection();
    if (!conn) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to get database connection for creating user");
        return std::nullopt;
    }
    
    DatabaseQuery query(conn);
    if (!query.prepare(sql)) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to prepare create user query");
        release_connection(conn);
        return std::nullopt;
    }
    
    if (!query.bind_string(1, name)) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to bind name parameter for creating user");
        release_connection(conn);
        return std::nullopt;
    }
    
    if (!query.execute()) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to execute create user query");
        release_connection(conn);
        return std::nullopt;
    }
    
    int64_t user_id = query.get_last_insert_rowid();
    release_connection(conn);
    
    CHAT_ARCHIVE_LOG_INFO("Created user with ID: {}, name: {}", user_id, name);
    return user_id;
}

std::optional<model::User> UserDAO::get_user_by_id(int64_t id) {
    const std::string sql = "SELECT id, name, created_at FROM users WHERE id = ?;";
    
    auto result = execute_query(sql, {}, {}, {{1, id}});
    
    if (!result.next()) {
        CHAT_ARCHIVE_LOG_DEBUG("User not found with ID: {}", id);
        return std::nullopt;
    }
    
    model::User user = build_user_from_result(result);
    return user;
}

std::optional<model::User> UserDAO::get_user_by_name(const std::string& name) {
    const std::string sql = "SELECT id, name, created_at FROM users WHERE name = ?;";
    
    auto result = execute_query(sql, {{1, name}}, {}, {});
    
    if (!result.next()) {
        CHAT_ARCHIVE_LOG_DEBUG("User not found with name: {}", name);
        return std::nullopt;
    }
    
    model::User user = build_user_from_result(result);
    return user;
}

std::vector<model::User> UserDAO::get_users(int limit, int offset) {
    const std::string sql = "SELECT id, name, created_at FROM users ORDER BY created_at DESC LIMIT ? OFFSET ?;";
    
    auto result = execute_query(sql, {}, {{1, limit}, {2, offset}}, {});
    
    std::vector<model::User> users;
    while (result.next()) {
        users.push_back(build_user_from_result(result));
    }
    
    CHAT_ARCHIVE_LOG_DEBUG("Retrieved {} users from database", users.size());
    return users;
}

int64_t UserDAO::get_total_users() {
    const std::string sql = "SELECT COUNT(*) FROM users;";
    
    auto result = execute_query(sql);
    
    if (!result.next()) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to get total users count");
        return 0;
    }
    
    return result.get_int64(0);
}

model::User UserDAO::build_user_from_result(const DatabaseResult& result) {
    int64_t id = result.get_int64(0);
    std::string name = result.get_string(1);
    std::string created_at_str = result.get_string(2);
    
    // 解析时间字符串
    std::tm tm = {};
    std::istringstream iss(created_at_str);
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    auto created_at = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    
    return model::User(id, name, created_at);
}

} // namespace dao
} // namespace chat_archive