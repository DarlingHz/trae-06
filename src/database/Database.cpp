#include "database/Database.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace pet_hospital {

// 全局数据库对象定义
std::unique_ptr<Database> g_database;

Database::~Database() {
    close();
}

bool Database::init(const std::string& connection_string) {
    try {
        // 加锁保证线程安全
        std::lock_guard<std::mutex> lock(db_mutex_);

        // 如果已经连接到数据库，先关闭连接
        if (db_ != nullptr) {
            close();
        }

        // 连接到 SQLite 数据库
        int rc = sqlite3_open(connection_string.c_str(), &db_);
        if (rc != SQLITE_OK) {
            std::string error_msg = "Failed to open database: " + std::string(sqlite3_errmsg(db_));
            sqlite3_close(db_);
            db_ = nullptr;
            LOG_ERROR(error_msg);
            return false;
        }

        // 初始化数据库表
        if (!init_tables()) {
            close();
            return false;
        }

        LOG_INFO("Database connected successfully: " + connection_string);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to initialize database: " + std::string(e.what()));
        return false;
    }
}

bool Database::execute_query(const std::string& sql, std::vector<std::vector<std::string>>& result) {
    return execute_sql(sql, &result, nullptr);
}

bool Database::execute_query(const std::string& sql, std::vector<std::vector<std::string>>& result, const std::vector<std::string>& params) {
    try {
        // 加锁保证线程安全
        std::lock_guard<std::mutex> lock(db_mutex_);

        // 检查数据库连接是否有效
        if (db_ == nullptr) {
            LOG_ERROR("Database connection is not valid");
            return false;
        }

        // 准备 SQL 语句
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::string error_msg_str = "Failed to prepare SQL: " + std::string(sql) + ", Error: " + std::string(sqlite3_errmsg(db_));
            LOG_ERROR(error_msg_str);
            return false;
        }

        // 绑定参数
        for (size_t i = 0; i < params.size(); ++i) {
            rc = sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
            if (rc != SQLITE_OK) {
                std::string error_msg_str = "Failed to bind parameter " + std::to_string(i + 1) + " for SQL: " + std::string(sql) + ", Error: " + std::string(sqlite3_errmsg(db_));
                LOG_ERROR(error_msg_str);
                sqlite3_finalize(stmt);
                return false;
            }
        }

        // 执行 SQL 语句并获取结果
        result.clear();
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            std::vector<std::string> row;
            int column_count = sqlite3_column_count(stmt);
            for (int i = 0; i < column_count; ++i) {
                const char* column_value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                row.push_back(column_value ? column_value : "");
            }
            result.push_back(row);
        }

        if (rc != SQLITE_DONE) {
            std::string error_msg_str = "Failed to execute SQL: " + std::string(sql) + ", Error: " + std::string(sqlite3_errmsg(db_));
            LOG_ERROR(error_msg_str);
            sqlite3_finalize(stmt);
            return false;
        }

        // 释放 SQL 语句
        sqlite3_finalize(stmt);

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to execute SQL: " + std::string(sql) + ", Error: " + std::string(e.what()));
        return false;
    }
}

bool Database::execute_statement(const std::string& sql, int& affected_rows) {
    return execute_sql(sql, nullptr, &affected_rows);
}

bool Database::execute_statement(const std::string& sql, int& affected_rows, const std::vector<std::string>& params) {
    try {
        // 加锁保证线程安全
        std::lock_guard<std::mutex> lock(db_mutex_);

        // 检查数据库连接是否有效
        if (db_ == nullptr) {
            LOG_ERROR("Database connection is not valid");
            return false;
        }

        // 准备 SQL 语句
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::string error_msg_str = "Failed to prepare SQL: " + std::string(sql) + ", Error: " + std::string(sqlite3_errmsg(db_));
            LOG_ERROR(error_msg_str);
            return false;
        }

        // 绑定参数
        for (size_t i = 0; i < params.size(); ++i) {
            rc = sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
            if (rc != SQLITE_OK) {
                std::string error_msg_str = "Failed to bind parameter " + std::to_string(i + 1) + " for SQL: " + std::string(sql) + ", Error: " + std::string(sqlite3_errmsg(db_));
                LOG_ERROR(error_msg_str);
                sqlite3_finalize(stmt);
                return false;
            }
        }

        // 执行 SQL 语句
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::string error_msg_str = "Failed to execute SQL: " + std::string(sql) + ", Error: " + std::string(sqlite3_errmsg(db_));
            LOG_ERROR(error_msg_str);
            sqlite3_finalize(stmt);
            return false;
        }

        // 获取受影响的行数
        affected_rows = sqlite3_changes(db_);

        // 释放 SQL 语句
        sqlite3_finalize(stmt);

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to execute SQL: " + std::string(sql) + ", Error: " + std::string(e.what()));
        return false;
    }
}

bool Database::begin_transaction() {
    int affected_rows;
    return execute_statement("BEGIN TRANSACTION;", affected_rows);
}

bool Database::commit_transaction() {
    int affected_rows;
    return execute_statement("COMMIT TRANSACTION;", affected_rows);
}

bool Database::rollback_transaction() {
    int affected_rows;
    return execute_statement("ROLLBACK TRANSACTION;", affected_rows);
}

void Database::close() {
    // 加锁保证线程安全
    std::lock_guard<std::mutex> lock(db_mutex_);

    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
        LOG_INFO("Database connection closed");
    }
}

bool Database::init_tables() {
    try {
        // 读取 SQL 表结构文件
        std::ifstream sql_file("sql/schema.sql");
        if (!sql_file.is_open()) {
            throw std::runtime_error("Failed to open SQL schema file: sql/schema.sql");
        }

        // 读取 SQL 文件内容
        std::stringstream sql_stream;
        sql_stream << sql_file.rdbuf();
        std::string sql = sql_stream.str();
        sql_file.close();

        // 执行 SQL 语句
        int affected_rows;
        if (!execute_statement(sql, affected_rows)) {
            return false;
        }

        LOG_INFO("Database tables initialized successfully");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to initialize database tables: " + std::string(e.what()));
        return false;
    }
}

bool Database::execute_sql(const std::string& sql, std::vector<std::vector<std::string>>* result, int* affected_rows) {
    try {
        // 加锁保证线程安全
        std::lock_guard<std::mutex> lock(db_mutex_);

        // 检查数据库连接是否有效
        if (db_ == nullptr) {
            LOG_ERROR("Database connection is not valid");
            return false;
        }

        // 执行 SQL 语句
        char* error_msg = nullptr;
        int rc = sqlite3_exec(db_, sql.c_str(), [](void* data, int argc, char** argv, [[maybe_unused]] char** col_names) -> int {
            auto result = static_cast<std::vector<std::vector<std::string>>*>(data);
            std::vector<std::string> row;
            for (int i = 0; i < argc; ++i) {
                row.push_back(argv[i] ? argv[i] : "");
            }
            result->push_back(row);
            return 0;
        }, result, &error_msg);

        // 处理执行结果
        if (rc != SQLITE_OK) {
            std::string error_msg_str = "Failed to execute SQL: " + std::string(sql) + ", Error: " + std::string(error_msg);
            sqlite3_free(error_msg);
            LOG_ERROR(error_msg_str);
            return false;
        }

        // 如果需要返回受影响的行数
        if (affected_rows != nullptr) {
            *affected_rows = sqlite3_changes(db_);
        }

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to execute SQL: " + std::string(sql) + ", Error: " + std::string(e.what()));
        return false;
    }
}

} // namespace pet_hospital
