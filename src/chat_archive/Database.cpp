#include "chat_archive/Database.h"
#include "chat_archive/Logger.h"
#include <stdexcept>

namespace chat_archive {

// DatabasePool implementation

bool DatabasePool::init(const std::string& db_path, int pool_size) {
    db_path_ = db_path;
    pool_size_ = pool_size;
    
    // 创建数据库连接池
    for (int i = 0; i < pool_size_; ++i) {
        sqlite3* conn = nullptr;
        int rc = sqlite3_open(db_path_.c_str(), &conn);
        
        if (rc != SQLITE_OK) {
            CHAT_ARCHIVE_LOG_ERROR("Failed to open database: {}", sqlite3_errmsg(conn));
            sqlite3_close(conn);
            return false;
        }
        
        // 设置数据库连接参数
        sqlite3_busy_timeout(conn, 5000); // 5秒超时
        
        // 初始化表结构（只需要在第一个连接上执行）
        if (i == 0 && !init_tables(conn)) {
            sqlite3_close(conn);
            return false;
        }
        
        connections_.push(std::shared_ptr<sqlite3>(conn, [](sqlite3* c) {
            sqlite3_close(c);
        }));
    }
    
    CHAT_ARCHIVE_LOG_INFO("Database pool initialized successfully with {} connections", pool_size_);
    return true;
}

std::shared_ptr<sqlite3> DatabasePool::get_connection() {
    std::unique_lock<std::mutex> lock(mutex_);
    
    if (connections_.empty()) {
        CHAT_ARCHIVE_LOG_WARN("Database pool is empty, waiting for connection");
        // 这里可以添加条件变量等待，但为了简单起见，我们直接返回空
        return nullptr;
    }
    
    auto conn = connections_.front();
    connections_.pop();
    
    return conn;
}

void DatabasePool::release_connection(std::shared_ptr<sqlite3> conn) {
    if (!conn) {
        return;
    }
    
    std::unique_lock<std::mutex> lock(mutex_);
    connections_.push(conn);
}

bool DatabasePool::init_tables(sqlite3* conn) {
    const char* create_users_table = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";
    
    const char* create_conversations_table = R"(
        CREATE TABLE IF NOT EXISTS conversations (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";
    
    const char* create_conversation_participants_table = R"(
        CREATE TABLE IF NOT EXISTS conversation_participants (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            conversation_id INTEGER NOT NULL,
            user_id INTEGER NOT NULL,
            FOREIGN KEY (conversation_id) REFERENCES conversations(id) ON DELETE CASCADE,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            UNIQUE(conversation_id, user_id)
        );
    )";
    
    const char* create_messages_table = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            conversation_id INTEGER NOT NULL,
            sender_id INTEGER NOT NULL,
            content TEXT NOT NULL,
            sent_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            edited_at DATETIME,
            deleted INTEGER DEFAULT 0,
            FOREIGN KEY (conversation_id) REFERENCES conversations(id) ON DELETE CASCADE,
            FOREIGN KEY (sender_id) REFERENCES users(id) ON DELETE CASCADE
        );
    )";
    
    // 创建索引
    const char* create_messages_index1 = R"(
        CREATE INDEX IF NOT EXISTS idx_messages_conversation_id_sent_at ON messages(conversation_id, sent_at);
    )";
    
    const char* create_messages_index2 = R"(
        CREATE INDEX IF NOT EXISTS idx_messages_sender_id_sent_at ON messages(sender_id, sent_at);
    )";
    
    const char* create_messages_index3 = R"(
        CREATE INDEX IF NOT EXISTS idx_messages_content ON messages(content);
    )";
    
    const char* create_conversation_participants_index = R"(
        CREATE INDEX IF NOT EXISTS idx_conversation_participants_user_id ON conversation_participants(user_id);
    )";
    
    const char* sql_statements[] = {
        create_users_table,
        create_conversations_table,
        create_conversation_participants_table,
        create_messages_table,
        create_messages_index1,
        create_messages_index2,
        create_messages_index3,
        create_conversation_participants_index
    };
    
    for (const char* sql : sql_statements) {
        char* errmsg = nullptr;
        int rc = sqlite3_exec(conn, sql, nullptr, nullptr, &errmsg);
        
        if (rc != SQLITE_OK) {
            CHAT_ARCHIVE_LOG_ERROR("Failed to create table/index: {}", errmsg);
            sqlite3_free(errmsg);
            return false;
        }
    }
    
    CHAT_ARCHIVE_LOG_INFO("Database tables and indexes created successfully");
    return true;
}

// DatabaseTransaction implementation

DatabaseTransaction::DatabaseTransaction(std::shared_ptr<sqlite3> conn) 
    : conn_(std::move(conn)) {
    if (!conn_) {
        throw std::invalid_argument("Invalid database connection");
    }
    
    char* errmsg = nullptr;
    int rc = sqlite3_exec(conn_.get(), "BEGIN TRANSACTION;", nullptr, nullptr, &errmsg);
    
    if (rc != SQLITE_OK) {
        std::string error_msg = errmsg;
        sqlite3_free(errmsg);
        throw std::runtime_error("Failed to begin transaction: " + error_msg);
    }
}

DatabaseTransaction::~DatabaseTransaction() {
    if (!conn_) {
        return;
    }
    
    if (!committed_ && !rolled_back_) {
        // 自动回滚未提交的事务
        char* errmsg = nullptr;
        sqlite3_exec(conn_.get(), "ROLLBACK TRANSACTION;", nullptr, nullptr, &errmsg);
        if (errmsg) {
            CHAT_ARCHIVE_LOG_WARN("Failed to rollback transaction: {}", errmsg);
            sqlite3_free(errmsg);
        }
    }
}

bool DatabaseTransaction::commit() {
    if (!conn_ || committed_ || rolled_back_) {
        return false;
    }
    
    char* errmsg = nullptr;
    int rc = sqlite3_exec(conn_.get(), "COMMIT TRANSACTION;", nullptr, nullptr, &errmsg);
    
    if (rc != SQLITE_OK) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to commit transaction: {}", errmsg);
        sqlite3_free(errmsg);
        return false;
    }
    
    committed_ = true;
    return true;
}

bool DatabaseTransaction::rollback() {
    if (!conn_ || committed_ || rolled_back_) {
        return false;
    }
    
    char* errmsg = nullptr;
    int rc = sqlite3_exec(conn_.get(), "ROLLBACK TRANSACTION;", nullptr, nullptr, &errmsg);
    
    if (rc != SQLITE_OK) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to rollback transaction: {}", errmsg);
        sqlite3_free(errmsg);
        return false;
    }
    
    rolled_back_ = true;
    return true;
}

// DatabaseResult implementation

DatabaseResult::DatabaseResult() = default;

DatabaseResult::~DatabaseResult() {
    if (stmt_) {
        sqlite3_finalize(stmt_);
    }
}

bool DatabaseResult::next() {
    if (!stmt_) {
        return false;
    }
    
    int rc = sqlite3_step(stmt_);
    
    if (rc == SQLITE_ROW) {
        return true;
    } else if (rc == SQLITE_DONE) {
        return false;
    } else {
        CHAT_ARCHIVE_LOG_ERROR("Failed to step through result: {}", sqlite3_errmsg(sqlite3_db_handle(stmt_)));
        return false;
    }
}

int DatabaseResult::get_int(int column) const {
    if (!stmt_) {
        return 0;
    }
    
    return sqlite3_column_int(stmt_, column);
}

int64_t DatabaseResult::get_int64(int column) const {
    if (!stmt_) {
        return 0;
    }
    
    return sqlite3_column_int64(stmt_, column);
}

double DatabaseResult::get_double(int column) {
    if (!stmt_) {
        return 0.0;
    }
    
    return sqlite3_column_double(stmt_, column);
}

std::string DatabaseResult::get_string(int column) const {
    if (!stmt_) {
        return "";
    }
    
    const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt_, column));
    if (!text) {
        return "";
    }
    
    return std::string(text);
}

const void* DatabaseResult::get_blob(int column, int* size) {
    if (!stmt_) {
        if (size) {
            *size = 0;
        }
        return nullptr;
    }
    
    const void* blob = sqlite3_column_blob(stmt_, column);
    if (size) {
        *size = sqlite3_column_bytes(stmt_, column);
    }
    
    return blob;
}

int DatabaseResult::get_column_count() const {
    if (!stmt_) {
        return 0;
    }
    
    return sqlite3_column_count(stmt_);
}

std::string DatabaseResult::get_column_name(int column) const {
    if (!stmt_) {
        return "";
    }
    
    const char* name = sqlite3_column_name(stmt_, column);
    return name ? std::string(name) : "";
}

void DatabaseResult::set_stmt(sqlite3_stmt* stmt) {
    if (stmt_) {
        sqlite3_finalize(stmt_);
    }
    
    stmt_ = stmt;
}

sqlite3_stmt* DatabaseResult::get_stmt() const {
    return stmt_;
}

// DatabaseQuery implementation

DatabaseQuery::DatabaseQuery(std::shared_ptr<sqlite3> conn) 
    : conn_(std::move(conn)) {
    if (!conn_) {
        throw std::invalid_argument("Invalid database connection");
    }
}

DatabaseQuery::~DatabaseQuery() {
    if (stmt_) {
        sqlite3_finalize(stmt_);
    }
}

bool DatabaseQuery::prepare(const std::string& sql) {
    if (!conn_) {
        return false;
    }
    
    if (stmt_) {
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
    }
    
    int rc = sqlite3_prepare_v2(conn_.get(), sql.c_str(), sql.size(), &stmt_, nullptr);
    
    if (rc != SQLITE_OK) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to prepare query: {}", sqlite3_errmsg(conn_.get()));
        return false;
    }
    
    return true;
}

bool DatabaseQuery::bind_int(int index, int value) {
    if (!stmt_) {
        return false;
    }
    
    int rc = sqlite3_bind_int(stmt_, index, value);
    
    if (rc != SQLITE_OK) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to bind int: {}", sqlite3_errmsg(sqlite3_db_handle(stmt_)));
        return false;
    }
    
    return true;
}

bool DatabaseQuery::bind_int64(int index, int64_t value) {
    if (!stmt_) {
        return false;
    }
    
    int rc = sqlite3_bind_int64(stmt_, index, value);
    
    if (rc != SQLITE_OK) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to bind int64: {}", sqlite3_errmsg(sqlite3_db_handle(stmt_)));
        return false;
    }
    
    return true;
}

bool DatabaseQuery::bind_double(int index, double value) {
    if (!stmt_) {
        return false;
    }
    
    int rc = sqlite3_bind_double(stmt_, index, value);
    
    if (rc != SQLITE_OK) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to bind double: {}", sqlite3_errmsg(sqlite3_db_handle(stmt_)));
        return false;
    }
    
    return true;
}

bool DatabaseQuery::bind_string(int index, const std::string& value) {
    if (!stmt_) {
        return false;
    }
    
    int rc = sqlite3_bind_text(stmt_, index, value.c_str(), value.size(), SQLITE_TRANSIENT);
    
    if (rc != SQLITE_OK) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to bind string: {}", sqlite3_errmsg(sqlite3_db_handle(stmt_)));
        return false;
    }
    
    return true;
}

bool DatabaseQuery::bind_blob(int index, const void* data, int size) {
    if (!stmt_) {
        return false;
    }
    
    int rc = sqlite3_bind_blob(stmt_, index, data, size, SQLITE_TRANSIENT);
    
    if (rc != SQLITE_OK) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to bind blob: {}", sqlite3_errmsg(sqlite3_db_handle(stmt_)));
        return false;
    }
    
    return true;
}

bool DatabaseQuery::bind_null(int index) {
    if (!stmt_) {
        return false;
    }
    
    int rc = sqlite3_bind_null(stmt_, index);
    
    if (rc != SQLITE_OK) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to bind null: {}", sqlite3_errmsg(sqlite3_db_handle(stmt_)));
        return false;
    }
    
    return true;
}

bool DatabaseQuery::execute() {
    if (!stmt_) {
        return false;
    }
    
    int rc = sqlite3_step(stmt_);
    
    if (rc == SQLITE_DONE) {
        return true;
    } else if (rc == SQLITE_ROW) {
        // 查询返回结果，需要通过get_result()获取
        return true;
    } else {
        CHAT_ARCHIVE_LOG_ERROR("Failed to execute query: {}", sqlite3_errmsg(sqlite3_db_handle(stmt_)));
        return false;
    }
}

DatabaseResult DatabaseQuery::get_result() {
    DatabaseResult result;
    
    if (!stmt_) {
        return result;
    }
    
    // 将stmt_转移给result，这样就不会在析构函数中finalize
    result.set_stmt(stmt_);
    stmt_ = nullptr;
    
    return result;
}

int64_t DatabaseQuery::get_affected_rows() const {
    if (!conn_) {
        return 0;
    }
    
    return sqlite3_changes(conn_.get());
}

int64_t DatabaseQuery::get_last_insert_rowid() const {
    if (!conn_) {
        return 0;
    }
    
    return sqlite3_last_insert_rowid(conn_.get());
}

} // namespace chat_archive