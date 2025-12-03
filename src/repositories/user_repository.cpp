#include "repositories/user_repository.h"
#include "utils/db_connection_pool.h"
#include "utils/logger.h"
#include <sqlite3.h>
#include <stdexcept>

UserRepository::UserRepository() {}

UserRepository::~UserRepository() {}

UserRepository& UserRepository::getInstance() {
    static UserRepository instance;
    return instance;
}

std::optional<User> UserRepository::getUserById(int id) {
    sqlite3* db = DBConnectionPool::getInstance().getConnection();
    if (!db) {
        LOG_ERROR("Failed to get database connection");
        return std::nullopt;
    }

    const char* query = "SELECT id, username, email, password_hash, salt, created_at, question_count, answer_count FROM users WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: " << sqlite3_errmsg(db));
        DBConnectionPool::getInstance().releaseConnection(db);
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, id);
    
    User user;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = User(
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)),
            sqlite3_column_int(stmt, 6),
            sqlite3_column_int(stmt, 7)
        );
        sqlite3_finalize(stmt);
        DBConnectionPool::getInstance().releaseConnection(db);
        return user;
    }

    sqlite3_finalize(stmt);
    DBConnectionPool::getInstance().releaseConnection(db);
    return std::nullopt;
}

std::optional<User> UserRepository::getUserByEmail(const std::string& email) {
    sqlite3* db = DBConnectionPool::getInstance().getConnection();
    if (!db) {
        LOG_ERROR("Failed to get database connection");
        return std::nullopt;
    }

    const char* query = "SELECT id, username, email, password_hash, salt, created_at, question_count, answer_count FROM users WHERE email = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: " << sqlite3_errmsg(db));
        DBConnectionPool::getInstance().releaseConnection(db);
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
    
    User user;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = User(
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)),
            sqlite3_column_int(stmt, 6),
            sqlite3_column_int(stmt, 7)
        );
        sqlite3_finalize(stmt);
        DBConnectionPool::getInstance().releaseConnection(db);
        return user;
    }

    sqlite3_finalize(stmt);
    DBConnectionPool::getInstance().releaseConnection(db);
    return std::nullopt;
}

bool UserRepository::checkUsernameExists(const std::string& username) {
    sqlite3* db = DBConnectionPool::getInstance().getConnection();
    if (!db) {
        LOG_ERROR("Failed to get database connection");
        return false;
    }

    const char* query = "SELECT id FROM users WHERE username = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: " << sqlite3_errmsg(db));
        DBConnectionPool::getInstance().releaseConnection(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    DBConnectionPool::getInstance().releaseConnection(db);
    return exists;
}

bool UserRepository::checkEmailExists(const std::string& email) {
    sqlite3* db = DBConnectionPool::getInstance().getConnection();
    if (!db) {
        LOG_ERROR("Failed to get database connection");
        return false;
    }

    const char* query = "SELECT id FROM users WHERE email = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: " << sqlite3_errmsg(db));
        DBConnectionPool::getInstance().releaseConnection(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
    
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    DBConnectionPool::getInstance().releaseConnection(db);
    return exists;
}

std::optional<User> UserRepository::createUser(const User& user) {
    sqlite3* db = DBConnectionPool::getInstance().getConnection();
    if (!db) {
        LOG_ERROR("Failed to get database connection");
        return std::nullopt;
    }

    const char* query = "INSERT INTO users (username, email, password_hash, salt, created_at) VALUES (?, ?, ?, ?, datetime('now'))";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: " << sqlite3_errmsg(db));
        DBConnectionPool::getInstance().releaseConnection(db);
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, user.getUsername().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user.getEmail().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, user.getPasswordHash().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, user.getSalt().c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        LOG_ERROR("Failed to insert user: " << sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        DBConnectionPool::getInstance().releaseConnection(db);
        return std::nullopt;
    }

    int new_id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    DBConnectionPool::getInstance().releaseConnection(db);
    
    return getUserById(new_id);
}

bool UserRepository::updateQuestionCount(int user_id, int count) {
    sqlite3* db = DBConnectionPool::getInstance().getConnection();
    if (!db) {
        LOG_ERROR("Failed to get database connection");
        return false;
    }

    const char* query = "UPDATE users SET question_count = ? WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: " << sqlite3_errmsg(db));
        DBConnectionPool::getInstance().releaseConnection(db);
        return false;
    }

    sqlite3_bind_int(stmt, 1, count);
    sqlite3_bind_int(stmt, 2, user_id);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        LOG_ERROR("Failed to update question count: " << sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        DBConnectionPool::getInstance().releaseConnection(db);
        return false;
    }

    sqlite3_finalize(stmt);
    DBConnectionPool::getInstance().releaseConnection(db);
    return true;
}

bool UserRepository::updateAnswerCount(int user_id, int count) {
    sqlite3* db = DBConnectionPool::getInstance().getConnection();
    if (!db) {
        LOG_ERROR("Failed to get database connection");
        return false;
    }

    const char* query = "UPDATE users SET answer_count = ? WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: " << sqlite3_errmsg(db));
        DBConnectionPool::getInstance().releaseConnection(db);
        return false;
    }

    sqlite3_bind_int(stmt, 1, count);
    sqlite3_bind_int(stmt, 2, user_id);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        LOG_ERROR("Failed to update answer count: " << sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        DBConnectionPool::getInstance().releaseConnection(db);
        return false;
    }

    sqlite3_finalize(stmt);
    DBConnectionPool::getInstance().releaseConnection(db);
    return true;
}
