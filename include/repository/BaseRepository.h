#pragma once

#include <sqlite3.h>
#include <string>
#include <stdexcept>

namespace repository {

class DatabaseException : public std::runtime_error {
public:
    explicit DatabaseException(const std::string& message) : std::runtime_error(message) {}
};

class BaseRepository {
public:
    explicit BaseRepository(const std::string& db_path);
    ~BaseRepository();

    // 禁止拷贝构造和赋值
    BaseRepository(const BaseRepository&) = delete;
    BaseRepository& operator=(const BaseRepository&) = delete;

    // 允许移动构造和赋值
    BaseRepository(BaseRepository&& other) noexcept;
    BaseRepository& operator=(BaseRepository&& other) noexcept;

protected:
    sqlite3* db_ = nullptr;

    // 执行 SQL 语句（不返回结果）
    void execute(const std::string& sql);

    // 执行 SQL 查询（返回结果）
    template <typename Callback>
    void query(const std::string& sql, Callback callback) {
        char* err_msg = nullptr;
        int rc = sqlite3_exec(db_, sql.c_str(), callback, nullptr, &err_msg);

        if (rc != SQLITE_OK) {
            std::string error_msg = "SQL error: " + std::string(err_msg);
            sqlite3_free(err_msg);
            throw DatabaseException(error_msg);
        }
    }
};

} // namespace repository