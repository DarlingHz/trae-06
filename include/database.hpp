#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <sqlite3.h>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Database {
public:
    // 单例模式
    static Database& getInstance() {
        static Database instance;
        return instance;
    }

    // 禁止拷贝和移动
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&&) = delete;
    Database& operator=(Database&&) = delete;

    // 初始化数据库
    bool init(const std::string& db_path);

    // 获取数据库连接
    sqlite3* getConnection();

    // 执行 SQL 语句（无返回结果）
    bool execute(const std::string& sql);

    // 执行 SQL 语句（有返回结果）
    bool query(const std::string& sql, std::function<bool(int, char**, char**)> callback);

    // 开始事务
    bool beginTransaction();

    // 提交事务
    bool commitTransaction();

    // 回滚事务
    bool rollbackTransaction();

private:
    // 私有构造函数
    Database() : db_(nullptr) {}

    // 析构函数
    ~Database() {
        if (db_ != nullptr) {
            sqlite3_close(db_);
        }
    }

    // 创建表结构
    bool createTables();

    sqlite3* db_;
};

#endif // DATABASE_HPP
