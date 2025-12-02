#ifndef SQLITE_GAME_REPOSITORY_H
#define SQLITE_GAME_REPOSITORY_H

#include "SQLiteBaseRepository.h"
#include "GameRepository.h"

namespace repository {

// SQLiteGameRepository类，实现了GameRepository接口和SQLiteBaseRepository抽象类
class SQLiteGameRepository : public SQLiteBaseRepository<model::Game>, public GameRepository {
public:
    explicit SQLiteGameRepository(const std::string& db_path);
    virtual ~SQLiteGameRepository() override = default;

    // 禁止拷贝构造和赋值操作
    SQLiteGameRepository(const SQLiteGameRepository&) = delete;
    SQLiteGameRepository& operator=(const SQLiteGameRepository&) = delete;

    // 允许移动构造和赋值操作
    SQLiteGameRepository(SQLiteGameRepository&&) noexcept = default;
    SQLiteGameRepository& operator=(SQLiteGameRepository&&) noexcept = default;

    // 从GameRepository接口继承的方法的实现
    std::optional<model::Game> findByGameKey(const std::string& game_key) override;
    bool existsByGameKey(const std::string& game_key) override;

private:
    // 从SQLiteBaseRepository继承的纯虚方法的实现
    std::string getTableName() const override;
    std::string getCreateTableSql() const override;
    std::string getInsertSql() const override;
    std::string getSelectByIdSql() const override;
    std::string getSelectAllSql() const override;
    std::string getUpdateSql() const override;
    std::string getDeleteByIdSql() const override;
    model::Game fromRow(sqlite3_stmt* stmt) const override;
    void bindValues(sqlite3_stmt* stmt, const model::Game& entity) const override;
};

} // namespace repository

#endif // SQLITE_GAME_REPOSITORY_H