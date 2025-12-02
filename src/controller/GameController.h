#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <httplib.h>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

namespace service {
    class GameService;
    class SessionService;
}

namespace controller {

class GameController {
public:
    GameController(
        std::shared_ptr<service::GameService> game_service,
        std::shared_ptr<service::SessionService> session_service);
    ~GameController() = default;

    // 禁用拷贝操作
    GameController(const GameController&) = delete;
    GameController& operator=(const GameController&) = delete;

    // 允许移动操作
    GameController(GameController&&) noexcept = default;
    GameController& operator=(GameController&&) noexcept = default;

    // 注册路由
    void registerRoutes(httplib::Server& server);

private:
    // 处理创建游戏请求
    void handleCreateGame(const httplib::Request& req, httplib::Response& res);

    // 处理获取游戏列表请求
    void handleGetGames(const httplib::Request& req, httplib::Response& res);

    // 处理获取游戏详情请求
    void handleGetGame(const httplib::Request& req, httplib::Response& res);

    // 验证请求中的token
    std::optional<int> validateToken(const httplib::Request& req);

    // 发送统一格式的JSON响应
    void sendJsonResponse(httplib::Response& res, bool success, int code, const std::string& message, const nlohmann::json& data = nlohmann::json());

    // 服务层依赖
    std::shared_ptr<service::GameService> game_service_;
    std::shared_ptr<service::SessionService> session_service_;
};

} // namespace controller

#endif // GAME_CONTROLLER_H