#ifndef SCORE_SERVICE_H
#define SCORE_SERVICE_H

#include <string>
#include <optional>
#include <vector>
#include <nlohmann/json.hpp>
#include "../model/Score.h"
#include "../repository/ScoreRepository.h"
#include "LeaderboardService.h"
#include "UserService.h"

namespace service {

// ScoreService接口，定义成绩相关的业务逻辑
class ScoreService {
public:
    virtual ~ScoreService() = default;

    // 构造函数，依赖于ScoreRepository接口、LeaderboardService接口和UserService接口
    explicit ScoreService(
        std::shared_ptr<repository::ScoreRepository> score_repository,
        std::shared_ptr<LeaderboardService> leaderboard_service,
        std::shared_ptr<UserService> user_service)
        : score_repository_(std::move(score_repository)),
          leaderboard_service_(std::move(leaderboard_service)),
          user_service_(std::move(user_service)) {}

    // 提交成绩
    // 参数：leaderboard_id - 排行榜ID
    //      user_id - 用户ID
    //      score - 成绩
    //      extra_data - 额外数据（可选）
    // 返回：成功则返回提交的成绩对象，失败则返回std::nullopt
    // 失败原因可能包括：排行榜不存在、用户不存在、成绩无效等
    std::optional<model::Score> submitScore(
        int leaderboard_id,
        int user_id,
        int score,
        const nlohmann::json& extra_data = nlohmann::json());

    // 根据ID查找成绩
    // 参数：id - 成绩ID
    // 返回：成功则返回成绩对象，失败则返回std::nullopt
    std::optional<model::Score> findScoreById(int id);

    // 根据排行榜ID查找成绩
    // 参数：leaderboard_id - 排行榜ID
    // 返回：成功则返回成绩列表，失败则返回空列表
    std::vector<model::Score> findScoresByLeaderboardId(int leaderboard_id);

    // 根据用户ID查找成绩
    // 参数：user_id - 用户ID
    // 返回：成功则返回成绩列表，失败则返回空列表
    std::vector<model::Score> findScoresByUserId(int user_id);

    // 根据排行榜ID查找前N名成绩
    // 参数：leaderboard_id - 排行榜ID
    //      limit - 限制数量
    // 返回：成功则返回成绩列表，失败则返回空列表
    std::vector<model::Score> findTopScoresByLeaderboardId(int leaderboard_id, int limit = 10);

    // 根据排行榜ID和用户ID查找最佳成绩
    // 参数：leaderboard_id - 排行榜ID
    //      user_id - 用户ID
    // 返回：成功则返回最佳成绩对象，失败则返回std::nullopt
    std::optional<model::Score> findBestScoreByLeaderboardIdAndUserId(int leaderboard_id, int user_id);

    // 根据排行榜ID和用户ID查找排名
    // 参数：leaderboard_id - 排行榜ID
    //      user_id - 用户ID
    // 返回：成功则返回排名（从1开始），失败则返回0
    int findRankByLeaderboardIdAndUserId(int leaderboard_id, int user_id);

    // 根据排行榜ID删除成绩
    // 参数：leaderboard_id - 排行榜ID
    // 返回：成功则返回true，失败则返回false
    bool deleteScoresByLeaderboardId(int leaderboard_id);

    // 根据用户ID删除成绩
    // 参数：user_id - 用户ID
    // 返回：成功则返回true，失败则返回false
    bool deleteScoresByUserId(int user_id);

private:
    // 依赖注入的ScoreRepository接口
    std::shared_ptr<repository::ScoreRepository> score_repository_;

    // 依赖注入的LeaderboardService接口
    std::shared_ptr<LeaderboardService> leaderboard_service_;

    // 依赖注入的UserService接口
    std::shared_ptr<UserService> user_service_;
};

} // namespace service

#endif // SCORE_SERVICE_H