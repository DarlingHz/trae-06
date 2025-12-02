#include "SQLiteSessionRepository.h"
#include <sqlite3.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace repository {

// 构造函数
SQLiteSessionRepository::SQLiteSessionRepository(const std::string& db_path)
    : SQLiteBaseRepository<model::Session>(db_path) {
    if (!initialize()) {
        throw ::std::runtime_error("Failed to initialize SQLiteSessionRepository");
    }
}

// 从SQLiteBaseRepository继承的纯虚方法的实现
std::string SQLiteSessionRepository::getTableName() const {
    return "sessions";
}

std::string SQLiteSessionRepository::getCreateTableSql() const {
    return R"(
        CREATE TABLE IF NOT EXISTS sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            token TEXT NOT NULL UNIQUE,
            expire_at DATETIME NOT NULL,
            created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        );
        CREATE INDEX IF NOT EXISTS idx_sessions_token ON sessions(token);
        CREATE INDEX IF NOT EXISTS idx_sessions_user_id ON sessions(user_id);
        CREATE INDEX IF NOT EXISTS idx_sessions_expire_at ON sessions(expire_at);
    )";
}

std::string SQLiteSessionRepository::getInsertSql() const {
    return R"(
        INSERT INTO sessions (user_id, token, expire_at, created_at)
        VALUES (?, ?, ?, ?);
    )";
}

std::string SQLiteSessionRepository::getSelectByIdSql() const {
    return R"(
        SELECT id, user_id, token, expire_at, created_at
        FROM sessions
        WHERE id = ?;
    )";
}

std::string SQLiteSessionRepository::getSelectAllSql() const {
    return R"(
        SELECT id, user_id, token, expire_at, created_at
        FROM sessions;
    )";
}

std::string SQLiteSessionRepository::getUpdateSql() const {
    return R"(
        UPDATE sessions
        SET user_id = ?, token = ?, expire_at = ?
        WHERE id = ?;
    )";
}

std::string SQLiteSessionRepository::getDeleteByIdSql() const {
    return R"(
        DELETE FROM sessions
        WHERE id = ?;
    )";
}

model::Session SQLiteSessionRepository::fromRow(sqlite3_stmt* stmt) const {
    // 提取字段值
    int id = sqlite3_column_int(stmt, 0);
    int user_id = sqlite3_column_int(stmt, 1);
    const char* token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    const char* expire_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

    // 转换时间戳
    std::tm tm = {};
    std::istringstream ss_expire(expire_at_str);
    ss_expire >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto expire_at = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    std::istringstream ss_created(created_at_str);
    ss_created >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto created_at = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    // 创建并返回Session对象
    return model::Session(id, user_id, token, expire_at, created_at);
}

void SQLiteSessionRepository::bindValues(sqlite3_stmt* stmt, const model::Session& entity) const {
    // 绑定参数
    sqlite3_bind_int(stmt, 1, entity.getUserId());
    sqlite3_bind_text(stmt, 2, entity.getToken().c_str(), -1, SQLITE_TRANSIENT);

    // 转换时间戳为字符串
    auto expire_at_time_t = std::chrono::system_clock::to_time_t(entity.getExpireAt());
    std::tm tm = {};
    localtime_r(&expire_at_time_t, &tm);
    std::ostringstream ss_expire;
    ss_expire << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::string expire_at_str = ss_expire.str();

    sqlite3_bind_text(stmt, 3, expire_at_str.c_str(), -1, SQLITE_TRANSIENT);

    auto created_at_time_t = std::chrono::system_clock::to_time_t(entity.getCreatedAt());
    localtime_r(&created_at_time_t, &tm);
    std::ostringstream ss_created;
    ss_created << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::string created_at_str = ss_created.str();

    sqlite3_bind_text(stmt, 4, created_at_str.c_str(), -1, SQLITE_TRANSIENT);

    // 如果是更新操作，还需要绑定id
    if (getUpdateSql() == sqlite3_sql(stmt)) {
        sqlite3_bind_int(stmt, 5, entity.getId());
    }
}

// 从SessionRepository接口继承的方法的实现
std::optional<model::Session> SQLiteSessionRepository::findByToken(const std::string& token) {
    std::optional<model::Session> result;
    std::string sql = R"(
        SELECT id, user_id, token, expire_at, created_at
        FROM sessions
        WHERE token = ?;
    )";

    executePreparedStatement(sql, [&token](sqlite3_stmt* stmt) {
        sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    }, [this, &result](sqlite3_stmt* stmt) {
        result = fromRow(stmt);
    });

    return result;
}

std::vector<model::Session> SQLiteSessionRepository::findByUserId(int user_id) {
    std::vector<model::Session> result;
    std::string sql = R"(
        SELECT id, user_id, token, expire_at, created_at
        FROM sessions
        WHERE user_id = ?;
    )";

    executePreparedStatement(sql, [user_id](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, user_id);
    }, [this, &result](sqlite3_stmt* stmt) {
        result.push_back(fromRow(stmt));
    });

    return result;
}

bool SQLiteSessionRepository::deleteExpired() {
    std::string sql = R"(
        DELETE FROM sessions
        WHERE expire_at < CURRENT_TIMESTAMP;
    )";

    return executeSql(sql);
}

bool SQLiteSessionRepository::deleteByUserId(int user_id) {
    std::string sql = R"(
        DELETE FROM sessions
        WHERE user_id = ?;
    )";

    return executePreparedStatement(sql, [user_id](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, user_id);
    });
}

} // namespace repository