#include "database.hpp"
#include <iostream>
#include <stdexcept>

using namespace std;

bool Database::init(const string& db_path) {
    // 如果已经连接，先关闭
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }

    // 打开数据库连接
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        cerr << "无法打开数据库: " << sqlite3_errmsg(db_) << endl;
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    // 创建表结构
    if (!createTables()) {
        cerr << "无法创建表结构" << endl;
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    return true;
}

sqlite3* Database::getConnection() {
    return db_;
}

bool Database::execute(const string& sql) {
    if (db_ == nullptr) {
        cerr << "数据库未连接" << endl;
        return false;
    }

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        cerr << "SQL 执行失败: " << err_msg << endl;
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool Database::query(const string& sql, function<bool(int, char**, char**)> callback) {
    if (db_ == nullptr) {
        cerr << "数据库未连接" << endl;
        return false;
    }

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), [](void* data, int argc, char** argv, char** col_names) {
        auto callback = *static_cast<function<bool(int, char**, char**)>*>(data);
        return callback(argc, argv, col_names) ? 0 : 1;
    }, &callback, &err_msg);

    if (rc != SQLITE_OK) {
        cerr << "SQL 查询失败: " << err_msg << endl;
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool Database::beginTransaction() {
    return execute("BEGIN TRANSACTION;");
}

bool Database::commitTransaction() {
    return execute("COMMIT TRANSACTION;");
}

bool Database::rollbackTransaction() {
    return execute("ROLLBACK TRANSACTION;");
}

bool Database::createTables() {
    // 创建用户表
    string create_users_table = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            email TEXT NOT NULL UNIQUE,
            password TEXT NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    // 创建菜谱表
    string create_recipes_table = R"(
        CREATE TABLE IF NOT EXISTS recipes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            owner_user_id INTEGER NOT NULL,
            title TEXT NOT NULL,
            description TEXT,
            servings INTEGER,
            tags TEXT,
            ingredients TEXT,
            steps TEXT,
            is_favorite INTEGER DEFAULT 0,
            is_archived INTEGER DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (owner_user_id) REFERENCES users(id)
        );
    )";

    // 创建餐食计划表
    string create_meal_plans_table = R"(
        CREATE TABLE IF NOT EXISTS meal_plans (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            week_start_date TEXT NOT NULL,
            entries TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id),
            UNIQUE(user_id, week_start_date)
        );
    )";

    // 创建用户token表
    string create_user_tokens_table = R"(
        CREATE TABLE IF NOT EXISTS user_tokens (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            token TEXT NOT NULL,
            expires_at INTEGER NOT NULL,
            created_at INTEGER DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        );
    )";

    // 创建餐食计划表
    string create_meal_plans_table = R"(
        CREATE TABLE IF NOT EXISTS meal_plans (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            week_start_date TEXT NOT NULL,
            entries TEXT NOT NULL,
            created_at INTEGER DEFAULT CURRENT_TIMESTAMP,
            updated_at INTEGER DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            UNIQUE(user_id, week_start_date)
        );
    )";

    // 创建索引
    string create_token_index = "CREATE INDEX IF NOT EXISTS idx_user_tokens_token ON user_tokens(token);";
    string create_user_id_index = "CREATE INDEX IF NOT EXISTS idx_user_tokens_user_id ON user_tokens(user_id);";
    string create_meal_plans_user_id_index = "CREATE INDEX IF NOT EXISTS idx_meal_plans_user_id ON meal_plans(user_id);";

    return execute(create_users_table) &&
           execute(create_recipes_table) &&
           execute(create_user_tokens_table) &&
           execute(create_meal_plans_table) &&
           execute(create_token_index) &&
           execute(create_user_id_index) &&
           execute(create_meal_plans_user_id_index);
}
