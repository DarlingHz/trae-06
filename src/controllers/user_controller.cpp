#include "parking/controllers.h"
#include "parking/services.h"
#include "parking/models.h"
#include "parking/utils.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

// UserController实现
void UserController::register_user(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析请求
        auto data = json::parse(req.body);

        // 验证参数
        if (!data.contains("name") || !data["name"].is_string() || data["name"].get<std::string>().empty()) {
            ResponseUtils::error(res, 400, "Invalid name");
            return;
        }
        if (!data.contains("email") || !data["email"].is_string() || data["email"].get<std::string>().empty()) {
            ResponseUtils::error(res, 400, "Invalid email");
            return;
        }
        if (!data.contains("password") || !data["password"].is_string() || data["password"].get<std::string>().empty()) {
            ResponseUtils::error(res, 400, "Invalid password");
            return;
        }

        std::string name = data["name"].get<std::string>();
        std::string email = data["email"].get<std::string>();
        std::string password = data["password"].get<std::string>();

        // 调用服务
        User user = user_service_->register_user(name, email, password);

        // 构建响应
        json response_data;
        response_data["user"] = user;
        ResponseUtils::success(res, 201, "User registered successfully", response_data);
    } catch (const ServiceError& e) {
        Logger::error("User registration error: " + std::string(e.what()));
        if (e.type() == ServiceError::Type::USER_ALREADY_EXISTS) {
            ResponseUtils::error(res, 409, e.what());
        } else {
            ResponseUtils::error(res, 400, e.what());
        }
    } catch (const std::exception& e) {
        Logger::error("User registration exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
    }
}

void UserController::login(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析请求
        auto data = json::parse(req.body);

        // 验证参数
        if (!data.contains("email") || !data["email"].is_string() || data["email"].get<std::string>().empty()) {
            ResponseUtils::error(res, 400, "Invalid email");
            return;
        }
        if (!data.contains("password") || !data["password"].is_string() || data["password"].get<std::string>().empty()) {
            ResponseUtils::error(res, 400, "Invalid password");
            return;
        }

        std::string email = data["email"].get<std::string>();
        std::string password = data["password"].get<std::string>();

        // 调用服务
        auto [user, token] = user_service_->login(email, password);

        // 构建响应
        json response_data;
        response_data["user"] = user;
        response_data["token"] = token;
        ResponseUtils::success(res, 200, "Login successful", response_data);
    } catch (const ServiceError& e) {
        Logger::error("Login error: " + std::string(e.what()));
        if (e.type() == ServiceError::Type::INVALID_CREDENTIALS || e.type() == ServiceError::Type::USER_NOT_FOUND) {
            ResponseUtils::error(res, 401, e.what());
        } else {
            ResponseUtils::error(res, 400, e.what());
        }
    } catch (const std::exception& e) {
        Logger::error("Login exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
    }
}

void UserController::get_current_user(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取当前用户
        const auto& user = req.get_header_value("X-Current-User-Id");
        if (user.empty()) {
            ResponseUtils::error(res, 401, "Unauthorized");
            return;
        }

        int user_id = std::stoi(user);
        auto user_opt = user_service_->get_user(user_id);
        if (!user_opt) {
            ResponseUtils::error(res, 401, "User not found");
            return;
        }

        // 构建响应
        json response_data;
        response_data["user"] = *user_opt;
        ResponseUtils::success(res, 200, "User info retrieved successfully", response_data);
    } catch (const std::exception& e) {
        Logger::error("Get current user exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
    }
}

void UserController::logout(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取Token
        auto token = req.get_header_value("X-Auth-Token");
        if (!token.empty()) {
            user_service_->logout(token);
        }

        ResponseUtils::success(res, 200, "Logout successful", {});
    } catch (const std::exception& e) {
        Logger::error("Logout exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
    }
}
