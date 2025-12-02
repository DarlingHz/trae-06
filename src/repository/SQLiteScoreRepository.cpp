#include "SQLiteScoreRepository.h"
#include <sqlite3.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <map>
#include <algorithm>

namespace repository {

// 构造函数
SQLiteScoreRepository::SQLiteScoreRepository(const std::string& db_path)
    : SQLiteBaseRepository<model::Score>(db_path) {
    if (!initialize()) {
        throw ::std::runtime_error("Failed to initialize SQLiteScoreRepository");
    }
}

// 从SQLiteBaseRepository继承的纯虚方法的实现
std::string SQLiteScoreRepository::getTableName() const {
    return "scores";
}

std::string SQLiteScoreRepository::getCreateTableSql() const {
    return R"(
        CREATE TABLE IF NOT EXISTS scores (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            leaderboard_id INTEGER NOT NULL,
            user_id INTEGER NOT NULL,
            score INTEGER NOT NULL,
            extra_data TEXT NOT NULL DEFAULT '',
            created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (leaderboard_id) REFERENCES leaderboards(id) ON DELETE CASCADE,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        );
        CREATE INDEX IF NOT EXISTS idx_scores_leaderboard_id ON scores(leaderboard_id);
        CREATE INDEX IF NOT EXISTS idx_scores_user_id ON scores(user_id);
        CREATE INDEX IF NOT EXISTS idx_scores_leaderboard_id_user_id ON scores(leaderboard_id, user_id);
        CREATE INDEX IF NOT EXISTS idx_scores_leaderboard_id_score ON scores(leaderboard_id, score DESC);
    )";
}

std::string SQLiteScoreRepository::getInsertSql() const {
    return R"(
        INSERT INTO scores (leaderboard_id, user_id, score, extra_data, created_at, updated_at)
        VALUES (?, ?, ?, ?, ?, ?);
    )";
}

std::string SQLiteScoreRepository::getSelectByIdSql() const {
    return R"(
        SELECT id, leaderboard_id, user_id, score, extra_data, created_at, updated_at
        FROM scores
        WHERE id = ?;
    )";
}

std::string SQLiteScoreRepository::getSelectAllSql() const {
    return R"(
        SELECT id, leaderboard_id, user_id, score, extra_data, created_at, updated_at
        FROM scores;
    )";
}

std::string SQLiteScoreRepository::getUpdateSql() const {
    return R"(
        UPDATE scores
        SET leaderboard_id = ?, user_id = ?, score = ?, extra_data = ?, updated_at = ?
        WHERE id = ?;
    )";
}

std::string SQLiteScoreRepository::getDeleteByIdSql() const {
    return R"(
        DELETE FROM scores
        WHERE id = ?;
    )";
}

model::Score SQLiteScoreRepository::fromRow(sqlite3_stmt* stmt) const {
    // 提取字段值
    int id = sqlite3_column_int(stmt, 0);
    int leaderboard_id = sqlite3_column_int(stmt, 1);
    int user_id = sqlite3_column_int(stmt, 2);
    long long score = sqlite3_column_int64(stmt, 3);
    const char* extra_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    const char* created_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    const char* updated_at_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

    // 转换时间戳
    std::tm tm = {};
    std::istringstream ss_created(created_at_str);
    ss_created >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto created_at = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    std::tm tm_updated = {};
    std::istringstream ss_updated(updated_at_str);
    ss_updated >> std::get_time(&tm_updated, "%Y-%m-%d %H:%M:%S");
    auto updated_at = std::chrono::system_clock::from_time_t(std::mktime(&tm_updated));

    // 创建并返回Score对象
    return model::Score(id, leaderboard_id, user_id, score, extra_data, created_at, updated_at);
}

void SQLiteScoreRepository::bindValues(sqlite3_stmt* stmt, const model::Score& entity) const {
    // 绑定参数
    sqlite3_bind_int(stmt, 1, entity.getLeaderboardId());
    sqlite3_bind_int(stmt, 2, entity.getUserId());
    sqlite3_bind_int64(stmt, 3, entity.getScore());
    sqlite3_bind_text(stmt, 4, entity.getExtraData().c_str(), -1, SQLITE_TRANSIENT);

    // 转换时间戳为字符串
    auto updated_at_time_t = std::chrono::system_clock::to_time_t(entity.getUpdatedAt());
    std::tm tm_updated = {};
    localtime_r(&updated_at_time_t, &tm_updated);
    std::ostringstream ss_updated;
    ss_updated << std::put_time(&tm_updated, "%Y-%m-%d %H:%M:%S");
    std::string updated_at_str = ss_updated.str();

    // 检查是否为更新操作
    std::string sql = sqlite3_sql(stmt);
    if (sql.find("UPDATE scores") != std::string::npos) {
        // 对于更新操作，第5个参数是updated_at，第6个参数是id
        sqlite3_bind_text(stmt, 5, updated_at_str.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 6, entity.getId());
    } else {
        // 对于插入操作，第5个参数是created_at，第6个参数是updated_at
        auto created_at_time_t = std::chrono::system_clock::to_time_t(entity.getCreatedAt());
        std::tm tm_created = {};
        localtime_r(&created_at_time_t, &tm_created);
        std::ostringstream ss_created;
        ss_created << std::put_time(&tm_created, "%Y-%m-%d %H:%M:%S");
        std::string created_at_str = ss_created.str();

        sqlite3_bind_text(stmt, 5, created_at_str.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, updated_at_str.c_str(), -1, SQLITE_TRANSIENT);
    }
}

// 从ScoreRepository接口继承的方法的实现
std::vector<model::Score> SQLiteScoreRepository::findByLeaderboardId(int leaderboard_id, int limit) {
    std::vector<model::Score> result;
    std::string sql = R"(
        SELECT id, leaderboard_id, user_id, score, extra_data, created_at, updated_at
        FROM scores
        WHERE leaderboard_id = ?
        ORDER BY created_at DESC
    )";

    if (limit > 0) {
        sql += " LIMIT ?";
    }

    executePreparedStatement(sql, [leaderboard_id, limit](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, leaderboard_id);
        if (limit > 0) {
            sqlite3_bind_int(stmt, 2, limit);
        }
    }, [this, &result](sqlite3_stmt* stmt) {
        result.push_back(fromRow(stmt));
    });

    return result;
}

std::vector<model::Score> SQLiteScoreRepository::findByUserId(int user_id, int limit) {
    std::vector<model::Score> result;
    std::string sql = R"(
        SELECT id, leaderboard_id, user_id, score, extra_data, created_at, updated_at
        FROM scores
        WHERE user_id = ?
        ORDER BY created_at DESC
    )";

    if (limit > 0) {
        sql += " LIMIT ?";
    }

    executePreparedStatement(sql, [user_id, limit](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, user_id);
        if (limit > 0) {
            sqlite3_bind_int(stmt, 2, limit);
        }
    }, [this, &result](sqlite3_stmt* stmt) {
        result.push_back(fromRow(stmt));
    });

    return result;
}

std::vector<model::Score> SQLiteScoreRepository::findByLeaderboardIdAndUserId(int leaderboard_id, int user_id, int limit) {
    std::vector<model::Score> result;
    std::string sql = R"(
        SELECT id, leaderboard_id, user_id, score, extra_data, created_at, updated_at
        FROM scores
        WHERE leaderboard_id = ? AND user_id = ?
        ORDER BY created_at DESC
    )";

    if (limit > 0) {
        sql += " LIMIT ?";
    }

    executePreparedStatement(sql, [leaderboard_id, user_id, limit](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, leaderboard_id);
        sqlite3_bind_int(stmt, 2, user_id);
        if (limit > 0) {
            sqlite3_bind_int(stmt, 3, limit);
        }
    }, [this, &result](sqlite3_stmt* stmt) {
        result.push_back(fromRow(stmt));
    });

    return result;
}

std::vector<model::Score> SQLiteScoreRepository::findTopByLeaderboardId(int leaderboard_id, int limit) {
    std::vector<model::Score> result;
    std::string sql = R"(
        SELECT s.id, s.leaderboard_id, s.user_id, s.score, s.extra_data, s.created_at, s.updated_at
        FROM scores s
        WHERE s.leaderboard_id = ?
        ORDER BY s.score DESC, s.created_at ASC
    )";

    executePreparedStatement(sql, [leaderboard_id](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, leaderboard_id);
    }, [this, &result](sqlite3_stmt* stmt) {
        result.push_back(fromRow(stmt));
    });

    // Filter to only keep the best score for each user
    std::map<int, model::Score> best_scores_per_user;
    for (const auto& score : result) {
        int user_id = score.getUserId();
        if (best_scores_per_user.find(user_id) == best_scores_per_user.end()) {
            best_scores_per_user[user_id] = score;
        } else {
            const auto& existing_score = best_scores_per_user[user_id];
            if (score.getScore() > existing_score.getScore() ||
                (score.getScore() == existing_score.getScore() &&
                 score.getCreatedAt() < existing_score.getCreatedAt())) {
                best_scores_per_user[user_id] = score;
            }
        }
    }

    // Convert the map to a vector
    std::vector<model::Score> best_scores;
    for (const auto& pair : best_scores_per_user) {
        best_scores.push_back(pair.second);
    }

    // Sort the best scores by score descending and created_at ascending
    std::sort(best_scores.begin(), best_scores.end(), [](const model::Score& a, const model::Score& b) {
        if (a.getScore() != b.getScore()) {
            return a.getScore() > b.getScore();
        }
        return a.getCreatedAt() < b.getCreatedAt();
    });

    // If limit is greater than 0, return only the first 'limit' results
    if (limit > 0 && best_scores.size() > static_cast<size_t>(limit)) {
        best_scores.resize(limit);
    }

    return best_scores;
}

std::optional<model::Score> SQLiteScoreRepository::findBestByLeaderboardIdAndUserId(int leaderboard_id, int user_id) {
    std::optional<model::Score> result;
    std::string sql = R"(
        SELECT id, leaderboard_id, user_id, score, extra_data, created_at, updated_at
        FROM scores
        WHERE leaderboard_id = ? AND user_id = ?
        ORDER BY score DESC, created_at ASC
        LIMIT 1
    )";

    executePreparedStatement(sql, [leaderboard_id, user_id](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, leaderboard_id);
        sqlite3_bind_int(stmt, 2, user_id);
    }, [this, &result](sqlite3_stmt* stmt) {
        result = fromRow(stmt);
    });

    return result;
}

std::optional<int> SQLiteScoreRepository::findRankByLeaderboardIdAndUserId(int leaderboard_id, int user_id) {
    std::optional<int> rank;
    std::string sql = R"(
        SELECT COUNT(*) + 1
        FROM (
            SELECT DISTINCT user_id, MAX(score) as max_score
            FROM scores
            WHERE leaderboard_id = ?
            GROUP BY user_id
        ) AS user_scores
        WHERE max_score > (
            SELECT MAX(score)
            FROM scores
            WHERE leaderboard_id = ? AND user_id = ?
        )
    )";

    executePreparedStatement(sql, [leaderboard_id, user_id](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, leaderboard_id);
        sqlite3_bind_int(stmt, 2, leaderboard_id);
        sqlite3_bind_int(stmt, 3, user_id);
    }, [&rank](sqlite3_stmt* stmt) {
        int count = sqlite3_column_int(stmt, 0);
        if (count > 0) {
            rank = count;
        } else {
            rank = std::nullopt;
        }
    });

    return rank;
}

bool SQLiteScoreRepository::deleteByLeaderboardId(int leaderboard_id) {
    std::string sql = R"(
        DELETE FROM scores
        WHERE leaderboard_id = ?;
    )";

    return executePreparedStatement(sql, [leaderboard_id](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, leaderboard_id);
    });
}

bool SQLiteScoreRepository::deleteByUserId(int user_id) {
    std::string sql = R"(
        DELETE FROM scores
        WHERE user_id = ?;
    )";

    return executePreparedStatement(sql, [user_id](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, user_id);
    });
}

} // namespace repository