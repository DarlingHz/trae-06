#ifndef SQLITE_LEADERBOARD_REPOSITORY_H
#define SQLITE_LEADERBOARD_REPOSITORY_H

#include "SQLiteBaseRepository.h"
#include "LeaderboardRepository.h"

namespace repository {

// SQLiteLeaderboardRepository类，实现了LeaderboardRepository接口的SQLite版本
class SQLiteLeaderboardRepository : public repository::SQLiteBaseRepository<model::Leaderboard>, public LeaderboardRepository {
public:
    explicit SQLiteLeaderboardRepository(const std::string& db_path);
    virtual ~SQLiteLeaderboardRepository() override = default;

    // 禁止拷贝构造和赋值操作
    SQLiteLeaderboardRepository(const SQLiteLeaderboardRepository&) = delete;
    SQLiteLeaderboardRepository& operator=(const SQLiteLeaderboardRepository&) = delete;

    // 允许移动构造和赋值操作
    SQLiteLeaderboardRepository(SQLiteLeaderboardRepository&&) noexcept = default;
    SQLiteLeaderboardRepository& operator=(SQLiteLeaderboardRepository&&) noexcept = default;

    // 从SQLiteBaseRepository继承的纯虚方法的实现
    std::string getTableName() const override;
    std::string getCreateTableSql() const override;
    std::string getInsertSql() const override;
    std::string getSelectByIdSql() const override;
    std::string getSelectAllSql() const override;
    std::string getUpdateSql() const override;
    std::string getDeleteByIdSql() const override;
    model::Leaderboard fromRow(sqlite3_stmt* stmt) const override;
    void bindValues(sqlite3_stmt* stmt, const model::Leaderboard& entity) const override;

    // 从LeaderboardRepository接口继承的方法的实现
    std::vector<model::Leaderboard> findByGameId(int game_id) override;
    std::optional<model::Leaderboard> findByGameIdAndName(int game_id, const std::string& name) override;
    std::vector<model::Leaderboard> findByGameIdAndRegion(int game_id, const std::string& region) override;
};

} // namespace repository

#endif // SQLITE_LEADERBOARD_REPOSITORY_H