#ifndef LEADERBOARD_REPOSITORY_H
#define LEADERBOARD_REPOSITORY_H

#include "BaseRepository.h"
#include "../model/Leaderboard.h"

namespace repository {

// LeaderboardRepository接口，定义排行榜相关的数据访问方法
class LeaderboardRepository : public virtual BaseRepository<model::Leaderboard> {
public:
    virtual ~LeaderboardRepository() = default;

    // 根据游戏ID查找排行榜
    virtual std::vector<model::Leaderboard> findByGameId(int game_id) = 0;

    // 根据游戏ID和名称查找排行榜
    virtual std::optional<model::Leaderboard> findByGameIdAndName(int game_id, const std::string& name) = 0;

    // 根据游戏ID和区域查找排行榜
    virtual std::vector<model::Leaderboard> findByGameIdAndRegion(int game_id, const std::string& region) = 0;
};

// 创建LeaderboardRepository实例的工厂函数
std::unique_ptr<LeaderboardRepository> createLeaderboardRepository(const std::string& db_path);

} // namespace repository

#endif // LEADERBOARD_REPOSITORY_H