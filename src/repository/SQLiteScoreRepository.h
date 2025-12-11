#ifndef SQLITE_SCORE_REPOSITORY_H
#define SQLITE_SCORE_REPOSITORY_H

#include "SQLiteBaseRepository.h"
#include "ScoreRepository.h"

namespace repository {

// SQLiteScoreRepository类，实现了ScoreRepository接口的SQLite版本
class SQLiteScoreRepository : public repository::SQLiteBaseRepository<model::Score>, public ScoreRepository {
public:
    explicit SQLiteScoreRepository(const std::string& db_path);
    virtual ~SQLiteScoreRepository() override = default;

    // 禁止拷贝构造和赋值操作
    SQLiteScoreRepository(const SQLiteScoreRepository&) = delete;
    SQLiteScoreRepository& operator=(const SQLiteScoreRepository&) = delete;

    // 允许移动构造和赋值操作
    SQLiteScoreRepository(SQLiteScoreRepository&&) noexcept = default;
    SQLiteScoreRepository& operator=(SQLiteScoreRepository&&) noexcept = default;

    // 从SQLiteBaseRepository继承的纯虚方法的实现
    std::string getTableName() const override;
    std::string getCreateTableSql() const override;
    std::string getInsertSql() const override;
    std::string getSelectByIdSql() const override;
    std::string getSelectAllSql() const override;
    std::string getUpdateSql() const override;
    std::string getDeleteByIdSql() const override;
    model::Score fromRow(sqlite3_stmt* stmt) const override;
    void bindValues(sqlite3_stmt* stmt, const model::Score& entity) const override;

    // 从ScoreRepository接口继承的方法的实现
    std::vector<model::Score> findByLeaderboardId(int leaderboard_id, int limit = 0) override;
    std::vector<model::Score> findByUserId(int user_id, int limit = 0) override;
    std::vector<model::Score> findByLeaderboardIdAndUserId(int leaderboard_id, int user_id, int limit = 0) override;
    std::vector<model::Score> findTopByLeaderboardId(int leaderboard_id, int limit) override;
    std::optional<model::Score> findBestByLeaderboardIdAndUserId(int leaderboard_id, int user_id) override;
    std::optional<int> findRankByLeaderboardIdAndUserId(int leaderboard_id, int user_id) override;
    bool deleteByLeaderboardId(int leaderboard_id) override;
    bool deleteByUserId(int user_id) override;
};

} // namespace repository

#endif // SQLITE_SCORE_REPOSITORY_H