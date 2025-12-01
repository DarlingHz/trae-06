#include "dao/SleepSessionDao.h"
#include <iostream>
#include <sqlite3.h>
#include <algorithm>
#include "util/Utils.h"

namespace dao {

SleepSessionDao::SleepSessionDao(sqlite3* db) : db_(db) {
}

SleepSessionDao::~SleepSessionDao() {
}

bool SleepSessionDao::createTable() const {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS sleep_sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            start_time TEXT NOT NULL,
            end_time TEXT NOT NULL,
            quality INTEGER NOT NULL CHECK(quality >= 0 AND quality <= 10),
            tags TEXT NOT NULL DEFAULT '',
            note TEXT NOT NULL DEFAULT '',
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        );
    )";

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to create sleep_sessions table: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    // 创建索引以提高查询性能
    const char* index_sql = R"(
        CREATE INDEX IF NOT EXISTS idx_sleep_sessions_user_id_start_time
        ON sleep_sessions (user_id, start_time DESC);
    )";

    rc = sqlite3_exec(db_, index_sql, nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to create index for sleep_sessions: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool SleepSessionDao::insertSleepSession(const model::SleepSession& session) const {
    // 将标签数组转换为逗号分隔的字符串
    std::string tags_str;
    for (size_t i = 0; i < session.tags.size(); i++) {
        if (i > 0) {
            tags_str += ",";
        }
        tags_str += session.tags[i];
    }

    const char* sql = R"(
        INSERT INTO sleep_sessions (user_id, start_time, end_time, quality, tags, note)
        VALUES (?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare insert sleep session statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, session.user_id);
    sqlite3_bind_text(stmt, 2, session.start_time.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, session.end_time.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, session.quality);
    sqlite3_bind_text(stmt, 5, tags_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, session.note.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert sleep session: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

model::SleepSession SleepSessionDao::findSleepSessionById(int id) const {
    model::SleepSession session;
    session.id = -1;

    const char* sql = R"(
        SELECT id, user_id, start_time, end_time, quality, tags, note
        FROM sleep_sessions
        WHERE id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare find sleep session by id statement: " << sqlite3_errmsg(db_) << std::endl;
        return session;
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        session.id = sqlite3_column_int(stmt, 0);
        session.user_id = sqlite3_column_int(stmt, 1);
        session.start_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        session.end_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        session.quality = sqlite3_column_int(stmt, 4);

        // 将逗号分隔的标签字符串转换为数组
        std::string tags_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        if (!tags_str.empty()) {
            session.tags = util::string::split(tags_str, ",");
        }

        session.note = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    }

    sqlite3_finalize(stmt);
    return session;
}

std::vector<model::SleepSession> SleepSessionDao::findSleepSessionsByUserIdAndDateRange(
    int user_id, const std::string& start_date, const std::string& end_date,
    int page, int page_size) const {

    std::vector<model::SleepSession> sessions;

    // 构建查询SQL
    std::string sql = R"(
        SELECT id, user_id, start_time, end_time, quality, tags, note
        FROM sleep_sessions
        WHERE user_id = ? AND start_time >= ? AND start_time <= ?
        ORDER BY start_time DESC
        LIMIT ? OFFSET ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare find sleep sessions by user id and date range statement: " << sqlite3_errmsg(db_) << std::endl;
        return sessions;
    }

    // 绑定参数
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, (start_date + "T00:00:00").c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, (end_date + "T23:59:59").c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, page_size);
    sqlite3_bind_int(stmt, 5, (page - 1) * page_size);

    // 执行查询
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        model::SleepSession session;
        session.id = sqlite3_column_int(stmt, 0);
        session.user_id = sqlite3_column_int(stmt, 1);
        session.start_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        session.end_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        session.quality = sqlite3_column_int(stmt, 4);

        // 将逗号分隔的标签字符串转换为数组
        std::string tags_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        if (!tags_str.empty()) {
            session.tags = util::string::split(tags_str, ",");
        }

        session.note = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

        sessions.push_back(session);
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute find sleep sessions by user id and date range statement: " << sqlite3_errmsg(db_) << std::endl;
    }

    sqlite3_finalize(stmt);
    return sessions;
}

bool SleepSessionDao::updateSleepSession(const model::SleepSession& session) const {
    // 将标签数组转换为逗号分隔的字符串
    std::string tags_str;
    for (size_t i = 0; i < session.tags.size(); i++) {
        if (i > 0) {
            tags_str += ",";
        }
        tags_str += session.tags[i];
    }

    const char* sql = R"(
        UPDATE sleep_sessions
        SET start_time = ?, end_time = ?, quality = ?, tags = ?, note = ?
        WHERE id = ? AND user_id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare update sleep session statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, session.start_time.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, session.end_time.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, session.quality);
    sqlite3_bind_text(stmt, 4, tags_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, session.note.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, session.id);
    sqlite3_bind_int(stmt, 7, session.user_id);

    // 执行更新
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to update sleep session: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 检查是否有行被更新
    int changes = sqlite3_changes(db_);
    if (changes == 0) {
        std::cerr << "No sleep session was updated. Either the session does not exist or you do not have permission to update it." << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool SleepSessionDao::deleteSleepSession(int id) const {
    const char* sql = R"(
        DELETE FROM sleep_sessions WHERE id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare delete sleep session statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    // 绑定参数
    sqlite3_bind_int(stmt, 1, id);

    // 执行删除
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to delete sleep session: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 检查是否有行被删除
    int changes = sqlite3_changes(db_);
    if (changes == 0) {
        std::cerr << "No sleep session was deleted. The session does not exist." << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

} // namespace dao
