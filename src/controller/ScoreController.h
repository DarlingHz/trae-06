#ifndef SCORE_CONTROLLER_H
#define SCORE_CONTROLLER_H

#include <httplib.h>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

namespace service {
    class ScoreService;
    class SessionService;
}

namespace controller {

class ScoreController {
public:
    ScoreController(
        std::shared_ptr<service::ScoreService> score_service,
        std::shared_ptr<service::SessionService> session_service);
    ~ScoreController() = default;

    // 禁用拷贝操作
    ScoreController(const ScoreController&) = delete;
    ScoreController& operator=(const ScoreController&) = delete;

    // 允许移动操作
    ScoreController(ScoreController&&) noexcept = default;
    ScoreController& operator=(ScoreController&&) noexcept = default;

    // 注册路由
    void registerRoutes(httplib::Server& server);

private:
    // 处理提交成绩请求
    void handleSubmitScore(const httplib::Request& req, httplib::Response& res);

    // 处理获取排行榜前N名请求
    void handleGetTopScores(const httplib::Request& req, httplib::Response& res);

    // 处理获取当前用户在排行榜中的排名和成绩请求
    void handleGetUserScoreInLeaderboard(const httplib::Request& req, httplib::Response& res);

    // 处理获取用户在指定排行榜下的历史成绩请求
    void handleGetUserScores(const httplib::Request& req, httplib::Response& res);

    // 验证请求中的token
    std::optional<int> validateToken(const httplib::Request& req);

    // 发送统一格式的JSON响应
    void sendJsonResponse(httplib::Response& res, bool success, int code, const std::string& message, const nlohmann::json& data = nlohmann::json());

    // 服务层依赖
    std::shared_ptr<service::ScoreService> score_service_;
    std::shared_ptr<service::SessionService> session_service_;
};

} // namespace controller

#endif // SCORE_CONTROLLER_H