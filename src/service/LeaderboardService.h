#ifndef LEADERBOARD_SERVICE_H
#define LEADERBOARD_SERVICE_H

#include <string>
#include <optional>
#include <vector>
#include "../model/Leaderboard.h"
#include "../model/Score.h"
#include "../repository/LeaderboardRepository.h"
#include "../repository/ScoreRepository.h"
#include "GameService.h"

namespace service {

// LeaderboardService接口，定义排行榜相关的业务逻辑
class LeaderboardService {
public:
    virtual ~LeaderboardService() = default;

    // 构造函数，依赖于LeaderboardRepository和ScoreRepository接口
    explicit LeaderboardService(
        std::shared_ptr<repository::LeaderboardRepository> leaderboard_repository,
        std::shared_ptr<repository::ScoreRepository> score_repository)
        : leaderboard_repository_(std::move(leaderboard_repository)),
          score_repository_(std::move(score_repository)) {}

    // 创建新排行榜
    // 参数：game_id - 游戏ID
    //      name - 排行榜名称
    //      region - 区域（可选）
    //      score_rule - 分数规则
    // 返回：成功则返回创建的排行榜对象，失败则返回std::nullopt
    // 失败原因可能包括：游戏不存在、参数无效等
    std::optional<model::Leaderboard> createLeaderboard(
        int game_id,
        const std::string& name,
        const std::string& region,
        model::ScoreRule score_rule);

    // 根据ID查找排行榜
    // 参数：id - 排行榜ID
    // 返回：成功则返回排行榜对象，失败则返回std::nullopt
    std::optional<model::Leaderboard> findLeaderboardById(int id);

    // 根据游戏ID查找排行榜
    // 参数：game_id - 游戏ID
    // 返回：成功则返回排行榜列表，失败则返回空列表
    std::vector<model::Leaderboard> findLeaderboardsByGameId(int game_id);

    // 提交成绩
    // 参数：leaderboard_id - 排行榜ID
    //      user_id - 用户ID
    //      score - 成绩
    // 返回：成功则返回true，失败则返回false
    bool submitScore(int leaderboard_id, int user_id, int score);

    // 获取用户排名
    // 参数：leaderboard_id - 排行榜ID
    //      user_id - 用户ID
    // 返回：成功则返回用户排名，失败则返回-1
    int getUserRank(int leaderboard_id, int user_id);

    // 获取用户分数
    // 参数：leaderboard_id - 排行榜ID
    //      user_id - 用户ID
    // 返回：成功则返回用户分数，失败则返回-1
    int getUserScore(int leaderboard_id, int user_id);

    // 获取Top N排名
    // 参数：leaderboard_id - 排行榜ID
    //      limit - 返回的排名数量
    // 返回：成功则返回Top N排名的成绩列表，失败则返回空列表
    std::vector<model::Score> getTopScores(int leaderboard_id, int limit);

    // 根据游戏ID和名称查找排行榜
    // 参数：game_id - 游戏ID
    //      name - 排行榜名称
    // 返回：成功则返回排行榜对象，失败则返回std::nullopt
    std::optional<model::Leaderboard> findLeaderboardByGameIdAndName(int game_id, const std::string& name);

    // 根据游戏ID和区域查找排行榜
    // 参数：game_id - 游戏ID
    //      region - 区域
    // 返回：成功则返回排行榜列表，失败则返回空列表
    std::vector<model::Leaderboard> findLeaderboardsByGameIdAndRegion(int game_id, const std::string& region);

    // 获取所有排行榜列表
    // 返回：排行榜列表
    std::vector<model::Leaderboard> getAllLeaderboards();

private:
    // 依赖注入的LeaderboardRepository接口
    std::shared_ptr<repository::LeaderboardRepository> leaderboard_repository_;
    // 依赖注入的ScoreRepository接口
    std::shared_ptr<repository::ScoreRepository> score_repository_;

    // 依赖注入的GameService接口
    std::shared_ptr<GameService> game_service_;
};

} // namespace service

#endif // LEADERBOARD_SERVICE_H