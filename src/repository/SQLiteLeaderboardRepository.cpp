#include "SQLiteLeaderboardRepository.h"
#include <sqlite3.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace repository {

// 构造函数
SQLiteLeaderboardRepository::SQLiteLeaderboardRepository(const std::string& db_path)
    : SQLiteBaseRepository<model::Leaderboard>(db_path) {
    if (!initialize()) {
        throw ::std::runtime_error("Failed to initialize SQLiteLeaderboardRepository");
    }
}

// 从SQLiteBaseRepository继承的纯虚方法的实现
std::string SQLiteLeaderboardRepository::getTableName() const {
    return "leaderboards";
}

std::string SQLiteLeaderboardRepository::getCreateTableSql() const {
    return R"(
        CREATE TABLE IF NOT EXISTS leaderboards (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            game_id INTEGER NOT NULL,
            name TEXT NOT NULL,
            region TEXT NOT NULL DEFAULT 'global',
            score_rule TEXT NOT NULL DEFAULT 'highest',
            created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (game_id) REFERENCES games(id) ON DELETE CASCADE
        );
        CREATE INDEX IF NOT EXISTS idx_leaderboards_game_id ON leaderboards(game_id);
        CREATE INDEX IF NOT EXISTS idx_leaderboards_game_id_name ON leaderboards(game_id, name);
        CREATE INDEX IF NOT EXISTS idx_leaderboards_game_id_region ON leaderboards(game_id, region);
    )";
}

std::string SQLiteLeaderboardRepository::getInsertSql() const {
    return R"(
        INSERT INTO leaderboards (game_id, name, region, score_rule, created_at)
        VALUES (?, ?, ?, ?, ?);
    )";
}

std::string SQLiteLeaderboardRepository::getSelectByIdSql() const {
    return R"(
        SELECT id, game_id, name, region, score_rule, created_at
        FROM leaderboards
        WHERE id = ?;
    )";
}

std::string SQLiteLeaderboardRepository::getSelectAllSql() const {
    return R"(
        SELECT id, game_id, name, region, score_rule, created_at
        FROM leaderboards;
    )";
}

std::string SQLiteLeaderboardRepository::getUpdateSql() const {
    return R"(
        UPDATE leaderboards
        SET game_id = ?, name = ?, region = ?, score_rule = ?
        WHERE id = ?;
    )";
}

std::string SQLiteLeaderboardRepository::getDeleteByIdSql() const {
    return R"(
        DELETE FROM leaderboards
        WHERE id = ?;
    )";
}

model::Leaderboard SQLiteLeaderboardRepository::fromRow(sqlite3_stmt* stmt) const {
    // 提取字段值
    int id = sqlite3_column_int(stmt, 0);
    int game_id = sqlite3_column_int(stmt, 1);
    const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    const char* region = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    const char* score_rule_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

    // 转换ScoreRule
    model::ScoreRule score_rule = model::Leaderboard::fromString(score_rule_str);

    // 转换时间戳
    std::tm tm = {};
    std::istringstream ss(created_at_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto created_at = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    // 创建并返回Leaderboard对象
    return model::Leaderboard(id, game_id, name, region, score_rule, created_at);
}

void SQLiteLeaderboardRepository::bindValues(sqlite3_stmt* stmt, const model::Leaderboard& entity) const {
    // 绑定参数
    sqlite3_bind_int(stmt, 1, entity.getGameId());
    sqlite3_bind_text(stmt, 2, entity.getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, entity.getRegion().c_str(), -1, SQLITE_TRANSIENT);
    std::string score_rule_str = model::Leaderboard::toString(entity.getScoreRule());
    sqlite3_bind_text(stmt, 4, score_rule_str.c_str(), -1, SQLITE_TRANSIENT);

    // 转换时间戳为字符串
    auto created_at_time_t = std::chrono::system_clock::to_time_t(entity.getCreatedAt());
    std::tm tm = {};
    localtime_r(&created_at_time_t, &tm);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::string created_at_str = ss.str();

    sqlite3_bind_text(stmt, 5, created_at_str.c_str(), -1, SQLITE_TRANSIENT);

    // 如果是更新操作，还需要绑定id
    if (getUpdateSql() == sqlite3_sql(stmt)) {
        sqlite3_bind_int(stmt, 6, entity.getId());
    }
}

// 从LeaderboardRepository接口继承的方法的实现
std::vector<model::Leaderboard> SQLiteLeaderboardRepository::findByGameId(int game_id) {
    std::vector<model::Leaderboard> result;
    std::string sql = R"(
        SELECT id, game_id, name, region, score_rule, created_at
        FROM leaderboards
        WHERE game_id = ?;
    )";

    executePreparedStatement(sql, [game_id](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, game_id);
    }, [this, &result](sqlite3_stmt* stmt) {
        result.push_back(fromRow(stmt));
    });

    return result;
}

std::optional<model::Leaderboard> SQLiteLeaderboardRepository::findByGameIdAndName(int game_id, const std::string& name) {
    std::optional<model::Leaderboard> result;
    std::string sql = R"(
        SELECT id, game_id, name, region, score_rule, created_at
        FROM leaderboards
        WHERE game_id = ? AND name = ?;
    )";

    executePreparedStatement(sql, [game_id, &name](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, game_id);
        sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);
    }, [this, &result](sqlite3_stmt* stmt) {
        result = fromRow(stmt);
    });

    return result;
}

std::vector<model::Leaderboard> SQLiteLeaderboardRepository::findByGameIdAndRegion(int game_id, const std::string& region) {
    std::vector<model::Leaderboard> result;
    std::string sql = R"(
        SELECT id, game_id, name, region, score_rule, created_at
        FROM leaderboards
        WHERE game_id = ? AND region = ?;
    )";

    executePreparedStatement(sql, [game_id, &region](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, game_id);
        sqlite3_bind_text(stmt, 2, region.c_str(), -1, SQLITE_TRANSIENT);
    }, [this, &result](sqlite3_stmt* stmt) {
        result.push_back(fromRow(stmt));
    });

    return result;
}

} // namespace repository