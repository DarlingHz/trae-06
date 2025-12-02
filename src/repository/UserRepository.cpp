#include "repository/UserRepository.h"
#include "model/User.h"
#include <sqlite3.h>
#include <string>
#include <optional>
#include <chrono>

namespace repository {

void UserRepository::createTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            created_at INTEGER NOT NULL DEFAULT (strftime('%s', 'now'))
        );
    )";
    execute(sql);
}

model::User UserRepository::createUser(const std::string& username, const std::string& password_hash) {
    std::string sql = R"(
        INSERT INTO users (username, password_hash) VALUES (?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数
    rc = sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind username: " + std::string(sqlite3_errmsg(db_)));
    }

    rc = sqlite3_bind_text(stmt, 2, password_hash.c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind password_hash: " + std::string(sqlite3_errmsg(db_)));
    }

    // 执行 SQL 语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 获取插入的 ID
    int user_id = sqlite3_last_insert_rowid(db_);

    // 释放资源
    sqlite3_finalize(stmt);

    // 获取当前时间
    auto created_at = std::chrono::system_clock::now();

    // 返回创建的用户对象
    return model::User(user_id, username, password_hash, created_at);
}

std::optional<model::User> UserRepository::findUserByUsername(const std::string& username) {
    std::string sql = R"(
        SELECT id, username, password_hash, created_at FROM users WHERE username = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数
    rc = sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind username: " + std::string(sqlite3_errmsg(db_)));
    }

    // 执行 SQL 查询
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        // 获取查询结果
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* username_ptr = sqlite3_column_text(stmt, 1);
        const unsigned char* password_hash_ptr = sqlite3_column_text(stmt, 2);
        long long created_at_seconds = sqlite3_column_int64(stmt, 3);

        // 转换数据类型
        std::string username_str(reinterpret_cast<const char*>(username_ptr));
        std::string password_hash_str(reinterpret_cast<const char*>(password_hash_ptr));
        auto created_at = std::chrono::system_clock::from_time_t(created_at_seconds);

        // 释放资源
        sqlite3_finalize(stmt);

        // 返回用户对象
        return model::User(id, username_str, password_hash_str, created_at);
    } else if (rc == SQLITE_DONE) {
        // 没有找到用户
        sqlite3_finalize(stmt);
        return std::nullopt;
    } else {
        // 查询失败
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute SQL query: " + std::string(sqlite3_errmsg(db_)));
    }
}

std::optional<model::User> UserRepository::findUserById(int user_id) {
    std::string sql = R"(
        SELECT id, username, password_hash, created_at FROM users WHERE id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数
    rc = sqlite3_bind_int(stmt, 1, user_id);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind user_id: " + std::string(sqlite3_errmsg(db_)));
    }

    // 执行 SQL 查询
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        // 获取查询结果
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* username_ptr = sqlite3_column_text(stmt, 1);
        const unsigned char* password_hash_ptr = sqlite3_column_text(stmt, 2);
        long long created_at_seconds = sqlite3_column_int64(stmt, 3);

        // 转换数据类型
        std::string username_str(reinterpret_cast<const char*>(username_ptr));
        std::string password_hash_str(reinterpret_cast<const char*>(password_hash_ptr));
        auto created_at = std::chrono::system_clock::from_time_t(created_at_seconds);

        // 释放资源
        sqlite3_finalize(stmt);

        // 返回用户对象
        return model::User(id, username_str, password_hash_str, created_at);
    } else if (rc == SQLITE_DONE) {
        // 没有找到用户
        sqlite3_finalize(stmt);
        return std::nullopt;
    } else {
        // 查询失败
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute SQL query: " + std::string(sqlite3_errmsg(db_)));
    }
}

} // namespace repository