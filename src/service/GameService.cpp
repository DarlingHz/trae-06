#include "GameService.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <chrono>

namespace service {

// 创建新游戏
std::optional<model::Game> GameService::createGame(const std::string& game_key, const std::string& name) {
    try {
        // 业务逻辑验证
        if (game_key.empty()) {
            spdlog::error("Game key cannot be empty");
            return std::nullopt;
        }

        if (name.empty()) {
            spdlog::error("Game name cannot be empty");
            return std::nullopt;
        }

        if (game_key.length() < 3 || game_key.length() > 20) {
            spdlog::error("Game key must be between 3 and 20 characters");
            return std::nullopt;
        }

        if (name.length() < 1 || name.length() > 50) {
            spdlog::error("Game name must be between 1 and 50 characters");
            return std::nullopt;
        }

        // 检查game_key是否已存在
        if (game_repository_->existsByGameKey(game_key)) {
            spdlog::error("Game key already exists: {}", game_key);
            return std::nullopt;
        }

        // 创建游戏对象
        model::Game game;
        game.setGameKey(game_key);
        game.setName(name);
        game.setCreatedAt(std::chrono::system_clock::now());

        // 保存游戏到数据库
        int game_id = game_repository_->create(game);
        if (game_id <= 0) {
            spdlog::error("Failed to create game: {}", name);
            return std::nullopt;
        }

        // 设置游戏ID
        game.setId(game_id);

        spdlog::info("Game created successfully: {}", name);
        return game;
    } catch (const std::exception& e) {
        spdlog::error("Error creating game: {}", e.what());
        return std::nullopt;
    }
}

// 根据ID查找游戏
std::optional<model::Game> GameService::findGameById(int id) {
    try {
        if (id <= 0) {
            spdlog::error("Invalid game ID: {}", id);
            return std::nullopt;
        }

        std::optional<model::Game> game = game_repository_->findById(id);
        if (!game) {
            spdlog::error("Game not found: ID = {}", id);
            return std::nullopt;
        }

        return game;
    } catch (const std::exception& e) {
        spdlog::error("Error finding game by ID: {}", e.what());
        return std::nullopt;
    }
}

// 根据game_key查找游戏
std::optional<model::Game> GameService::findGameByGameKey(const std::string& game_key) {
    try {
        if (game_key.empty()) {
            spdlog::error("Game key cannot be empty");
            return std::nullopt;
        }

        std::optional<model::Game> game = game_repository_->findByGameKey(game_key);
        if (!game) {
            spdlog::error("Game not found: Game key = {}", game_key);
            return std::nullopt;
        }

        return game;
    } catch (const std::exception& e) {
        spdlog::error("Error finding game by game key: {}", e.what());
        return std::nullopt;
    }
}

// 获取所有游戏列表
std::vector<model::Game> GameService::getAllGames() {
    try {
        std::vector<model::Game> games = game_repository_->findAll();
        spdlog::info("Retrieved all games: Total = {}", games.size());
        return games;
    } catch (const std::exception& e) {
        spdlog::error("Error retrieving all games: {}", e.what());
        return std::vector<model::Game>();
    }
}

// 检查game_key是否存在
bool GameService::existsByGameKey(const std::string& game_key) {
    try {
        if (game_key.empty()) {
            spdlog::error("Game key cannot be empty");
            return false;
        }

        return game_repository_->existsByGameKey(game_key);
    } catch (const std::exception& e) {
        spdlog::error("Error checking if game key exists: {}", e.what());
        return false;
    }
}

} // namespace service