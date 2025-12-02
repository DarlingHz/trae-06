#include "UserController.h"
#include "../service/UserService.h"
#include "../service/SessionService.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <nlohmann/json.hpp>

namespace controller {

UserController::UserController(
    std::shared_ptr<service::UserService> user_service,
    std::shared_ptr<service::SessionService> session_service)
    : user_service_(std::move(user_service)),
      session_service_(std::move(session_service)) {
    if (!user_service_) {
        throw std::invalid_argument("UserService cannot be null");
    }
    if (!session_service_) {
        throw std::invalid_argument("SessionService cannot be null");
    }
}

void UserController::registerRoutes(httplib::Server& server) {
    // 注册用户相关的路由
    server.Post("/api/users/register", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleUserRegister(req, res);
    });

    server.Post("/api/users/login", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleUserLogin(req, res);
    });

    server.Get("/api/users/me", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetUserInfo(req, res);
    });
}

void UserController::handleUserRegister(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析请求体中的JSON数据
        nlohmann::json request_body = nlohmann::json::parse(req.body);

        // 验证请求参数
        if (!request_body.contains("username") || !request_body.contains("password")) {
            sendJsonResponse(res, false, 400, "Missing required parameters");
            return;
        }

        std::string username = request_body["username"];
        std::string password = request_body["password"];

        if (username.empty() || password.empty()) {
            sendJsonResponse(res, false, 400, "Username or password cannot be empty");
            return;
        }

        // 调用UserService注册用户
        std::optional<model::User> user = user_service_->registerUser(username, password);
        if (!user) {
            sendJsonResponse(res, false, 409, "Username already exists");
            return;
        }

        // 构造响应数据（不包含密码哈希）
        nlohmann::json user_data;
        user_data["id"] = user->getId();
        user_data["username"] = user->getUsername();
        user_data["created_at"] = user->getCreatedAt().time_since_epoch().count();

        sendJsonResponse(res, true, 0, "User registered successfully", user_data);
        spdlog::info("User registered successfully: Username = {}", username);
    } catch (const nlohmann::json::parse_error& e) {
        sendJsonResponse(res, false, 400, "Invalid JSON format");
        spdlog::error("JSON parse error in user registration: {}", e.what());
    } catch (const std::exception& e) {
        sendJsonResponse(res, false, 500, "Internal server error");
        spdlog::error("Error in user registration: {}", e.what());
    }
}

void UserController::handleUserLogin(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析请求体中的JSON数据
        nlohmann::json request_body = nlohmann::json::parse(req.body);

        // 验证请求参数
        if (!request_body.contains("username") || !request_body.contains("password")) {
            sendJsonResponse(res, false, 400, "Missing required parameters");
            return;
        }

        std::string username = request_body["username"];
        std::string password = request_body["password"];

        if (username.empty() || password.empty()) {
            sendJsonResponse(res, false, 400, "Username or password cannot be empty");
            return;
        }

        // 调用UserService验证用户密码
        std::optional<model::User> user = user_service_->authenticateUser(username, password);
        if (!user) {
            sendJsonResponse(res, false, 401, "Invalid username or password");
            return;
        }

        // 调用SessionService创建会话（会话有效期为7天）
        std::optional<model::Session> session = session_service_->createSession(
            user->getId(),
            std::chrono::hours(24 * 7)
        );
        if (!session) {
            sendJsonResponse(res, false, 500, "Failed to create session");
            return;
        }

        // 构造响应数据
        nlohmann::json session_data;
        session_data["token"] = session->getToken();
        session_data["expire_at"] = session->getExpireAt().time_since_epoch().count();

        sendJsonResponse(res, true, 0, "User logged in successfully", session_data);
        spdlog::info("User logged in successfully: Username = {}", username);
    } catch (const nlohmann::json::parse_error& e) {
        sendJsonResponse(res, false, 400, "Invalid JSON format");
        spdlog::error("JSON parse error in user login: {}", e.what());
    } catch (const std::exception& e) {
        sendJsonResponse(res, false, 500, "Internal server error");
        spdlog::error("Error in user login: {}", e.what());
    }
}

void UserController::handleGetUserInfo(const httplib::Request& req, httplib::Response& res) {
    try {
        // 验证请求中的token
        std::optional<int> user_id = validateToken(req);
        if (!user_id) {
            sendJsonResponse(res, false, 401, "Invalid or expired token");
            return;
        }

        // 调用UserService查找用户
        std::optional<model::User> user = user_service_->findUserById(*user_id);
        if (!user) {
            sendJsonResponse(res, false, 404, "User not found");
            return;
        }

        // 构造响应数据（不包含密码哈希）
        nlohmann::json user_data;
        user_data["id"] = user->getId();
        user_data["username"] = user->getUsername();
        user_data["created_at"] = user->getCreatedAt().time_since_epoch().count();

        sendJsonResponse(res, true, 0, "User info retrieved successfully", user_data);
        spdlog::info("User info retrieved successfully: User ID = {}", *user_id);
    } catch (const std::exception& e) {
        sendJsonResponse(res, false, 500, "Internal server error");
        spdlog::error("Error in getting user info: {}", e.what());
    }
}

std::optional<int> UserController::validateToken(const httplib::Request& req) {
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

void UserController::sendJsonResponse(httplib::Response& res, bool success, int code, const std::string& message, const nlohmann::json& data) {
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