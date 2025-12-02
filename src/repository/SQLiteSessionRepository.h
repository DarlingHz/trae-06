#ifndef SQLITE_SESSION_REPOSITORY_H
#define SQLITE_SESSION_REPOSITORY_H

#include "SQLiteBaseRepository.h"
#include "SessionRepository.h"

namespace repository {

// SQLiteSessionRepository类，实现了SessionRepository接口的SQLite版本
class SQLiteSessionRepository : public repository::SQLiteBaseRepository<model::Session>, public SessionRepository {
public:
    explicit SQLiteSessionRepository(const std::string& db_path);
    virtual ~SQLiteSessionRepository() override = default;

    // 禁止拷贝构造和赋值操作
    SQLiteSessionRepository(const SQLiteSessionRepository&) = delete;
    SQLiteSessionRepository& operator=(const SQLiteSessionRepository&) = delete;

    // 允许移动构造和赋值操作
    SQLiteSessionRepository(SQLiteSessionRepository&&) noexcept = default;
    SQLiteSessionRepository& operator=(SQLiteSessionRepository&&) noexcept = default;

    // 从SQLiteBaseRepository继承的方法
    using SQLiteBaseRepository<model::Session>::create;
    using SQLiteBaseRepository<model::Session>::findById;
    using SQLiteBaseRepository<model::Session>::findAll;
    using SQLiteBaseRepository<model::Session>::update;
    using SQLiteBaseRepository<model::Session>::deleteById;

    // 从SQLiteBaseRepository继承的纯虚方法的实现
    std::string getTableName() const override;
    std::string getCreateTableSql() const override;
    std::string getInsertSql() const override;
    std::string getSelectByIdSql() const override;
    std::string getSelectAllSql() const override;
    std::string getUpdateSql() const override;
    std::string getDeleteByIdSql() const override;
    model::Session fromRow(sqlite3_stmt* stmt) const override;
    void bindValues(sqlite3_stmt* stmt, const model::Session& entity) const override;

    // 从SessionRepository接口继承的方法的实现
    std::optional<model::Session> findByToken(const std::string& token) override;
    std::vector<model::Session> findByUserId(int user_id) override;
    bool deleteExpired() override;
    bool deleteByUserId(int user_id) override;
};

} // namespace repository

#endif // SQLITE_SESSION_REPOSITORY_H