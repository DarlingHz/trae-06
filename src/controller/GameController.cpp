#include "GameController.h"
#include "../service/GameService.h"
#include "../service/SessionService.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <nlohmann/json.hpp>

namespace controller {

GameController::GameController(
    std::shared_ptr<service::GameService> game_service,
    std::shared_ptr<service::SessionService> session_service)
    : game_service_(std::move(game_service)),
      session_service_(std::move(session_service)) {
    if (!game_service_) {
        throw std::invalid_argument("GameService cannot be null");
    }
    if (!session_service_) {
        throw std::invalid_argument("SessionService cannot be null");
    }
}

void GameController::registerRoutes(httplib::Server& server) {
    // 注册游戏相关的路由
    server.Post("/api/games", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleCreateGame(req, res);
    });

    server.Get("/api/games", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetGames(req, res);
    });

    server.Get("/api/games/:id", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetGame(req, res);
    });
}

void GameController::handleCreateGame(const httplib::Request& req, httplib::Response& res) {
    try {
        (void)req; // 忽略未使用的参数
        // 验证请求中的token（这里可以简化为不需要登录，或者只允许管理员创建游戏）
        // 为了简单起见，这里暂时不验证token

        // 解析请求体中的JSON数据
        nlohmann::json request_body = nlohmann::json::parse(req.body);

        // 验证请求参数
        if (!request_body.contains("game_key") || !request_body.contains("name")) {
            sendJsonResponse(res, false, 400, "Missing required parameters");
            return;
        }

        std::string game_key = request_body["game_key"];
        std::string name = request_body["name"];

        if (game_key.empty() || name.empty()) {
            sendJsonResponse(res, false, 400, "Game key or name cannot be empty");
            return;
        }

        // 调用GameService创建游戏
        std::optional<model::Game> game = game_service_->createGame(game_key, name);
        if (!game) {
            sendJsonResponse(res, false, 409, "Game key already exists");
            return;
        }

        // 构造响应数据
        nlohmann::json game_data;
        game_data["id"] = game->getId();
        game_data["game_key"] = game->getGameKey();
        game_data["name"] = game->getName();
        game_data["created_at"] = game->getCreatedAt().time_since_epoch().count();

        sendJsonResponse(res, true, 0, "Game created successfully", game_data);
        spdlog::info("Game created successfully: Game Key = {}, Name = {}", game_key, name);
    } catch (const nlohmann::json::parse_error& e) {
        sendJsonResponse(res, false, 400, "Invalid JSON format");
        spdlog::error("JSON parse error in game creation: {}", e.what());
    } catch (const std::exception& e) {
        sendJsonResponse(res, false, 500, "Internal server error");
        spdlog::error("Error in game creation: {}", e.what());
    }
}

void GameController::handleGetGames(const httplib::Request& req, httplib::Response& res) {
    try {
        (void)req; // 忽略未使用的参数
        // 验证请求中的token（这里可以简化为不需要登录）
        // 为了简单起见，这里暂时不验证token

        // 调用GameService获取游戏列表
        std::vector<model::Game> games = game_service_->getAllGames();

        // 构造响应数据
        nlohmann::json games_data = nlohmann::json::array();
        for (const auto& game : games) {
            nlohmann::json game_data;
            game_data["id"] = game.getId();
            game_data["game_key"] = game.getGameKey();
            game_data["name"] = game.getName();
            game_data["created_at"] = game.getCreatedAt().time_since_epoch().count();
            games_data.push_back(game_data);
        }

        sendJsonResponse(res, true, 0, "Games retrieved successfully", games_data);
        spdlog::info("Games retrieved successfully: Total = {}", games.size());
    } catch (const std::exception& e) {
        sendJsonResponse(res, false, 500, "Internal server error");
        spdlog::error("Error in getting games: {}", e.what());
    }
}

void GameController::handleGetGame(const httplib::Request& req, httplib::Response& res) {
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

        // 调用GameService查找游戏
        std::optional<model::Game> game = game_service_->findGameById(game_id);
        if (!game) {
            sendJsonResponse(res, false, 404, "Game not found");
            return;
        }

        // 构造响应数据
        nlohmann::json game_data;
        game_data["id"] = game->getId();
        game_data["game_key"] = game->getGameKey();
        game_data["name"] = game->getName();
        game_data["created_at"] = game->getCreatedAt().time_since_epoch().count();

        sendJsonResponse(res, true, 0, "Game retrieved successfully", game_data);
        spdlog::info("Game retrieved successfully: Game ID = {}", game_id);
    } catch (const std::invalid_argument& e) {
        sendJsonResponse(res, false, 400, "Invalid game ID format");
        spdlog::error("Invalid game ID format: {}", e.what());
    } catch (const std::exception& e) {
        sendJsonResponse(res, false, 500, "Internal server error");
        spdlog::error("Error in getting game: {}", e.what());
    }
}

std::optional<int> GameController::validateToken(const httplib::Request& req) {
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

void GameController::sendJsonResponse(httplib::Response& res, bool success, int code, const std::string& message, const nlohmann::json& data) {
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