#ifndef SQLITE_BASE_REPOSITORY_H
#define SQLITE_BASE_REPOSITORY_H

#include "BaseRepository.h"
#include <sqlite3.h>
#include <string>
#include <functional>
#include <memory>
#include <utility>
#include <forward_list>
#include <type_traits>

namespace repository {










// SQLiteBaseRepository类，实现了BaseRepository接口的通用方法
// 其他SQLiteRepository类应该继承自这个类和对应的接口
template <typename T>
class SQLiteBaseRepository : public virtual BaseRepository<T> {
public:
    SQLiteBaseRepository(const ::std::string& db_path);
    virtual ~SQLiteBaseRepository();

    // 禁止拷贝构造和赋值操作
    SQLiteBaseRepository(const SQLiteBaseRepository&) = delete;
    SQLiteBaseRepository& operator=(const SQLiteBaseRepository&) = delete;

    // 允许移动构造和赋值操作
    SQLiteBaseRepository(SQLiteBaseRepository&&) noexcept = default;
    SQLiteBaseRepository& operator=(SQLiteBaseRepository&&) noexcept = default;

    // 初始化方法，用于创建表
    bool initialize();

    // BaseRepository接口方法的实现
    int create(const T& entity) override;
    ::std::optional<T> findById(int id) override;
    ::std::vector<T> findAll() override;
    bool update(const T& entity) override;
    bool deleteById(int id) override;

protected:
    // 纯虚方法，需要子类实现
    virtual ::std::string getTableName() const = 0;
    virtual ::std::string getCreateTableSql() const = 0;
    virtual ::std::string getInsertSql() const = 0;
    virtual ::std::string getSelectByIdSql() const = 0;
    virtual ::std::string getSelectAllSql() const = 0;
    virtual ::std::string getUpdateSql() const = 0;
    virtual ::std::string getDeleteByIdSql() const = 0;
    virtual T fromRow(sqlite3_stmt* stmt) const = 0;
    virtual void bindValues(sqlite3_stmt* stmt, const T& entity) const = 0;

    // 执行SQL语句的辅助方法
    bool executeSql(const ::std::string& sql);
    bool executeSqlWithCallback(const ::std::string& sql, const ::std::function<void(sqlite3_stmt*)>& callback);
    bool executePreparedStatement(const ::std::string& sql, const ::std::function<void(sqlite3_stmt*)>& bind_callback);
    bool executePreparedStatement(const ::std::string& sql, const ::std::function<void(sqlite3_stmt*)>& bind_callback, const ::std::function<void(sqlite3_stmt*)>& step_callback);

    // 数据库连接
    sqlite3* db_ = nullptr;
};

} // namespace repository

// 模板类的实现需要放在头文件中
#include "SQLiteBaseRepository.tpp"

#endif // SQLITE_BASE_REPOSITORY_H