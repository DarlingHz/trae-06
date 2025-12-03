#include "repository/DatabasePool.h"
#include <stdexcept>
#include <iostream>

namespace repository {

DatabasePool::DatabasePool(const std::string& db_path, size_t max_connections)
    : db_path_(db_path), max_connections_(max_connections) {
    for(size_t i = 0; i < max_connections_; ++i) {
        sqlite3* conn = create_connection();
        if(conn) {
            connections_.push_back(conn);
        }
    }
}

DatabasePool::~DatabasePool() {
    for(auto conn : connections_) {
        destroy_connection(conn);
    }
}

sqlite3* DatabasePool::create_connection() {
    sqlite3* conn = nullptr;
    int rc = sqlite3_open(db_path_.c_str(), &conn);
    if(rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_close(conn);
        return nullptr;
    }
    
    // Enable WAL mode for better concurrency
    char* err_msg = nullptr;
    rc = sqlite3_exec(conn, "PRAGMA journal_mode = WAL;", nullptr, nullptr, &err_msg);
    if(rc != SQLITE_OK) {
        std::cerr << "Failed to enable WAL: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }
    
    // Set busy timeout
    sqlite3_busy_timeout(conn, 5000);
    
    return conn;
}

void DatabasePool::destroy_connection(sqlite3* conn) {
    if(conn) {
        sqlite3_close(conn);
    }
}

std::shared_ptr<sqlite3> DatabasePool::get_connection() {
    std::unique_lock<std::mutex> lock(mutex_);
    
    while(connections_.empty()) {
        cv_.wait(lock);
    }
    
    sqlite3* conn = connections_.back();
    connections_.pop_back();
    
    return std::shared_ptr<sqlite3>(conn, [this](sqlite3* conn) {
        std::unique_lock<std::mutex> lock(mutex_);
        connections_.push_back(conn);
        cv_.notify_one();
    });
}

void DatabasePool::initialize_tables() {
    auto conn = get_connection();
    
    const char* create_users_table = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            email TEXT NOT NULL UNIQUE,
            password_hash TEXT NOT NULL,
            nickname TEXT NOT NULL,
            created_at INTEGER NOT NULL,
            updated_at INTEGER NOT NULL
        );
    )";
    
    const char* create_bookmarks_table = R"(
        CREATE TABLE IF NOT EXISTS bookmarks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            url TEXT NOT NULL,
            title TEXT NOT NULL,
            description TEXT,
            tags TEXT,
            folder TEXT DEFAULT '',
            is_favorite INTEGER DEFAULT 0,
            read_status TEXT DEFAULT 'unread',
            created_at INTEGER NOT NULL,
            updated_at INTEGER NOT NULL,
            last_accessed_at INTEGER NOT NULL,
            click_count INTEGER DEFAULT 0,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        );
    )";
    
    const char* create_indexes = R"(
        CREATE INDEX IF NOT EXISTS idx_user_id ON bookmarks(user_id);
        CREATE INDEX IF NOT EXISTS idx_created_at ON bookmarks(created_at);
        CREATE INDEX IF NOT EXISTS idx_last_accessed ON bookmarks(last_accessed_at);
        CREATE INDEX IF NOT EXISTS idx_is_favorite ON bookmarks(is_favorite);
        CREATE INDEX IF NOT EXISTS idx_read_status ON bookmarks(read_status);
        CREATE INDEX IF NOT EXISTS idx_folder ON bookmarks(folder);
        CREATE INDEX IF NOT EXISTS idx_user_tags ON bookmarks(user_id, tags);
        CREATE INDEX IF NOT EXISTS idx_user_created ON bookmarks(user_id, created_at);
        CREATE INDEX IF NOT EXISTS idx_user_last_accessed ON bookmarks(user_id, last_accessed_at);
    )";
    
    char* err_msg = nullptr;
    int rc = sqlite3_exec(conn.get(), create_users_table, nullptr, nullptr, &err_msg);
    if(rc != SQLITE_OK) {
        std::string error = err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error("Failed to create users table: " + error);
    }
    
    rc = sqlite3_exec(conn.get(), create_bookmarks_table, nullptr, nullptr, &err_msg);
    if(rc != SQLITE_OK) {
        std::string error = err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error("Failed to create bookmarks table: " + error);
    }
    
    rc = sqlite3_exec(conn.get(), create_indexes, nullptr, nullptr, &err_msg);
    if(rc != SQLITE_OK) {
        std::string error = err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error("Failed to create indexes: " + error);
    }
}

} // namespace repository
