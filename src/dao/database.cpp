#include "parking/database.h"
#include <stdexcept>
#include <string>

// 全局数据库实例
namespace db {
    std::unique_ptr<Database> instance;

    void init(const std::string& db_path) {
        instance = std::make_unique<Database>(db_path);
    }

    void shutdown() {
        instance.reset();
    }

    Database& get() {
        if (!instance) {
            throw std::runtime_error("Database not initialized");
        }
        return *instance;
    }
}

// Database类实现
Database::Database(const std::string& db_path) {
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string error = sqlite3_errmsg(db_);
        sqlite3_close(db_);
        db_ = nullptr;
        throw std::runtime_error("Failed to open database: " + error);
    }

    // 启用外键约束
    execute("PRAGMA foreign_keys = ON;");
}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

Database::Database(Database&& other) noexcept : db_(other.db_) {
    other.db_ = nullptr;
}

Database& Database::operator=(Database&& other) noexcept {
    if (this != &other) {
        if (db_) {
            sqlite3_close(db_);
        }
        db_ = other.db_;
        other.db_ = nullptr;
    }
    return *this;
}

void Database::execute(const std::string& sql) {
    if (!db_) {
        throw std::runtime_error("Database connection not open");
    }

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        std::string err = err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error("Database execute failed: " + err + " (SQL: " + sql + ")");
    }
}

int64_t Database::last_insert_rowid() const {
    if (!db_) {
        throw std::runtime_error("Database connection not open");
    }
    return sqlite3_last_insert_rowid(db_);
}

void Database::begin_transaction() {
    execute("BEGIN TRANSACTION;");
}

void Database::commit() {
    execute("COMMIT;");
}

void Database::rollback() {
    execute("ROLLBACK;");
}
