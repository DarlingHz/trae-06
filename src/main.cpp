#include <iostream>
#include <memory>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// 项目组件头文件
#include "controller/UserController.h"
#include "controller/GameController.h"
#include "controller/LeaderboardController.h"
#include "controller/ScoreController.h"
#include "service/UserService.h"
#include "service/GameService.h"
#include "service/LeaderboardService.h"
#include "service/ScoreService.h"
#include "service/SessionService.h"
#include "repository/SQLiteUserRepository.h"
#include "repository/SQLiteGameRepository.h"
#include "repository/SQLiteLeaderboardRepository.h"
#include "repository/SQLiteScoreRepository.h"
#include "repository/SQLiteSessionRepository.h"

using json = nlohmann::json;
using namespace httplib;
using namespace controller;
using namespace service;
using namespace repository;

int main() {
    // 初始化日志系统
    auto logger = spdlog::stdout_color_mt("console");
    spdlog::set_level(spdlog::level::info);
    logger->info("Starting multi-game real-time leaderboard backend service...");

    // 初始化数据库连接
    const std::string db_path = "./leaderboard.db";
    logger->info("Connecting to database: {}", db_path);

    try {
        // 创建仓库层实例
        auto user_repo = std::make_unique<SQLiteUserRepository>(db_path);
        auto game_repo = std::make_unique<SQLiteGameRepository>(db_path);
        auto leaderboard_repo = std::make_shared<SQLiteLeaderboardRepository>(db_path);
        auto score_repo = std::make_shared<SQLiteScoreRepository>(db_path);
        auto session_repo = std::make_unique<SQLiteSessionRepository>(db_path);

        // 创建服务层实例
        auto user_service = std::make_unique<UserService>(std::move(user_repo));
        auto session_service = std::make_unique<SessionService>(
            std::move(session_repo), std::move(user_service));
        auto game_service = std::make_unique<GameService>(std::move(game_repo));
        auto leaderboard_service = std::make_unique<LeaderboardService>(
            std::move(leaderboard_repo), score_repo);
        auto score_service = std::make_unique<ScoreService>(
            std::move(score_repo), std::move(leaderboard_service),
            session_service->getUserService());

        // 创建控制器层实例
        auto user_controller = std::make_unique<UserController>(
            std::move(user_service), std::move(session_service));
        auto game_controller = std::make_unique<GameController>(
            std::move(game_service), std::move(session_service));
        auto leaderboard_controller = std::make_unique<LeaderboardController>(
            std::move(leaderboard_service), std::move(session_service));
        auto score_controller = std::make_unique<ScoreController>(
            std::move(score_service), std::move(session_service));

        // 配置HTTP服务器
        Server svr;
        const int port = 8080;

        // 注册路由
        user_controller->registerRoutes(svr);
        game_controller->registerRoutes(svr);
        leaderboard_controller->registerRoutes(svr);
        score_controller->registerRoutes(svr);

        // 健康检查路由
        svr.Get("/api/health", [logger](const Request&, Response& res) {
            json response;
            response["success"] = true;
            response["code"] = 0;
            response["message"] = "Service is running";
            res.set_content(response.dump(), "application/json");
            logger->info("Health check requested");
        });

        // 启动服务器
        logger->info("Starting HTTP server on port {}", port);
        if (svr.listen("0.0.0.0", port)) {
            logger->info("Server started successfully");
        } else {
            logger->error("Failed to start server on port {}", port);
            return 1;
        }

    } catch (const std::exception& e) {
        logger->error("Failed to initialize service: {}", e.what());
        return 1;
    }

    return 0;
}
