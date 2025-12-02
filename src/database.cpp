#include "database.h"
#include "log.h"
#include <sqlite3.h>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <cstring>

namespace recruitment {

// DatabaseConnection 实现

DatabaseConnection::DatabaseConnection(const std::string& db_path)
    : db_(nullptr), is_in_transaction_(false) {
    // 打开数据库连接
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string error_msg = "Failed to open database: " + std::string(sqlite3_errmsg(db_));
        sqlite3_close(db_);
        db_ = nullptr;
        LOG_ERROR(error_msg);
        throw std::runtime_error(error_msg);
    }
    LOG_DEBUG("Database connection established: " + db_path);
}

DatabaseConnection::~DatabaseConnection() {
    if (db_ != nullptr) {
        // 如果在事务中，回滚事务
        if (is_in_transaction_) {
            rollbackTransaction();
        }
        sqlite3_close(db_);
        db_ = nullptr;
        LOG_DEBUG("Database connection closed");
    }
}

sqlite3* DatabaseConnection::getConnection() const {
    return db_;
}

QueryResult DatabaseConnection::executeQuery(const std::string& sql, const std::vector<QueryParameter>& parameters) {
    sqlite3_stmt* stmt = nullptr;
    QueryResult result;

    try {
        // 准备SQL语句
        int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::string error_msg = "Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db_));
            LOG_ERROR(error_msg);
            throw std::runtime_error(error_msg);
        }

        // 绑定参数
        for (size_t i = 0; i < parameters.size(); ++i) {
            const QueryParameter& param = parameters[i];
            int index = static_cast<int>(i + 1); // SQLite参数索引从1开始

            switch (param.type) {
                case QueryParameterType::INTEGER:
                    rc = sqlite3_bind_int64(stmt, index, param.int_value);
                    break;
                case QueryParameterType::TEXT:
                    rc = sqlite3_bind_text(stmt, index, param.text_value.c_str(), -1, SQLITE_TRANSIENT);
                    break;
                case QueryParameterType::REAL:
                    rc = sqlite3_bind_double(stmt, index, param.real_value);
                    break;
                case QueryParameterType::BLOB:
                    rc = sqlite3_bind_blob(stmt, index, param.blob_value.data(), static_cast<int>(param.blob_value.size()), SQLITE_TRANSIENT);
                    break;
                case QueryParameterType::NULL_TYPE:
                    rc = sqlite3_bind_null(stmt, index);
                    break;
            }

            if (rc != SQLITE_OK) {
                std::string error_msg = "Failed to bind parameter " + std::to_string(index) + ": " + std::string(sqlite3_errmsg(db_));
                LOG_ERROR(error_msg);
                throw std::runtime_error(error_msg);
            }
        }

        // 执行查询
        rc = sqlite3_step(stmt);
        while (rc == SQLITE_ROW) {
            // 获取列数
            int column_count = sqlite3_column_count(stmt);
            QueryRow row;

            // 获取每列的值
            for (int i = 0; i < column_count; ++i) {
                const char* column_name = sqlite3_column_name(stmt, i);
                int column_type = sqlite3_column_type(stmt, i);

                QueryRow::ColumnValue value;
                switch (column_type) {
                    case SQLITE_INTEGER:
                        value.int_value = sqlite3_column_int64(stmt, i);
                        value.is_null = false;
                        break;
                    case SQLITE_TEXT:
                        value.text_value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                        value.is_null = false;
                        break;
                    case SQLITE_FLOAT:
                        value.real_value = sqlite3_column_double(stmt, i);
                        value.is_null = false;
                        break;
                    case SQLITE_BLOB: {
                        const void* blob_data = sqlite3_column_blob(stmt, i);
                        int blob_size = sqlite3_column_bytes(stmt, i);
                        value.blob_value.assign(static_cast<const char*>(blob_data), static_cast<const char*>(blob_data) + blob_size);
                        value.is_null = false;
                        break;
                    }
                    case SQLITE_NULL:
                        value.is_null = true;
                        break;
                }

                row[column_name] = value;
            }

            result.rows.push_back(row);
            rc = sqlite3_step(stmt);
        }

        // 检查执行结果
        if (rc != SQLITE_DONE) {
            std::string error_msg = "Failed to execute SQL query: " + std::string(sqlite3_errmsg(db_));
            LOG_ERROR(error_msg);
            throw std::runtime_error(error_msg);
        }

        // 获取受影响的行数
        result.rows_affected = sqlite3_changes(db_);
        // 获取最后插入的ID
        result.last_insert_id = sqlite3_last_insert_rowid(db_);

        LOG_DEBUG("SQL query executed successfully: " + sql);

    } catch (const std::exception& e) {
        LOG_ERROR("Error executing SQL query: " + std::string(e.what()));
        throw;
    } catch (...) {
        // 清理语句
        if (stmt != nullptr) {
            sqlite3_finalize(stmt);
        }
        throw;
    }

    return result;
}

int DatabaseConnection::executeNonQuery(const std::string& sql) {
    return executeNonQuery(sql, std::vector<QueryParameter>());
}

int DatabaseConnection::executeNonQuery(const std::string& sql, const std::vector<QueryParameter>& parameters) {
    QueryResult result = executeQuery(sql, parameters);
    return static_cast<int>(result.rows_affected);
}

bool DatabaseConnection::beginTransaction() {
    if (is_in_transaction_) {
        LOG_WARN("Transaction already in progress");
        return true;
    }

    int rc = sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::string error_msg = "Failed to begin transaction: " + std::string(sqlite3_errmsg(db_));
        LOG_ERROR(error_msg);
        return false;
    }

    is_in_transaction_ = true;
    LOG_DEBUG("Transaction started");
    return true;
}

bool DatabaseConnection::commitTransaction() {
    if (!is_in_transaction_) {
        LOG_WARN("No transaction in progress");
        return true;
    }

    int rc = sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::string error_msg = "Failed to commit transaction: " + std::string(sqlite3_errmsg(db_));
        LOG_ERROR(error_msg);
        return false;
    }

    is_in_transaction_ = false;
    LOG_DEBUG("Transaction committed");
    return true;
}

bool DatabaseConnection::rollbackTransaction() {
    if (!is_in_transaction_) {
        LOG_WARN("No transaction in progress");
        return true;
    }

    int rc = sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::string error_msg = "Failed to rollback transaction: " + std::string(sqlite3_errmsg(db_));
        LOG_ERROR(error_msg);
        return false;
    }

    is_in_transaction_ = false;
    LOG_DEBUG("Transaction rolled back");
    return true;
}

// ConnectionPool 实现

ConnectionPool::ConnectionPool(const std::string& db_path, int max_connections)
    : db_path_(db_path), max_connections_(max_connections), current_connections_(0) {
    LOG_INFO("Connection pool initialized with max connections: " + std::to_string(max_connections_));
}

ConnectionPool::~ConnectionPool() {
    // 关闭所有连接
    std::lock_guard<std::mutex> lock(mutex_);
    while (!available_connections_.empty()) {
        available_connections_.pop();
    }
    all_connections_.clear();
    LOG_INFO("Connection pool destroyed");
}

std::shared_ptr<DatabaseConnection> ConnectionPool::getConnection(int timeout_ms) {
    std::unique_lock<std::mutex> lock(mutex_);

    // 等待可用连接或可以创建新连接
    if (timeout_ms > 0) {
        auto now = std::chrono::system_clock::now();
        auto timeout = now + std::chrono::milliseconds(timeout_ms);
        
        while (available_connections_.empty() && current_connections_ >= max_connections_) {
            if (condition_.wait_until(lock, timeout) == std::cv_status::timeout) {
                LOG_WARN("Timeout waiting for database connection");
                return nullptr;
            }
        }
    } else {
        while (available_connections_.empty() && current_connections_ >= max_connections_) {
            condition_.wait(lock);
        }
    }

    std::shared_ptr<DatabaseConnection> conn;

    // 如果有可用连接，使用它
    if (!available_connections_.empty()) {
        conn = available_connections_.front();
        available_connections_.pop();
        LOG_DEBUG("Reusing database connection");
    } else {
        // 否则创建新连接
        try {
            conn = std::make_shared<DatabaseConnection>(db_path_);
            current_connections_++;
            LOG_DEBUG("Created new database connection (total: " + std::to_string(current_connections_) + ")");
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create database connection: " + std::string(e.what()));
            throw;
        }
    }

    // 使用自定义删除器，当shared_ptr超出范围时将连接放回池
    return std::shared_ptr<DatabaseConnection>(conn.get(), [this, conn](DatabaseConnection* /*conn*/) {
        this->returnConnection(conn);
    });
}

void ConnectionPool::returnConnection(std::shared_ptr<DatabaseConnection> connection) {
    std::lock_guard<std::mutex> lock(mutex_);
    available_connections_.push(connection);
    condition_.notify_one();
    LOG_DEBUG("Database connection returned to pool");
}

void ConnectionPool::getStatus(int& total_connections, int& available_connections, int& used_connections) const {
    std::lock_guard<std::mutex> lock(mutex_);
    total_connections = current_connections_;
    available_connections = static_cast<int>(available_connections_.size());
    used_connections = total_connections - available_connections;
}



// Database 实现

std::unique_ptr<ConnectionPool> Database::connection_pool_;
std::mutex Database::mutex_;

std::shared_ptr<DatabaseConnection> Database::getConnection(int timeout_ms) {
    return connection_pool_->getConnection(timeout_ms);
}

void Database::returnConnection(std::shared_ptr<DatabaseConnection> connection) {
    connection_pool_->returnConnection(connection);
}

void Database::getPoolStatus(int& total_connections, int& available_connections, int& used_connections) {
    connection_pool_->getStatus(total_connections, available_connections, used_connections);
}

bool Database::executeScript(const std::string& sql_script) {
    // 实现执行SQL脚本的功能
    // 这里只是一个示例，实际实现可能需要更复杂的逻辑
    auto conn = getConnection();
    if (!conn) {
        LOG_ERROR("Failed to get database connection for executing script");
        return false;
    }

    // 分割SQL脚本为多个语句
    std::istringstream iss(sql_script);
    std::string line;
    std::string statement;

    while (std::getline(iss, line)) {
        // 去除行首和行尾的空白字符
        size_t start = line.find_first_not_of(" \t\n\r");
        size_t end = line.find_last_not_of(" \t\n\r");
        if (start == std::string::npos || end == std::string::npos) {
            continue; // 空行
        }
        std::string trimmed_line = line.substr(start, end - start + 1);

        // 忽略注释行
        if (trimmed_line.size() >= 2 && trimmed_line[0] == '-' && trimmed_line[1] == '-') {
            continue;
        }

        // 添加到当前语句
        statement += trimmed_line;

        // 如果语句以分号结尾，则执行该语句
        if (statement.back() == ';') {
            try {
                conn->executeNonQuery(statement);
                LOG_DEBUG("Executed SQL statement: " + statement);
            } catch (const std::exception& e) {
                LOG_ERROR("Failed to execute SQL statement: " + statement + ", error: " + e.what());
                return false;
            }

            // 重置语句
            statement.clear();
        }
    }

    // 执行最后一个语句（如果有的话）
    if (!statement.empty()) {
        try {
            conn->executeNonQuery(statement);
            LOG_DEBUG("Executed SQL statement: " + statement);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to execute SQL statement: " + statement + ", error: " + e.what());
            return false;
        }
    }

    LOG_INFO("Successfully executed SQL script");
    return true;
}

} // namespace recruitment