#ifndef PARKING_DATABASE_H
#define PARKING_DATABASE_H

#include <sqlite3.h>
#include <string>
#include <memory>
#include <stdexcept>

class Database {
private:
    sqlite3* db_ = nullptr;

    // 禁用拷贝构造和赋值
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

public:
    Database(const std::string& db_path);
    ~Database();

    // 移动构造和赋值
    Database(Database&& other) noexcept;
    Database& operator=(Database&& other) noexcept;

    // 执行SQL语句（无返回结果）
    void execute(const std::string& sql);

    // 执行查询并返回结果回调
    template<typename Callback>
    void query(const std::string& sql, Callback callback) {
        char* err_msg = nullptr;
        int rc = sqlite3_exec(db_, sql.c_str(),
            [](void* data, int argc, char** argv, char** azColName) -> int {
                auto* callback = static_cast<Callback*>(data);
                return (*callback)(argc, argv, azColName);
            },
            &callback, &err_msg);

        if (rc != SQLITE_OK) {
            std::string err = err_msg;
            sqlite3_free(err_msg);
            throw std::runtime_error("Database query failed: " + err + " (SQL: " + sql + ")");
        }
    }

    // 获取最后插入的ID
    int64_t last_insert_rowid() const;

    // 开始事务
    void begin_transaction();

    // 提交事务
    void commit();

    // 回滚事务
    void rollback();

    // 获取原生数据库指针（谨慎使用）
    sqlite3* get_native_handle() const { return db_; }
};

// 全局数据库实例管理
namespace db {
    extern std::unique_ptr<Database> instance;

    void init(const std::string& db_path);
    void shutdown();
    Database& get();
}

#endif // PARKING_DATABASE_H
