#include "ScoreController.h"
#include "../service/ScoreService.h"
#include "../service/SessionService.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <nlohmann/json.hpp>

namespace controller {

ScoreController::ScoreController(
    std::shared_ptr<service::ScoreService> score_service,
    std::shared_ptr<service::SessionService> session_service)
    : score_service_(std::move(score_service)),
      session_service_(std::move(session_service)) {
    if (!score_service_) {
        throw std::invalid_argument("ScoreService cannot be null");
    }
    if (!session_service_) {
        throw std::invalid_argument("SessionService cannot be null");
    }
}

void ScoreController::registerRoutes(httplib::Server& server) {
    // 注册成绩相关的路由
    server.Post("/api/leaderboards/:leaderboard_id/submit", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleSubmitScore(req, res);
    });

    server.Get("/api/leaderboards/:leaderboard_id/top", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetTopScores(req, res);
    });

    server.Get("/api/leaderboards/:leaderboard_id/me", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetUserScoreInLeaderboard(req, res);
    });

    server.Get("/api/users/:user_id/scores", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetUserScores(req, res);
    });
}

void ScoreController::handleSubmitScore(const httplib::Request& req, httplib::Response& res) {
    try {
        // 验证请求中的token
        std::optional<int> user_id = validateToken(req);
        if (!user_id) {
            sendJsonResponse(res, false, 401, "Invalid or expired token");
            return;
        }

        // 提取排行榜ID参数
        if (req.matches.size() < 2) {
            sendJsonResponse(res, false, 400, "Missing leaderboard ID parameter");
            return;
        }

        int leaderboard_id = std::stoi(req.matches[1]);
        if (leaderboard_id <= 0) {
            sendJsonResponse(res, false, 400, "Invalid leaderboard ID");
            return;
        }

        // 解析请求体中的JSON数据
        nlohmann::json request_body = nlohmann::json::parse(req.body);

        // 验证请求参数
        if (!request_body.contains("score")) {
            sendJsonResponse(res, false, 400, "Missing required parameters");
            return;
        }

        int score = request_body["score"];
        if (score < 0) {
            sendJsonResponse(res, false, 400, "Score cannot be negative");
            return;
        }

        std::string extra_data = request_body.value("extra_data", "");

        // 调用ScoreService提交成绩
        std::optional<model::Score> submitted_score = score_service_->submitScore(
            leaderboard_id,
            *user_id,
            score,
            extra_data
        );
        if (!submitted_score) {
            sendJsonResponse(res, false, 404, "Leaderboard not found");
            return;
        }

        // 构造响应数据
        nlohmann::json score_data;
        score_data["id"] = submitted_score->getId();
        score_data["leaderboard_id"] = submitted_score->getLeaderboardId();
        score_data["user_id"] = submitted_score->getUserId();
        score_data["score"] = submitted_score->getScore();
        score_data["extra_data"] = submitted_score->getExtraData();
        score_data["created_at"] = submitted_score->getCreatedAt().time_since_epoch().count();

        sendJsonResponse(res, true, 0, "Score submitted successfully", score_data);
        spdlog::info("Score submitted successfully: Leaderboard ID = {}, User ID = {}, Score = {}", leaderboard_id, *user_id, score);
    } catch (const std::invalid_argument& e) {
        sendJsonResponse(res, false, 400, "Invalid leaderboard ID format");
        spdlog::error("Invalid leaderboard ID format: {}", e.what());
    } catch (const nlohmann::json::parse_error& e) {
        sendJsonResponse(res, false, 400, "Invalid JSON format");
        spdlog::error("JSON parse error in score submission: {}", e.what());
    } catch (const std::exception& e) {
        sendJsonResponse(res, false, 500, "Internal server error");
        spdlog::error("Error in score submission: {}", e.what());
    }
}

void ScoreController::handleGetTopScores(const httplib::Request& req, httplib::Response& res) {
    try {
        // 验证请求中的token（这里可以简化为不需要登录）
        // 为了简单起见，这里暂时不验证token

        // 提取排行榜ID参数
        if (req.matches.size() < 2) {
            sendJsonResponse(res, false, 400, "Missing leaderboard ID parameter");
            return;
        }

        int leaderboard_id = std::stoi(req.matches[1]);
        if (leaderboard_id <= 0) {
            sendJsonResponse(res, false, 400, "Invalid leaderboard ID");
            return;
        }

        // 提取limit参数（默认为10）
        int limit = 10;
        auto limit_it = req.params.find("limit");
        if (limit_it != req.params.end()) {
            limit = std::stoi(limit_it->second);
            if (limit < 1 || limit > 100) {
                sendJsonResponse(res, false, 400, "Limit must be between 1 and 100");
                return;
            }
        }

        // 调用ScoreService获取排行榜前N名
        std::vector<model::Score> top_scores = score_service_->findTopScoresByLeaderboardId(leaderboard_id, limit);

        // 构造响应数据
        nlohmann::json top_scores_data = nlohmann::json::array();
        for (size_t i = 0; i < top_scores.size(); ++i) {
            const auto& score = top_scores[i];
            nlohmann::json score_data;
            score_data["rank"] = i + 1;
            score_data["user_id"] = score.getUserId();
            score_data["score"] = score.getScore();
            score_data["extra_data"] = score.getExtraData();
            score_data["created_at"] = score.getCreatedAt().time_since_epoch().count();
            top_scores_data.push_back(score_data);
        }

        sendJsonResponse(res, true, 0, "Top scores retrieved successfully", top_scores_data);
        spdlog::info("Top scores retrieved successfully: Leaderboard ID = {}, Limit = {}, Total = {}", leaderboard_id, limit, top_scores.size());
    } catch (const std::invalid_argument& e) {
        sendJsonResponse(res, false, 400, "Invalid leaderboard ID or limit format");
        spdlog::error("Invalid leaderboard ID or limit format: {}", e.what());
    } catch (const std::exception& e) {
        sendJsonResponse(res, false, 500, "Internal server error");
        spdlog::error("Error in getting top scores: {}", e.what());
    }
}

void ScoreController::handleGetUserScoreInLeaderboard(const httplib::Request& req, httplib::Response& res) {
    try {
        // 验证请求中的token
        std::optional<int> user_id = validateToken(req);
        if (!user_id) {
            sendJsonResponse(res, false, 401, "Invalid or expired token");
            return;
        }

        // 提取排行榜ID参数
        if (req.matches.size() < 2) {
            sendJsonResponse(res, false, 400, "Missing leaderboard ID parameter");
            return;
        }

        int leaderboard_id = std::stoi(req.matches[1]);
        if (leaderboard_id <= 0) {
            sendJsonResponse(res, false, 400, "Invalid leaderboard ID");
            return;
        }

        // 调用ScoreService获取用户在排行榜中的排名和成绩
        std::optional<int> rank = score_service_->findRankByLeaderboardIdAndUserId(leaderboard_id, *user_id);
        std::optional<model::Score> best_score = score_service_->findBestScoreByLeaderboardIdAndUserId(leaderboard_id, *user_id);

        // 构造响应数据
        nlohmann::json user_score_data;
        if (rank && best_score) {
            user_score_data["rank"] = *rank;
            user_score_data["score"] = best_score->getScore();
            user_score_data["extra_data"] = best_score->getExtraData();
            user_score_data["created_at"] = best_score->getCreatedAt().time_since_epoch().count();
        } else {
            user_score_data["rank"] = nullptr;
            user_score_data["score"] = nullptr;
            user_score_data["extra_data"] = nullptr;
            user_score_data["created_at"] = nullptr;
        }

        sendJsonResponse(res, true, 0, "User score in leaderboard retrieved successfully", user_score_data);
        spdlog::info("User score in leaderboard retrieved successfully: Leaderboard ID = {}, User ID = {}", leaderboard_id, *user_id);
    } catch (const std::invalid_argument& e) {
        sendJsonResponse(res, false, 400, "Invalid leaderboard ID format");
        spdlog::error("Invalid leaderboard ID format: {}", e.what());
    } catch (const std::exception& e) {
        sendJsonResponse(res, false, 500, "Internal server error");
        spdlog::error("Error in getting user score in leaderboard: {}", e.what());
    }
}

void ScoreController::handleGetUserScores(const httplib::Request& req, httplib::Response& res) {
    try {
        // 验证请求中的token（这里可以简化为不需要登录）
        // 为了简单起见，这里暂时不验证token

        // 提取用户ID参数
        if (req.matches.size() < 2) {
            sendJsonResponse(res, false, 400, "Missing user ID parameter");
            return;
        }

        int user_id = std::stoi(req.matches[1]);
        if (user_id <= 0) {
            sendJsonResponse(res, false, 400, "Invalid user ID");
            return;
        }

        // 提取可选的leaderboard_id参数
        std::optional<int> leaderboard_id;
        auto leaderboard_id_it = req.params.find("leaderboard_id");
        if (leaderboard_id_it != req.params.end()) {
            leaderboard_id = std::stoi(leaderboard_id_it->second);
            if (leaderboard_id <= 0) {
                sendJsonResponse(res, false, 400, "Invalid leaderboard ID");
                return;
            }
        }

        // 提取limit参数（默认为10）
        int limit = 10;
        auto limit_it = req.params.find("limit");
        if (limit_it != req.params.end()) {
            limit = std::stoi(limit_it->second);
            if (limit < 1 || limit > 100) {
                sendJsonResponse(res, false, 400, "Limit must be between 1 and 100");
                return;
            }
        }

        // 调用ScoreService获取用户的历史成绩
        std::vector<model::Score> user_scores;
        if (leaderboard_id) {
            // 先获取该排行榜的所有成绩
            std::vector<model::Score> all_scores = score_service_->findScoresByLeaderboardId(*leaderboard_id);
            // 过滤出该用户的成绩
            for (const auto& score : all_scores) {
                if (score.getUserId() == user_id) {
                    user_scores.push_back(score);
                }
            }
            // 按时间倒序排序
            std::sort(user_scores.begin(), user_scores.end(), [](const model::Score& a, const model::Score& b) {
                return a.getCreatedAt() > b.getCreatedAt();
            });
            // 限制数量
            if (user_scores.size() > static_cast<size_t>(limit)) {
                user_scores.resize(limit);
            }
        } else {
            // 先获取该用户的所有成绩
            std::vector<model::Score> all_scores = score_service_->findScoresByUserId(user_id);
            // 按时间倒序排序
            std::sort(all_scores.begin(), all_scores.end(), [](const model::Score& a, const model::Score& b) {
                return a.getCreatedAt() > b.getCreatedAt();
            });
            // 限制数量
            if (all_scores.size() > static_cast<size_t>(limit)) {
                all_scores.resize(limit);
            }
            user_scores = all_scores;
        }

        // 构造响应数据
        nlohmann::json user_scores_data = nlohmann::json::array();
        for (const auto& score : user_scores) {
            nlohmann::json score_data;
            score_data["id"] = score.getId();
            score_data["leaderboard_id"] = score.getLeaderboardId();
            score_data["score"] = score.getScore();
            score_data["extra_data"] = score.getExtraData();
            score_data["created_at"] = score.getCreatedAt().time_since_epoch().count();
            user_scores_data.push_back(score_data);
        }

        sendJsonResponse(res, true, 0, "User scores retrieved successfully", user_scores_data);
        spdlog::info("User scores retrieved successfully: User ID = {}, Leaderboard ID = {}, Limit = {}, Total = {}", 
            user_id, leaderboard_id ? std::to_string(*leaderboard_id) : "all", limit, user_scores.size());
    } catch (const std::invalid_argument& e) {
        sendJsonResponse(res, false, 400, "Invalid user ID, leaderboard ID, or limit format");
        spdlog::error("Invalid user ID, leaderboard ID, or limit format: {}", e.what());
    } catch (const std::exception& e) {
        sendJsonResponse(res, false, 500, "Internal server error");
        spdlog::error("Error in getting user scores: {}", e.what());
    }
}

std::optional<int> ScoreController::validateToken(const httplib::Request& req) {
    // 检查请求头中是否包含Authorization字段
    auto it = req.headers.find("Authorization");
    if (it == req.headers.end()) {
        spdlog::error("Authorization header not found");
        return std::nullopt;
    }

    // 提取token（假设Authorization字段的格式为"Bearer <token>"）
    std::string auth_header = it->second;
    if (auth_header.substr(0, 7) != "Bearer ") {
        spdlog::error("Invalid Authorization header format");
        return std::nullopt;
    }

    std::string token = auth_header.substr(7);
    if (token.empty()) {
        spdlog::error("Token cannot be empty");
        return std::nullopt;
    }

    // 调用SessionService验证token
    std::optional<model::Session> session = session_service_->findSessionByToken(token);
    if (!session) {
        spdlog::error("Invalid or expired token: {}", token);
        return std::nullopt;
    }

    return session->getUserId();
}

void ScoreController::sendJsonResponse(httplib::Response& res, bool success, int code, const std::string& message, const nlohmann::json& data) {
    // 构造统一格式的JSON响应
    nlohmann::json response;
    response["success"] = success;
    response["code"] = code;
    response["message"] = message;

    if (!data.is_null() && !data.empty()) {
        response["data"] = data;
    }

    // 设置响应体
    res.set_content(response.dump(), "application/json");
}

} // namespace controller