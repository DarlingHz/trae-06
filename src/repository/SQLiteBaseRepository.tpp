#include "SQLiteBaseRepository.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

template <typename T>
repository::SQLiteBaseRepository<T>::SQLiteBaseRepository(const ::std::string& db_path) {
    // 打开数据库连接
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        spdlog::error("Can't open database: {}", sqlite3_errmsg(db_));
        sqlite3_close(db_);
        throw ::std::runtime_error("Failed to open database");
    }

    spdlog::info("Successfully opened database");
}

template <typename T>
bool repository::SQLiteBaseRepository<T>::initialize() {
    // 创建表
    if (!executeSql(getCreateTableSql())) {
        spdlog::error("Failed to create table: {}", getTableName());
        return false;
    }

    spdlog::info("Successfully created table: {}", getTableName());
    return true;
}

template <typename T>
repository::SQLiteBaseRepository<T>::~SQLiteBaseRepository<T>() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

template <typename T>
int repository::SQLiteBaseRepository<T>::create(const T& entity) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, getInsertSql().c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        spdlog::error("Can't prepare statement: {}", sqlite3_errmsg(db_));
        return -1;
    }

    // 绑定参数
    bindValues(stmt, entity);

    // 执行语句
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        spdlog::error("SQL error: {}", sqlite3_errmsg(db_));
        sqlite3_finalize(stmt);
        return -1;
    }

    // 获取数据库自动生成的id值
    int id = sqlite3_last_insert_rowid(db_);

    sqlite3_finalize(stmt);
    return id;
}

template <typename T>
::std::optional<T> repository::SQLiteBaseRepository<T>::findById(int id) {
    ::std::optional<T> result;
    executePreparedStatement(getSelectByIdSql(), [id](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, id);
    }, [this, &result](sqlite3_stmt* stmt) {
        result = fromRow(stmt);
    });
    return result;
}

template <typename T>
::std::vector<T> repository::SQLiteBaseRepository<T>::findAll() {
    ::std::vector<T> result;
    executeSqlWithCallback(getSelectAllSql(), [this, &result](sqlite3_stmt* stmt) {
        result.push_back(fromRow(stmt));
    });
    return result;
}

template <typename T>
bool repository::SQLiteBaseRepository<T>::update(const T& entity) {
    return executePreparedStatement(getUpdateSql(), [this, &entity](sqlite3_stmt* stmt) {
        bindValues(stmt, entity);
    });
}

template <typename T>
bool repository::SQLiteBaseRepository<T>::deleteById(int id) {
    return executePreparedStatement(getDeleteByIdSql(), [id](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, id);
    });
}

template <typename T>
bool repository::SQLiteBaseRepository<T>::executeSql(const ::std::string& sql) {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        spdlog::error("SQL error: {}", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

template <typename T>
bool repository::SQLiteBaseRepository<T>::executeSqlWithCallback(const ::std::string& sql, const ::std::function<void(sqlite3_stmt*)>& callback) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        spdlog::error("Can't prepare statement: {}", sqlite3_errmsg(db_));
        return false;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        callback(stmt);
    }

    if (rc != SQLITE_DONE) {
        spdlog::error("SQL error: {}", sqlite3_errmsg(db_));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

template <typename T>
bool repository::SQLiteBaseRepository<T>::executePreparedStatement(const ::std::string& sql, const ::std::function<void(sqlite3_stmt*)>& bind_callback) {
    return executePreparedStatement(sql, bind_callback, nullptr);
}

template <typename T>
bool repository::SQLiteBaseRepository<T>::executePreparedStatement(const ::std::string& sql, const ::std::function<void(sqlite3_stmt*)>& bind_callback, const ::std::function<void(sqlite3_stmt*)>& step_callback) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        spdlog::error("Can't prepare statement: {}", sqlite3_errmsg(db_));
        return false;
    }

    // 绑定参数
    if (bind_callback) {
        bind_callback(stmt);
    }

    // 执行语句并处理所有行
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (step_callback) {
            step_callback(stmt);
        }
    }

    if (rc != SQLITE_DONE) {
        spdlog::error("SQL error: {}", sqlite3_errmsg(db_));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

