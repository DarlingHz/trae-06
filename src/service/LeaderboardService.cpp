#include "LeaderboardService.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <chrono>

namespace service {

// 提交成绩
bool LeaderboardService::submitScore(int leaderboard_id, int user_id, int score) {
    try {
        // 业务逻辑验证
        if (leaderboard_id <= 0) {
            spdlog::error("Invalid leaderboard ID: {}", leaderboard_id);
            return false;
        }

        if (user_id <= 0) {
            spdlog::error("Invalid user ID: {}", user_id);
            return false;
        }

        if (score < 0) {
            spdlog::error("Invalid score: {}", score);
            return false;
        }

        // 检查排行榜是否存在
        std::optional<model::Leaderboard> leaderboard = leaderboard_repository_->findById(leaderboard_id);
        if (!leaderboard) {
            spdlog::error("Leaderboard not found: ID = {}", leaderboard_id);
            return false;
        }

        // 检查用户是否已提交过成绩
        std::vector<model::Score> existing_scores = score_repository_->findByLeaderboardIdAndUserId(leaderboard_id, user_id);
        std::optional<model::Score> existing_score;
        if (!existing_scores.empty()) {
            existing_score = existing_scores[0];
        }

        if (existing_score) {
            // 如果用户已提交过成绩，根据排行榜规则决定是否更新
            if (leaderboard->getScoreRule() == model::ScoreRule::HIGHEST) {
                if (score > existing_score->getScore()) {
                    // 更新为更高的成绩
                    model::Score updated_score = *existing_score;
                    updated_score.setScore(score);
                    updated_score.setUpdatedAt(std::chrono::system_clock::now());

                    if (!score_repository_->update(updated_score)) {
                        spdlog::error("Failed to update score for user ID = {} in leaderboard ID = {}", user_id, leaderboard_id);
                        return false;
                    }

                    spdlog::info("Score updated successfully for user ID = {} in leaderboard ID = {}", user_id, leaderboard_id);
                } else {
                    // 新成绩不高于已有成绩，不更新
                    spdlog::info("New score is not higher than existing score for user ID = {} in leaderboard ID = {}", user_id, leaderboard_id);
                }
            } else if (leaderboard->getScoreRule() == model::ScoreRule::CUMULATIVE) {
                // 累计成绩，更新为已有成绩加上新成绩
                model::Score updated_score = *existing_score;
                updated_score.setScore(existing_score->getScore() + score);
                updated_score.setUpdatedAt(std::chrono::system_clock::now());

                if (!score_repository_->update(updated_score)) {
                    spdlog::error("Failed to update score for user ID = {} in leaderboard ID = {}", user_id, leaderboard_id);
                    return false;
                }

                spdlog::info("Score updated successfully for user ID = {} in leaderboard ID = {}", user_id, leaderboard_id);
            }
        } else {
            // 如果用户未提交过成绩，创建新成绩
            model::Score new_score;
            new_score.setLeaderboardId(leaderboard_id);
            new_score.setUserId(user_id);
            new_score.setScore(score);
            new_score.setCreatedAt(std::chrono::system_clock::now());
            new_score.setUpdatedAt(std::chrono::system_clock::now());

            int score_id = score_repository_->create(new_score);
            if (score_id <= 0) {
                spdlog::error("Failed to create score for user ID = {} in leaderboard ID = {}", user_id, leaderboard_id);
                return false;
            }

            spdlog::info("Score created successfully for user ID = {} in leaderboard ID = {}", user_id, leaderboard_id);
        }

        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error submitting score: {}", e.what());
        return false;
    }
}

// 获取用户排名
int LeaderboardService::getUserRank(int leaderboard_id, int user_id) {
    try {
        // 业务逻辑验证
        if (leaderboard_id <= 0) {
            spdlog::error("Invalid leaderboard ID: {}", leaderboard_id);
            return -1;
        }

        if (user_id <= 0) {
            spdlog::error("Invalid user ID: {}", user_id);
            return -1;
        }

        // 检查排行榜是否存在
        std::optional<model::Leaderboard> leaderboard = leaderboard_repository_->findById(leaderboard_id);
        if (!leaderboard) {
            spdlog::error("Leaderboard not found: ID = {}", leaderboard_id);
            return -1;
        }

        // 获取用户的最好成绩
        std::optional<model::Score> user_score = score_repository_->findBestByLeaderboardIdAndUserId(leaderboard_id, user_id);
        if (!user_score) {
            spdlog::error("Score not found for user ID = {} in leaderboard ID = {}", user_id, leaderboard_id);
            return -1;
        }

        // 获取排行榜中所有成绩
        std::vector<model::Score> all_scores = score_repository_->findByLeaderboardId(leaderboard_id);
        if (all_scores.empty()) {
            spdlog::error("No scores found in leaderboard ID = {}", leaderboard_id);
            return -1;
        }

        // 根据排行榜规则排序成绩
        if (leaderboard->getScoreRule() == model::ScoreRule::HIGHEST) {
            // 按成绩降序排序
            std::sort(all_scores.begin(), all_scores.end(), [](const model::Score& a, const model::Score& b) {
                return a.getScore() > b.getScore();
            });
        } else if (leaderboard->getScoreRule() == model::ScoreRule::CUMULATIVE) {
            // 按成绩降序排序
            std::sort(all_scores.begin(), all_scores.end(), [](const model::Score& a, const model::Score& b) {
                return a.getScore() > b.getScore();
            });
        }

        // 查找用户的排名
        int rank = 1;
        for (const auto& score : all_scores) {
            if (score.getUserId() == user_id) {
                return rank;
            }
            rank++;
        }

        // 理论上不会走到这里，因为用户的成绩已经存在于all_scores中
        spdlog::error("User ID = {} not found in all scores for leaderboard ID = {}", user_id, leaderboard_id);
        return -1;
    } catch (const std::exception& e) {
        spdlog::error("Error getting user rank: {}", e.what());
        return -1;
    }
}

// 获取用户分数
int LeaderboardService::getUserScore(int leaderboard_id, int user_id) {
    try {
        // 业务逻辑验证
        if (leaderboard_id <= 0) {
            spdlog::error("Invalid leaderboard ID: {}", leaderboard_id);
            return -1;
        }

        if (user_id <= 0) {
            spdlog::error("Invalid user ID: {}", user_id);
            return -1;
        }

        // 获取用户的成绩
        std::vector<model::Score> user_scores = score_repository_->findByLeaderboardIdAndUserId(leaderboard_id, user_id);
        std::optional<model::Score> user_score;
        if (!user_scores.empty()) {
            user_score = user_scores[0];
        }
        if (!user_score) {
            spdlog::error("Score not found for user ID = {} in leaderboard ID = {}", user_id, leaderboard_id);
            return -1;
        }

        return user_score->getScore();
    } catch (const std::exception& e) {
        spdlog::error("Error getting user score: {}", e.what());
        return -1;
    }
}

// 获取Top N排名
std::vector<model::Score> LeaderboardService::getTopScores(int leaderboard_id, int limit) {
    try {
        // 业务逻辑验证
        if (leaderboard_id <= 0) {
            spdlog::error("Invalid leaderboard ID: {}", leaderboard_id);
            return {};
        }

        if (limit <= 0) {
            spdlog::error("Invalid limit: {}", limit);
            return {};
        }

        // 检查排行榜是否存在
        std::optional<model::Leaderboard> leaderboard = leaderboard_repository_->findById(leaderboard_id);
        if (!leaderboard) {
            spdlog::error("Leaderboard not found: ID = {}", leaderboard_id);
            return {};
        }

        // 获取排行榜中前N名成绩
        std::vector<model::Score> top_scores = score_repository_->findTopByLeaderboardId(leaderboard_id, limit);
        if (top_scores.empty()) {
            spdlog::error("No scores found in leaderboard ID = {}", leaderboard_id);
            return {};
        }

        return top_scores;
    } catch (const std::exception& e) {
        spdlog::error("Error getting top scores: {}", e.what());
        return {};
    }
}

// 创建新排行榜
std::optional<model::Leaderboard> LeaderboardService::createLeaderboard(
    int game_id,
    const std::string& name,
    const std::string& region,
    model::ScoreRule score_rule) {
    try {
        // 业务逻辑验证
        if (game_id <= 0) {
            spdlog::error("Invalid game ID: {}", game_id);
            return std::nullopt;
        }

        if (name.empty()) {
            spdlog::error("Leaderboard name cannot be empty");
            return std::nullopt;
        }

        if (name.length() < 1 || name.length() > 50) {
            spdlog::error("Leaderboard name must be between 1 and 50 characters");
            return std::nullopt;
        }

        if (region.length() > 20) {
            spdlog::error("Leaderboard region must be less than 20 characters");
            return std::nullopt;
        }

        // 检查游戏下是否已存在同名排行榜
        std::optional<model::Leaderboard> existing_leaderboard = 
            leaderboard_repository_->findByGameIdAndName(game_id, name);
        if (existing_leaderboard) {
            spdlog::error("Leaderboard with name '{}' already exists for game ID = {}", name, game_id);
            return std::nullopt;
        }

        // 创建排行榜对象
        model::Leaderboard leaderboard;
        leaderboard.setGameId(game_id);
        leaderboard.setName(name);
        leaderboard.setRegion(region);
        leaderboard.setScoreRule(score_rule);
        leaderboard.setCreatedAt(std::chrono::system_clock::now());

        // 保存排行榜到数据库
        int leaderboard_id = leaderboard_repository_->create(leaderboard);
        if (leaderboard_id <= 0) {
            spdlog::error("Failed to create leaderboard: {}", name);
            return std::nullopt;
        }

        // 设置排行榜ID
        leaderboard.setId(leaderboard_id);

        spdlog::info("Leaderboard created successfully: {} for game ID = {}", name, game_id);
        return leaderboard;
    } catch (const std::exception& e) {
        spdlog::error("Error creating leaderboard: {}", e.what());
        return std::nullopt;
    }
}

// 根据ID查找排行榜
std::optional<model::Leaderboard> LeaderboardService::findLeaderboardById(int id) {
    try {
        if (id <= 0) {
            spdlog::error("Invalid leaderboard ID: {}", id);
            return std::nullopt;
        }

        std::optional<model::Leaderboard> leaderboard = leaderboard_repository_->findById(id);
        if (!leaderboard) {
            spdlog::error("Leaderboard not found: ID = {}", id);
            return std::nullopt;
        }

        return leaderboard;
    } catch (const std::exception& e) {
        spdlog::error("Error finding leaderboard by ID: {}", e.what());
        return std::nullopt;
    }
}

// 根据游戏ID查找排行榜
std::vector<model::Leaderboard> LeaderboardService::findLeaderboardsByGameId(int game_id) {
    try {
        if (game_id <= 0) {
            spdlog::error("Invalid game ID: {}", game_id);
            return std::vector<model::Leaderboard>();
        }

        // 检查游戏是否存在
        std::optional<model::Game> game = game_service_->findGameById(game_id);
        if (!game) {
            spdlog::error("Game not found: ID = {}", game_id);
            return std::vector<model::Leaderboard>();
        }

        std::vector<model::Leaderboard> leaderboards = leaderboard_repository_->findByGameId(game_id);
        spdlog::info("Retrieved leaderboards for game ID = {}: Total = {}", game_id, leaderboards.size());
        return leaderboards;
    } catch (const std::exception& e) {
        spdlog::error("Error retrieving leaderboards by game ID: {}", e.what());
        return std::vector<model::Leaderboard>();
    }
}

// 根据游戏ID和名称查找排行榜
std::optional<model::Leaderboard> LeaderboardService::findLeaderboardByGameIdAndName(int game_id, const std::string& name) {
    try {
        if (game_id <= 0) {
            spdlog::error("Invalid game ID: {}", game_id);
            return std::nullopt;
        }

        if (name.empty()) {
            spdlog::error("Leaderboard name cannot be empty");
            return std::nullopt;
        }

        // 检查游戏是否存在
        std::optional<model::Game> game = game_service_->findGameById(game_id);
        if (!game) {
            spdlog::error("Game not found: ID = {}", game_id);
            return std::nullopt;
        }

        std::optional<model::Leaderboard> leaderboard = 
            leaderboard_repository_->findByGameIdAndName(game_id, name);
        if (!leaderboard) {
            spdlog::error("Leaderboard not found: Game ID = {}, Name = {}", game_id, name);
            return std::nullopt;
        }

        return leaderboard;
    } catch (const std::exception& e) {
        spdlog::error("Error finding leaderboard by game ID and name: {}", e.what());
        return std::nullopt;
    }
}

// 根据游戏ID和区域查找排行榜
std::vector<model::Leaderboard> LeaderboardService::findLeaderboardsByGameIdAndRegion(int game_id, const std::string& region) {
    try {
        if (game_id <= 0) {
            spdlog::error("Invalid game ID: {}", game_id);
            return std::vector<model::Leaderboard>();
        }

        if (region.empty()) {
            spdlog::error("Leaderboard region cannot be empty");
            return std::vector<model::Leaderboard>();
        }

        // 检查游戏是否存在
        std::optional<model::Game> game = game_service_->findGameById(game_id);
        if (!game) {
            spdlog::error("Game not found: ID = {}", game_id);
            return std::vector<model::Leaderboard>();
        }

        std::vector<model::Leaderboard> leaderboards = 
            leaderboard_repository_->findByGameIdAndRegion(game_id, region);
        spdlog::info("Retrieved leaderboards for game ID = {} and region = {}: Total = {}", game_id, region, leaderboards.size());
        return leaderboards;
    } catch (const std::exception& e) {
        spdlog::error("Error retrieving leaderboards by game ID and region: {}", e.what());
        return std::vector<model::Leaderboard>();
    }
}

// 获取所有排行榜列表
std::vector<model::Leaderboard> LeaderboardService::getAllLeaderboards() {
    try {
        std::vector<model::Leaderboard> leaderboards = leaderboard_repository_->findAll();
        spdlog::info("Retrieved all leaderboards: Total = {}", leaderboards.size());
        return leaderboards;
    } catch (const std::exception& e) {
        spdlog::error("Error retrieving all leaderboards: {}", e.what());
        return std::vector<model::Leaderboard>();
    }
}

} // namespace service