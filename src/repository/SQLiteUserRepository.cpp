#include "SQLiteUserRepository.h"
#include <sqlite3.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace repository {

// 构造函数
SQLiteUserRepository::SQLiteUserRepository(const std::string& db_path)
    : SQLiteBaseRepository<model::User>(db_path) {
    if (!initialize()) {
        throw ::std::runtime_error("Failed to initialize SQLiteUserRepository");
    }
}

// 从SQLiteBaseRepository继承的纯虚方法的实现
std::string SQLiteUserRepository::getTableName() const {
    return "users";
}

std::string SQLiteUserRepository::getCreateTableSql() const {
    return R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            password_hash TEXT NOT NULL,
            created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
        );
    )";
}

std::string SQLiteUserRepository::getInsertSql() const {
    return R"(
        INSERT INTO users (username, password_hash, created_at)
        VALUES (?, ?, ?);
    )";
}

std::string SQLiteUserRepository::getSelectByIdSql() const {
    return R"(
        SELECT id, username, password_hash, created_at
        FROM users
        WHERE id = ?;
    )";
}

std::string SQLiteUserRepository::getSelectAllSql() const {
    return R"(
        SELECT id, username, password_hash, created_at
        FROM users;
    )";
}

std::string SQLiteUserRepository::getUpdateSql() const {
    return R"(
        UPDATE users
        SET username = ?, password_hash = ?
        WHERE id = ?;
    )";
}

std::string SQLiteUserRepository::getDeleteByIdSql() const {
    return R"(
        DELETE FROM users
        WHERE id = ?;
    )";
}

model::User SQLiteUserRepository::fromRow(sqlite3_stmt* stmt) const {
    // 提取字段值
    int id = sqlite3_column_int(stmt, 0);
    const char* username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    const char* password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

    // 转换时间戳
    std::tm tm = {};
    std::istringstream ss(created_at_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto created_at = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    // 创建并返回User对象
    return model::User(id, username, password_hash, created_at);
}

void SQLiteUserRepository::bindValues(sqlite3_stmt* stmt, const model::User& entity) const {
    // 绑定参数
    sqlite3_bind_text(stmt, 1, entity.getUsername().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, entity.getPasswordHash().c_str(), -1, SQLITE_TRANSIENT);

    // 转换时间戳为字符串
    auto created_at_time_t = std::chrono::system_clock::to_time_t(entity.getCreatedAt());
    std::tm tm = {};
    localtime_r(&created_at_time_t, &tm);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::string created_at_str = ss.str();

    sqlite3_bind_text(stmt, 3, created_at_str.c_str(), -1, SQLITE_TRANSIENT);

    // 如果是更新操作，还需要绑定id
    if (getUpdateSql() == sqlite3_sql(stmt)) {
        sqlite3_bind_int(stmt, 4, entity.getId());
    }
}

// 从UserRepository接口继承的方法的实现
std::optional<model::User> SQLiteUserRepository::findByUsername(const std::string& username) {
    std::optional<model::User> result;
    std::string sql = R"(
        SELECT id, username, password_hash, created_at
        FROM users
        WHERE username = ?;
    )";

    executePreparedStatement(sql, [&username](sqlite3_stmt* stmt) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    }, [this, &result](sqlite3_stmt* stmt) {
        result = fromRow(stmt);
    });

    return result;
}

bool SQLiteUserRepository::existsByUsername(const std::string& username) {
    std::string sql = R"(
        SELECT EXISTS(SELECT 1 FROM users WHERE username = ?);
    )";

    bool exists = false;
    executePreparedStatement(sql, [&username](sqlite3_stmt* stmt) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    }, [&exists](sqlite3_stmt* stmt) {
        exists = sqlite3_column_int(stmt, 0) == 1;
    });

    return exists;
}

} // namespace repository