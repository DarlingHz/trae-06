#pragma once

#include <sqlite3.h>
#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <queue>

namespace chat_archive {

// 数据库连接池类
class DatabasePool {
public:
    static DatabasePool& get() {
        static DatabasePool instance;
        return instance;
    }
    
    bool init(const std::string& db_path, int pool_size = 5);
    
    // 获取数据库连接
    std::shared_ptr<sqlite3> get_connection();
    
    // 释放数据库连接
    void release_connection(std::shared_ptr<sqlite3> conn);
    
private:
    DatabasePool() = default;
    ~DatabasePool() = default;
    
    DatabasePool(const DatabasePool&) = delete;
    DatabasePool& operator=(const DatabasePool&) = delete;
    
    // 初始化数据库表结构
    bool init_tables(sqlite3* conn);
    
    std::string db_path_;
    int pool_size_ = 5;
    
    std::mutex mutex_;
    std::queue<std::shared_ptr<sqlite3>> connections_;
};

// 数据库事务类
class DatabaseTransaction {
public:
    explicit DatabaseTransaction(std::shared_ptr<sqlite3> conn);
    ~DatabaseTransaction();
    
    bool commit();
    bool rollback();
    
private:
    std::shared_ptr<sqlite3> conn_;
    bool committed_ = false;
    bool rolled_back_ = false;
};

// 数据库查询结果类
class DatabaseResult {
public:
    DatabaseResult();
    ~DatabaseResult();
    
    bool next();
    
    // 获取列值
    int get_int(int column) const;
    int64_t get_int64(int column) const;
    double get_double(int column);
    std::string get_string(int column) const;
    const void* get_blob(int column, int* size);
    
    // 获取列数
    int get_column_count() const;
    
    // 获取列名
    std::string get_column_name(int column) const;
    
    // 内部使用
    void set_stmt(sqlite3_stmt* stmt);
    sqlite3_stmt* get_stmt() const;
    
private:
    sqlite3_stmt* stmt_ = nullptr;
};

// 数据库查询类
class DatabaseQuery {
public:
    explicit DatabaseQuery(std::shared_ptr<sqlite3> conn);
    ~DatabaseQuery();
    
    bool prepare(const std::string& sql);
    
    // 绑定参数
    bool bind_int(int index, int value);
    bool bind_int64(int index, int64_t value);
    bool bind_double(int index, double value);
    bool bind_string(int index, const std::string& value);
    bool bind_blob(int index, const void* data, int size);
    bool bind_null(int index);
    
    // 执行查询
    bool execute();
    
    // 获取结果
    DatabaseResult get_result();
    
    // 获取受影响的行数
    int64_t get_affected_rows() const;
    
    // 获取最后插入的ID
    int64_t get_last_insert_rowid() const;
    
private:
    std::shared_ptr<sqlite3> conn_;
    sqlite3_stmt* stmt_ = nullptr;
};

} // namespace chat_archive