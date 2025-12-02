#ifndef GAME_REPOSITORY_H
#define GAME_REPOSITORY_H

#include "BaseRepository.h"
#include "../model/Game.h"

namespace repository {

// GameRepository接口，定义游戏相关的数据库操作
class GameRepository : public virtual BaseRepository<model::Game> {
public:
    virtual ~GameRepository() = default;

    // 根据游戏键查找游戏
    virtual std::optional<model::Game> findByGameKey(const std::string& game_key) = 0;

    // 检查游戏键是否存在
    virtual bool existsByGameKey(const std::string& game_key) = 0;
};

// 创建GameRepository实例的工厂函数
std::unique_ptr<GameRepository> createGameRepository(const std::string& db_path);

} // namespace repository

#endif // GAME_REPOSITORY_H