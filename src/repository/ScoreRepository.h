#ifndef SCORE_REPOSITORY_H
#define SCORE_REPOSITORY_H

#include "BaseRepository.h"
#include "../model/Score.h"

namespace repository {

// ScoreRepository接口，定义成绩相关的数据访问方法
class ScoreRepository : public virtual BaseRepository<model::Score> {
public:
    virtual ~ScoreRepository() = default;

    // 根据排行榜ID查找成绩
    virtual std::vector<model::Score> findByLeaderboardId(int leaderboard_id, int limit = 0) = 0;

    // 根据用户ID查找成绩
    virtual std::vector<model::Score> findByUserId(int user_id, int limit = 0) = 0;

    // 根据排行榜ID和用户ID查找成绩
    virtual std::vector<model::Score> findByLeaderboardIdAndUserId(int leaderboard_id, int user_id, int limit = 0) = 0;

    // 获取某个排行榜的前N名成绩
    virtual std::vector<model::Score> findTopByLeaderboardId(int leaderboard_id, int limit) = 0;

    // 获取某个用户在某个排行榜的最好成绩
    virtual std::optional<model::Score> findBestByLeaderboardIdAndUserId(int leaderboard_id, int user_id) = 0;

    // 获取某个用户在某个排行榜的当前名次
    virtual std::optional<int> findRankByLeaderboardIdAndUserId(int leaderboard_id, int user_id) = 0;

    // 删除某个排行榜的所有成绩
    virtual bool deleteByLeaderboardId(int leaderboard_id) = 0;

    // 删除某个用户的所有成绩
    virtual bool deleteByUserId(int user_id) = 0;
};

// 创建ScoreRepository实例的工厂函数
std::unique_ptr<ScoreRepository> createScoreRepository(const std::string& db_path);

} // namespace repository

#endif // SCORE_REPOSITORY_H