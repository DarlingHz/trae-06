#include "LeaderboardController.h"
#include "../service/LeaderboardService.h"
#include "../service/SessionService.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <nlohmann/json.hpp>

namespace controller {

LeaderboardController::LeaderboardController(
    std::shared_ptr<service::LeaderboardService> leaderboard_service,
    std::shared_ptr<service::SessionService> session_service)
    : leaderboard_service_(std::move(leaderboard_service)),
      session_service_(std::move(session_service)) {
    if (!leaderboard_service_) {
        throw std::invalid_argument("LeaderboardService cannot be null");
    }
    if (!session_service_) {
        throw std::invalid_argument("SessionService cannot be null");
    }
}

void LeaderboardController::registerRoutes(httplib::Server& server) {
    // 注册排行榜相关的路由
    server.Post("/api/games/:game_id/leaderboards", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleCreateLeaderboard(req, res);
    });

    server.Get("/api/games/:game_id/leaderboards", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetLeaderboardsByGameId(req, res);
    });

    server.Get("/api/leaderboards/:id", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetLeaderboard(req, res);
    });
}

void LeaderboardController::handleCreateLeaderboard(const httplib::Request& req, httplib::Response& res) {
    try {
        // 验证请求中的token（这里可以简化为不需要登录，或者只允许管理员创建排行榜）
        // 为了简单起见，这里暂时不验证token

        // 提取游戏ID参数
        if (req.matches.size() < 2) {
            sendJsonResponse(res, false, 400, "Missing game ID parameter");
            return;
        }

        int game_id = std::stoi(req.matches[1]);
        if (game_id <= 0) {
            sendJsonResponse(res, false, 400, "Invalid game ID");
            return;
        }

        // 解析请求体中的JSON数据
        nlohmann::json request_body = nlohmann::json::parse(req.body);

        // 验证请求参数
        if (!request_body.contains("name")) {
            sendJsonResponse(res, false, 400, "Missing required parameters");
            return;
        }

        std::string name = request_body["name"];
        std::string region = request_body.value("region", "");
        std::string score_rule = request_body.value("score_rule", "max");

        if (name.empty()) {
            sendJsonResponse(res, false, 400, "Leaderboard name cannot be empty");
            return;
        }

        // 验证score_rule是否有效
        if (score_rule != "max" && score_rule != "sum" && score_rule != "latest") {
            sendJsonResponse(res, false, 400, "Invalid score rule. Must be 'max', 'sum', or 'latest'");
            return;
        }

        // 将score_rule从std::string转换为model::ScoreRule
        model::ScoreRule score_rule_enum;
        try {
            score_rule_enum = model::Leaderboard::fromString(score_rule);
        } catch (const std::invalid_argument& e) {
            sendJsonResponse(res, false, 400, "Invalid score rule. Must be 'highest' or 'cumulative'");
            return;
        }

        // 调用LeaderboardService创建排行榜
        std::optional<model::Leaderboard> leaderboard = leaderboard_service_->createLeaderboard(
            game_id,
            name,
            region,
            score_rule_enum
        );
        if (!leaderboard) {
            sendJsonResponse(res, false, 409, "Leaderboard with the same name already exists for this game");
            return;
        }

        // 构造响应数据
        nlohmann::json leaderboard_data;
        leaderboard_data["id"] = leaderboard->getId();
        leaderboard_data["game_id"] = leaderboard->getGameId();
        leaderboard_data["name"] = leaderboard->getName();
        leaderboard_data["region"] = leaderboard->getRegion();
        leaderboard_data["score_rule"] = leaderboard->getScoreRule();
        leaderboard_data["created_at"] = leaderboard->getCreatedAt().time_since_epoch().count();

        sendJsonResponse(res, true, 0, "Leaderboard created successfully", leaderboard_data);
        spdlog::info("Leaderboard created successfully: Game ID = {}, Name = {}", game_id, name);
    } catch (const std::invalid_argument& e) {
        sendJsonResponse(res, false, 400, "Invalid game ID format");
        spdlog::error("Invalid game ID format: {}", e.what());
    } catch (const nlohmann::json::parse_error& e) {
        sendJsonResponse(res, false, 400, "Invalid JSON format");
        spdlog::error("JSON parse error in leaderboard creation: {}", e.what());
    } catch (const std::exception& e) {
        sendJsonResponse(res, false, 500, "Internal server error");
        spdlog::error("Error in leaderboard creation: {}", e.what());
    }
}

void LeaderboardController::handleGetLeaderboardsByGameId(const httplib::Request& req, httplib::Response& res) {
    try {
        // 验证请求中的token（这里可以简化为不需要登录）
        // 为了简单起见，这里暂时不验证token

        // 提取游戏ID参数
        if (req.matches.size() < 2) {
            sendJsonResponse(res, false, 400, "Missing game ID parameter");
            return;
        }

        int game_id = std::stoi(req.matches[1]);
        if (game_id <= 0) {
            sendJsonResponse(res, false, 400, "Invalid game ID");
            return;
        }

        // 提取可选的region参数
        std::string region;
        auto region_it = req.params.find("region");
        if (region_it != req.params.end()) {
            region = region_it->second;
        }

        // 调用LeaderboardService获取游戏下的排行榜列表
        std::vector<model::Leaderboard> leaderboards;
        if (region.empty()) {
            leaderboards = leaderboard_service_->findLeaderboardsByGameId(game_id);
        } else {
            leaderboards = leaderboard_service_->findLeaderboardsByGameIdAndRegion(game_id, region);
        }

        // 构造响应数据
        nlohmann::json leaderboards_data = nlohmann::json::array();
        for (const auto& leaderboard : leaderboards) {
            nlohmann::json leaderboard_data;
            leaderboard_data["id"] = leaderboard.getId();
            leaderboard_data["game_id"] = leaderboard.getGameId();
            leaderboard_data["name"] = leaderboard.getName();
            leaderboard_data["region"] = leaderboard.getRegion();
            leaderboard_data["score_rule"] = leaderboard.getScoreRule();
            leaderboard_data["created_at"] = leaderboard.getCreatedAt().time_since_epoch().count();
            leaderboards_data.push_back(leaderboard_data);
        }

        sendJsonResponse(res, true, 0, "Leaderboards retrieved successfully", leaderboards_data);
        spdlog::info("Leaderboards retrieved successfully: Game ID = {}, Total = {}", game_id, leaderboards.size());
    } catch (const std::invalid_argument& e) {
        sendJsonResponse(res, false, 400, "Invalid game ID format");
        spdlog::error("Invalid game ID format: {}", e.what());
    } catch (const std::exception& e) {
        sendJsonResponse(res, false, 500, "Internal server error");
        spdlog::error("Error in getting leaderboards: {}", e.what());
    }
}

void LeaderboardController::handleGetLeaderboard(const httplib::Request& req, httplib::Response& res) {
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

        // 调用LeaderboardService查找排行榜
        std::optional<model::Leaderboard> leaderboard = leaderboard_service_->findLeaderboardById(leaderboard_id);
        if (!leaderboard) {
            sendJsonResponse(res, false, 404, "Leaderboard not found");
            return;
        }

        // 构造响应数据
        nlohmann::json leaderboard_data;
        leaderboard_data["id"] = leaderboard->getId();
        leaderboard_data["game_id"] = leaderboard->getGameId();
        leaderboard_data["name"] = leaderboard->getName();
        leaderboard_data["region"] = leaderboard->getRegion();
        leaderboard_data["score_rule"] = leaderboard->getScoreRule();
        leaderboard_data["created_at"] = leaderboard->getCreatedAt().time_since_epoch().count();

        sendJsonResponse(res, true, 0, "Leaderboard retrieved successfully", leaderboard_data);
        spdlog::info("Leaderboard retrieved successfully: Leaderboard ID = {}", leaderboard_id);
    } catch (const std::invalid_argument& e) {
        sendJsonResponse(res, false, 400, "Invalid leaderboard ID format");
        spdlog::error("Invalid leaderboard ID format: {}", e.what());
    } catch (const std::exception& e) {
        sendJsonResponse(res, false, 500, "Internal server error");
        spdlog::error("Error in getting leaderboard: {}", e.what());
    }
}

std::optional<int> LeaderboardController::validateToken(const httplib::Request& req) {
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

void LeaderboardController::sendJsonResponse(httplib::Response& res, bool success, int code, const std::string& message, const nlohmann::json& data) {
    // 构造统一格式的JSON响应
    nlohmann::json response;
    response["success"] = success;
    response["code"] = code;
    response["message"] = message;

    if (!data.is_null() && !data.empty()) {
        response["data"] = data;
    }

    // 设置响应头和响应体
    res.set_content(response.dump(), "application/json");
}

} // namespace controller