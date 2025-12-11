#ifndef LEADERBOARD_CONTROLLER_H
#define LEADERBOARD_CONTROLLER_H

#include <httplib.h>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

namespace service {
    class LeaderboardService;
    class SessionService;
}

namespace controller {

class LeaderboardController {
public:
    LeaderboardController(
        std::shared_ptr<service::LeaderboardService> leaderboard_service,
        std::shared_ptr<service::SessionService> session_service);
    ~LeaderboardController() = default;

    // 禁用拷贝操作
    LeaderboardController(const LeaderboardController&) = delete;
    LeaderboardController& operator=(const LeaderboardController&) = delete;

    // 允许移动操作
    LeaderboardController(LeaderboardController&&) noexcept = default;
    LeaderboardController& operator=(LeaderboardController&&) noexcept = default;

    // 注册路由
    void registerRoutes(httplib::Server& server);

private:
    // 处理创建排行榜请求
    void handleCreateLeaderboard(const httplib::Request& req, httplib::Response& res);

    // 处理获取游戏下的排行榜列表请求
    void handleGetLeaderboardsByGameId(const httplib::Request& req, httplib::Response& res);

    // 处理获取排行榜详情请求
    void handleGetLeaderboard(const httplib::Request& req, httplib::Response& res);

    // 验证请求中的token
    std::optional<int> validateToken(const httplib::Request& req);

    // 发送统一格式的JSON响应
    void sendJsonResponse(httplib::Response& res, bool success, int code, const std::string& message, const nlohmann::json& data = nlohmann::json());

    // 服务层依赖
    std::shared_ptr<service::LeaderboardService> leaderboard_service_;
    std::shared_ptr<service::SessionService> session_service_;
};

} // namespace controller

#endif // LEADERBOARD_CONTROLLER_H