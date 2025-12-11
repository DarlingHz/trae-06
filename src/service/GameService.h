#ifndef GAME_SERVICE_H
#define GAME_SERVICE_H

#include <string>
#include <optional>
#include <vector>
#include "../model/Game.h"
#include "../repository/GameRepository.h"

namespace service {

// GameService接口，定义游戏相关的业务逻辑
class GameService {
public:
    virtual ~GameService() = default;

    // 构造函数，依赖于GameRepository接口
    explicit GameService(std::unique_ptr<repository::GameRepository> game_repository)
        : game_repository_(std::move(game_repository)) {}

    // 创建新游戏
    // 参数：game_key - 游戏唯一标识
    //      name - 游戏名称
    // 返回：成功则返回创建的游戏对象，失败则返回std::nullopt
    // 失败原因可能包括：game_key已存在、参数无效等
    std::optional<model::Game> createGame(const std::string& game_key, const std::string& name);

    // 根据ID查找游戏
    // 参数：id - 游戏ID
    // 返回：成功则返回游戏对象，失败则返回std::nullopt
    std::optional<model::Game> findGameById(int id);

    // 根据game_key查找游戏
    // 参数：game_key - 游戏唯一标识
    // 返回：成功则返回游戏对象，失败则返回std::nullopt
    std::optional<model::Game> findGameByGameKey(const std::string& game_key);

    // 获取所有游戏列表
    // 返回：游戏列表
    std::vector<model::Game> getAllGames();

    // 检查game_key是否存在
    // 参数：game_key - 游戏唯一标识
    // 返回：存在则返回true，否则返回false
    bool existsByGameKey(const std::string& game_key);

private:
    // 依赖注入的GameRepository接口
    std::unique_ptr<repository::GameRepository> game_repository_;
};

} // namespace service

#endif // GAME_SERVICE_H