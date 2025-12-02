#include "SQLiteGameRepository.h"
#include <sqlite3.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <memory>

namespace repository {

// 构造函数
SQLiteGameRepository::SQLiteGameRepository(const ::std::string& db_path)
    : SQLiteBaseRepository<model::Game>(db_path) {
    if (!initialize()) {
        throw ::std::runtime_error("Failed to initialize SQLiteGameRepository");
    }
}



// 从GameRepository接口继承的方法的实现
::std::optional<model::Game> SQLiteGameRepository::findByGameKey(const ::std::string& game_key) {
    ::std::optional<model::Game> result;
    executePreparedStatement(R"(
        SELECT id, game_key, name, created_at
        FROM games
        WHERE game_key = ?;
    )", [&game_key](sqlite3_stmt* stmt) {
        sqlite3_bind_text(stmt, 1, game_key.c_str(), -1, SQLITE_TRANSIENT);
    }, [this, &result](sqlite3_stmt* stmt) {
        result = fromRow(stmt);
    });
    return result;
}

bool SQLiteGameRepository::existsByGameKey(const ::std::string& game_key) {
    bool exists = false;
    executePreparedStatement(R"(
        SELECT EXISTS(SELECT 1 FROM games WHERE game_key = ?);
    )", [&game_key](sqlite3_stmt* stmt) {
        sqlite3_bind_text(stmt, 1, game_key.c_str(), -1, SQLITE_TRANSIENT);
    }, [&exists](sqlite3_stmt* stmt) {
        exists = sqlite3_column_int(stmt, 0) == 1;
    });
    return exists;
}

// 从SQLiteBaseRepository继承的纯虚方法的实现
::std::string SQLiteGameRepository::getTableName() const {
    return "games";
}

::std::string SQLiteGameRepository::getCreateTableSql() const {
    return R"(
        CREATE TABLE IF NOT EXISTS games (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            game_key TEXT NOT NULL UNIQUE,
            name TEXT NOT NULL,
            created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
        );
    )";
}

::std::string SQLiteGameRepository::getInsertSql() const {
    return R"(
        INSERT INTO games (game_key, name, created_at)
        VALUES (?, ?, ?);
    )";
}

::std::string SQLiteGameRepository::getSelectByIdSql() const {
    return R"(
        SELECT id, game_key, name, created_at
        FROM games
        WHERE id = ?;
    )";
}

::std::string SQLiteGameRepository::getSelectAllSql() const {
    return R"(
        SELECT id, game_key, name, created_at
        FROM games
        ORDER BY created_at DESC;
    )";
}

::std::string SQLiteGameRepository::getUpdateSql() const {
    return R"(
        UPDATE games SET game_key = ?, name = ?, created_at = ? WHERE id = ?;
    )";
}

::std::string SQLiteGameRepository::getDeleteByIdSql() const {
    return R"(
        DELETE FROM games WHERE id = ?;
    )";
}

model::Game SQLiteGameRepository::fromRow(sqlite3_stmt* stmt) const {
    model::Game game;
    game.setId(sqlite3_column_int(stmt, 0));
    game.setGameKey(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
    game.setName(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
    // 转换时间戳
    const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    ::std::tm tm = {};
    ::std::istringstream ss(created_at_str);
    ss >> ::std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto created_at = ::std::chrono::system_clock::from_time_t(::std::mktime(&tm));
    game.setCreatedAt(created_at);
    return game;
}

void SQLiteGameRepository::bindValues(sqlite3_stmt* stmt, const model::Game& entity) const {
    sqlite3_bind_text(stmt, 1, entity.getGameKey().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, entity.getName().c_str(), -1, SQLITE_TRANSIENT);
    // 转换时间戳为字符串
    auto created_at_time_t = ::std::chrono::system_clock::to_time_t(entity.getCreatedAt());
    ::std::tm tm = {};
    localtime_r(&created_at_time_t, &tm);
    ::std::ostringstream ss;
    ss << ::std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    ::std::string created_at_str = ss.str();
    sqlite3_bind_text(stmt, 3, created_at_str.c_str(), -1, SQLITE_TRANSIENT);
    // 如果是更新操作，还需要绑定id
    if (getUpdateSql() == sqlite3_sql(stmt)) {
        sqlite3_bind_int(stmt, 4, entity.getId());
    }
}

} // namespace repository