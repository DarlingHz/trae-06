#pragma once

#include <string>
#include <sqlite3.h>
#include <mutex>
#include <vector>
#include <memory>
#include "logging/Logging.h"

namespace pet_hospital {

class Database {
public:
    Database() = default;
    ~Database();

    // 初始化数据库连接
    bool init(const std::string& connection_string);

    // 执行 SQL 查询（返回结果集）
    bool execute_query(const std::string& sql, std::vector<std::vector<std::string>>& result);
    // 执行带参数的 SQL 查询（返回结果集）
    bool execute_query(const std::string& sql, std::vector<std::vector<std::string>>& result, const std::vector<std::string>& params);

    // 执行 SQL 语句（不返回结果集）
    bool execute_statement(const std::string& sql, int& affected_rows);
    // 执行带参数的 SQL 语句（不返回结果集）
    bool execute_statement(const std::string& sql, int& affected_rows, const std::vector<std::string>& params);

    // 开始事务
    bool begin_transaction();

    // 提交事务
    bool commit_transaction();

    // 回滚事务
    bool rollback_transaction();

    // 关闭数据库连接
    void close();

    // 检查数据库连接是否有效
    bool is_connected() const { return db_ != nullptr; }

private:
    // 初始化数据库表
    bool init_tables();

    // 执行 SQL 语句的内部方法
    bool execute_sql(const std::string& sql, std::vector<std::vector<std::string>>* result = nullptr, int* affected_rows = nullptr);

private:
    sqlite3* db_ = nullptr;
    std::mutex db_mutex_;
};

// 全局数据库对象声明
extern std::unique_ptr<Database> g_database;

} // namespace pet_hospital
