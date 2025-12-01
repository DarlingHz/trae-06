#include "dao/UserDao.h"
#include <iostream>
#include <sqlite3.h>

namespace dao {

UserDao::UserDao(sqlite3* db) : db_(db) {
}

UserDao::~UserDao() {
}

bool UserDao::createTable() const {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            email TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            nickname TEXT NOT NULL,
            timezone TEXT NOT NULL DEFAULT 'UTC',
            created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
        );
    )";

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to create users table: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool UserDao::insertUser(const model::User& user) const {
    const char* sql = R"(
        INSERT INTO users (email, password_hash, nickname, timezone, created_at)
        VALUES (?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare insert user statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, user.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.password_hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.nickname.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, user.timezone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, user.created_at.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert user: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

model::User UserDao::findUserByEmail(const std::string& email) const {
    model::User user;
    user.id = -1;

    const char* sql = R"(
        SELECT id, email, password_hash, nickname, timezone, created_at
        FROM users
        WHERE email = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare find user by email statement: " << sqlite3_errmsg(db_) << std::endl;
        return user;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        user.id = sqlite3_column_int(stmt, 0);
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.nickname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user.timezone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        user.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    }

    sqlite3_finalize(stmt);
    return user;
}

model::User UserDao::findUserById(int id) const {
    model::User user;
    user.id = -1;

    const char* sql = R"(
        SELECT id, email, password_hash, nickname, timezone, created_at
        FROM users
        WHERE id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare find user by id statement: " << sqlite3_errmsg(db_) << std::endl;
        return user;
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        user.id = sqlite3_column_int(stmt, 0);
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.nickname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user.timezone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        user.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    }

    sqlite3_finalize(stmt);
    return user;
}

} // namespace dao
