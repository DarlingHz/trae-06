#include "ScoreService.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <chrono>

namespace service {

// 提交成绩
std::optional<model::Score> ScoreService::submitScore(
    int leaderboard_id,
    int user_id,
    int score,
    const nlohmann::json& extra_data) {
    try {
        // 业务逻辑验证
        if (leaderboard_id <= 0) {
            spdlog::error("Invalid leaderboard ID: {}", leaderboard_id);
            return std::nullopt;
        }

        if (user_id <= 0) {
            spdlog::error("Invalid user ID: {}", user_id);
            return std::nullopt;
        }

        if (score < 0) {
            spdlog::error("Invalid score: {}", score);
            return std::nullopt;
        }

        // 检查排行榜是否存在
        std::optional<model::Leaderboard> leaderboard = leaderboard_service_->findLeaderboardById(leaderboard_id);
        if (!leaderboard) {
            spdlog::error("Leaderboard not found: ID = {}", leaderboard_id);
            return std::nullopt;
        }

        // 检查用户是否存在
        std::optional<model::User> user = user_service_->findUserById(user_id);
        if (!user) {
            spdlog::error("User not found: ID = {}", user_id);
            return std::nullopt;
        }

        // 根据排行榜规则处理成绩
        model::Score new_score;
        new_score.setLeaderboardId(leaderboard_id);
        new_score.setUserId(user_id);
        new_score.setScore(score);
        new_score.setExtraData(extra_data);
        new_score.setCreatedAt(std::chrono::system_clock::now());

        std::optional<model::Score> best_score;
    std::optional<model::Score> cumulative_score;

    switch (leaderboard->getScoreRule()) {
        case model::ScoreRule::HIGHEST:
            // 查找用户当前最佳成绩
            best_score = score_repository_->findBestByLeaderboardIdAndUserId(leaderboard_id, user_id);
            if (best_score && best_score->getScore() >= score) {
                spdlog::info("New score is not better than current best score for user ID = {} in leaderboard ID = {}", user_id, leaderboard_id);
                return best_score;
            }
            break;

        case model::ScoreRule::CUMULATIVE:
            // 查找用户当前累计成绩
            cumulative_score = score_repository_->findBestByLeaderboardIdAndUserId(leaderboard_id, user_id);
            if (cumulative_score) {
                new_score.setScore(cumulative_score->getScore() + score);
            }
            break;

        default:
            spdlog::error("Unknown score rule: {}", static_cast<int>(leaderboard->getScoreRule()));
            return std::nullopt;
    }

        // 保存成绩到数据库
        int score_id = score_repository_->create(new_score);
        if (score_id <= 0) {
            spdlog::error("Failed to submit score for user ID = {} in leaderboard ID = {}", user_id, leaderboard_id);
            return std::nullopt;
        }

        // 设置成绩ID
        new_score.setId(score_id);
        std::optional<model::Score> created_score = new_score;

        spdlog::info("Score submitted successfully for user ID = {} in leaderboard ID = {}: Score = {}", user_id, leaderboard_id, created_score->getScore());
        return created_score;
    } catch (const std::exception& e) {
        spdlog::error("Error submitting score: {}", e.what());
        return std::nullopt;
    }
}

// 根据ID查找成绩
std::optional<model::Score> ScoreService::findScoreById(int id) {
    try {
        if (id <= 0) {
            spdlog::error("Invalid score ID: {}", id);
            return std::nullopt;
        }

        std::optional<model::Score> score = score_repository_->findById(id);
        if (!score) {
            spdlog::error("Score not found: ID = {}", id);
            return std::nullopt;
        }

        return score;
    } catch (const std::exception& e) {
        spdlog::error("Error finding score by ID: {}", e.what());
        return std::nullopt;
    }
}

// 根据排行榜ID查找成绩
std::vector<model::Score> ScoreService::findScoresByLeaderboardId(int leaderboard_id) {
    try {
        if (leaderboard_id <= 0) {
            spdlog::error("Invalid leaderboard ID: {}", leaderboard_id);
            return std::vector<model::Score>();
        }

        // 检查排行榜是否存在
        std::optional<model::Leaderboard> leaderboard = leaderboard_service_->findLeaderboardById(leaderboard_id);
        if (!leaderboard) {
            spdlog::error("Leaderboard not found: ID = {}", leaderboard_id);
            return std::vector<model::Score>();
        }

        std::vector<model::Score> scores = score_repository_->findByLeaderboardId(leaderboard_id);
        spdlog::info("Retrieved scores for leaderboard ID = {}: Total = {}", leaderboard_id, scores.size());
        return scores;
    } catch (const std::exception& e) {
        spdlog::error("Error retrieving scores by leaderboard ID: {}", e.what());
        return std::vector<model::Score>();
    }
}

// 根据用户ID查找成绩
std::vector<model::Score> ScoreService::findScoresByUserId(int user_id) {
    try {
        if (user_id <= 0) {
            spdlog::error("Invalid user ID: {}", user_id);
            return std::vector<model::Score>();
        }

        // 检查用户是否存在
        std::optional<model::User> user = user_service_->findUserById(user_id);
        if (!user) {
            spdlog::error("User not found: ID = {}", user_id);
            return std::vector<model::Score>();
        }

        std::vector<model::Score> scores = score_repository_->findByUserId(user_id);
        spdlog::info("Retrieved scores for user ID = {}: Total = {}", user_id, scores.size());
        return scores;
    } catch (const std::exception& e) {
        spdlog::error("Error retrieving scores by user ID: {}", e.what());
        return std::vector<model::Score>();
    }
}

// 根据排行榜ID查找前N名成绩
std::vector<model::Score> ScoreService::findTopScoresByLeaderboardId(int leaderboard_id, int limit) {
    try {
        if (leaderboard_id <= 0) {
            spdlog::error("Invalid leaderboard ID: {}", leaderboard_id);
            return std::vector<model::Score>();
        }

        if (limit <= 0) {
            spdlog::error("Invalid limit: {}", limit);
            return std::vector<model::Score>();
        }

        // 检查排行榜是否存在
        std::optional<model::Leaderboard> leaderboard = leaderboard_service_->findLeaderboardById(leaderboard_id);
        if (!leaderboard) {
            spdlog::error("Leaderboard not found: ID = {}", leaderboard_id);
            return std::vector<model::Score>();
        }

        std::vector<model::Score> top_scores = score_repository_->findTopByLeaderboardId(leaderboard_id, limit);
        spdlog::info("Retrieved top {} scores for leaderboard ID = {}: Total = {}", limit, leaderboard_id, top_scores.size());
        return top_scores;
    } catch (const std::exception& e) {
        spdlog::error("Error retrieving top scores by leaderboard ID: {}", e.what());
        return std::vector<model::Score>();
    }
}

// 根据排行榜ID和用户ID查找最佳成绩
std::optional<model::Score> ScoreService::findBestScoreByLeaderboardIdAndUserId(int leaderboard_id, int user_id) {
    try {
        if (leaderboard_id <= 0) {
            spdlog::error("Invalid leaderboard ID: {}", leaderboard_id);
            return std::nullopt;
        }

        if (user_id <= 0) {
            spdlog::error("Invalid user ID: {}", user_id);
            return std::nullopt;
        }

        // 检查排行榜是否存在
        std::optional<model::Leaderboard> leaderboard = leaderboard_service_->findLeaderboardById(leaderboard_id);
        if (!leaderboard) {
            spdlog::error("Leaderboard not found: ID = {}", leaderboard_id);
            return std::nullopt;
        }

        // 检查用户是否存在
        std::optional<model::User> user = user_service_->findUserById(user_id);
        if (!user) {
            spdlog::error("User not found: ID = {}", user_id);
            return std::nullopt;
        }

        std::optional<model::Score> best_score = score_repository_->findBestByLeaderboardIdAndUserId(leaderboard_id, user_id);
        if (!best_score) {
            spdlog::error("Best score not found for user ID = {} in leaderboard ID = {}", user_id, leaderboard_id);
            return std::nullopt;
        }

        return best_score;
    } catch (const std::exception& e) {
        spdlog::error("Error finding best score by leaderboard ID and user ID: {}", e.what());
        return std::nullopt;
    }
}

// 根据排行榜ID和用户ID查找排名
int ScoreService::findRankByLeaderboardIdAndUserId(int leaderboard_id, int user_id) {
    try {
        if (leaderboard_id <= 0) {
            spdlog::error("Invalid leaderboard ID: {}", leaderboard_id);
            return 0;
        }

        if (user_id <= 0) {
            spdlog::error("Invalid user ID: {}", user_id);
            return 0;
        }

        // 检查排行榜是否存在
        std::optional<model::Leaderboard> leaderboard = leaderboard_service_->findLeaderboardById(leaderboard_id);
        if (!leaderboard) {
            spdlog::error("Leaderboard not found: ID = {}", leaderboard_id);
            return 0;
        }

        // 检查用户是否存在
        std::optional<model::User> user = user_service_->findUserById(user_id);
        if (!user) {
            spdlog::error("User not found: ID = {}", user_id);
            return 0;
        }

        int rank = score_repository_->findRankByLeaderboardIdAndUserId(leaderboard_id, user_id).value_or(0);
        spdlog::info("Retrieved rank for user ID = {} in leaderboard ID = {}: Rank = {}", user_id, leaderboard_id, rank);
        return rank;
    } catch (const std::exception& e) {
        spdlog::error("Error finding rank by leaderboard ID and user ID: {}", e.what());
        return 0;
    }
}

// 根据排行榜ID删除成绩
bool ScoreService::deleteScoresByLeaderboardId(int leaderboard_id) {
    try {
        if (leaderboard_id <= 0) {
            spdlog::error("Invalid leaderboard ID: {}", leaderboard_id);
            return false;
        }

        // 检查排行榜是否存在
        std::optional<model::Leaderboard> leaderboard = leaderboard_service_->findLeaderboardById(leaderboard_id);
        if (!leaderboard) {
            spdlog::error("Leaderboard not found: ID = {}", leaderboard_id);
            return false;
        }

        if (!score_repository_->deleteByLeaderboardId(leaderboard_id)) {
            spdlog::error("Failed to delete scores for leaderboard ID = {}", leaderboard_id);
            return false;
        }

        spdlog::info("Scores deleted successfully for leaderboard ID = {}", leaderboard_id);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error deleting scores by leaderboard ID: {}", e.what());
        return false;
    }
}

// 根据用户ID删除成绩
bool ScoreService::deleteScoresByUserId(int user_id) {
    try {
        if (user_id <= 0) {
            spdlog::error("Invalid user ID: {}", user_id);
            return false;
        }

        // 检查用户是否存在
        std::optional<model::User> user = user_service_->findUserById(user_id);
        if (!user) {
            spdlog::error("User not found: ID = {}", user_id);
            return false;
        }

        if (!score_repository_->deleteByUserId(user_id)) {
            spdlog::error("Failed to delete scores for user ID = {}", user_id);
            return false;
        }

        spdlog::info("Scores deleted successfully for user ID = {}", user_id);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error deleting scores by user ID: {}", e.what());
        return false;
    }
}

} // namespace service