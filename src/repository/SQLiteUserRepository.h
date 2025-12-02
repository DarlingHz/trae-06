#ifndef SQLITE_USER_REPOSITORY_H
#define SQLITE_USER_REPOSITORY_H

#include "SQLiteBaseRepository.h"
#include "UserRepository.h"

namespace repository {

// SQLiteUserRepository类，实现了UserRepository接口的SQLite版本
class SQLiteUserRepository : public repository::SQLiteBaseRepository<model::User>, public UserRepository {
public:
    explicit SQLiteUserRepository(const std::string& db_path);
    virtual ~SQLiteUserRepository() override = default;

    // 禁止拷贝构造和赋值操作
    SQLiteUserRepository(const SQLiteUserRepository&) = delete;
    SQLiteUserRepository& operator=(const SQLiteUserRepository&) = delete;

    // 允许移动构造和赋值操作
    SQLiteUserRepository(SQLiteUserRepository&&) noexcept = default;
    SQLiteUserRepository& operator=(SQLiteUserRepository&&) noexcept = default;

    // 从SQLiteBaseRepository继承的纯虚方法的实现
    std::string getTableName() const override;
    std::string getCreateTableSql() const override;
    std::string getInsertSql() const override;
    std::string getSelectByIdSql() const override;
    std::string getSelectAllSql() const override;
    std::string getUpdateSql() const override;
    std::string getDeleteByIdSql() const override;
    model::User fromRow(sqlite3_stmt* stmt) const override;
    void bindValues(sqlite3_stmt* stmt, const model::User& entity) const override;

    // 从UserRepository接口继承的方法的实现
    std::optional<model::User> findByUsername(const std::string& username) override;
    bool existsByUsername(const std::string& username) override;
};

} // namespace repository

#endif // SQLITE_USER_REPOSITORY_H