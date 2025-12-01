#include "dao/UserSettingDao.h"
#include <iostream>
#include <sqlite3.h>

namespace dao {

UserSettingDao::UserSettingDao(sqlite3* db) : db_(db) {
}

UserSettingDao::~UserSettingDao() {
}

bool UserSettingDao::createTable() const {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS user_settings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL UNIQUE,
            goal_hours_per_day REAL NOT NULL DEFAULT 8.0,
            updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        );
    )";

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to create user_settings table: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool UserSettingDao::upsertUserSetting(const model::UserSetting& setting) const {
    const char* sql = R"(
        INSERT INTO user_settings (user_id, goal_hours_per_day, updated_at)
        VALUES (?, ?, ?)
        ON CONFLICT(user_id) DO UPDATE SET
            goal_hours_per_day = excluded.goal_hours_per_day,
            updated_at = excluded.updated_at;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare upsert user setting statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, setting.user_id);
    sqlite3_bind_double(stmt, 2, setting.goal_hours_per_day);
    sqlite3_bind_text(stmt, 3, setting.updated_at.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to upsert user setting: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

model::UserSetting UserSettingDao::findUserSettingByUserId(int user_id) const {
    model::UserSetting setting;
    setting.id = -1;

    const char* sql = R"(
        SELECT id, user_id, goal_hours_per_day, updated_at
        FROM user_settings
        WHERE user_id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare find user setting by user id statement: " << sqlite3_errmsg(db_) << std::endl;
        return setting;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        setting.id = sqlite3_column_int(stmt, 0);
        setting.user_id = sqlite3_column_int(stmt, 1);
        setting.goal_hours_per_day = sqlite3_column_double(stmt, 2);
        setting.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    }

    sqlite3_finalize(stmt);
    return setting;
}

} // namespace dao
