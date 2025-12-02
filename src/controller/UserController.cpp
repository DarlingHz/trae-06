#include "controller/UserController.h"
#include "service/UserService.h"
#include "service/ServiceException.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

using json = nlohmann::json;

namespace controller {

UserController::UserController(std::shared_ptr<service::UserService> user_service, 
                               std::shared_ptr<server::HttpServer> http_server)
    : user_service_(std::move(user_service)),
      http_server_(std::move(http_server)) {
}

void UserController::registerEndpoints() {
    // 注册用户注册端点
    http_server_->registerHandler("POST", "/api/users/register", 
        [this](const server::http::request<server::http::string_body>& request, 
               server::http::response<server::http::string_body>& response) {
            handleRegister(request, response);
        });

    // 注册用户登录端点
    http_server_->registerHandler("POST", "/api/users/login", 
        [this](const server::http::request<server::http::string_body>& request, 
               server::http::response<server::http::string_body>& response) {
            handleLogin(request, response);
        });

    // 注册用户退出登录端点
    http_server_->registerHandler("POST", "/api/users/logout", 
        [this](const server::http::request<server::http::string_body>& request, 
               server::http::response<server::http::string_body>& response) {
            handleLogout(request, response);
        });
}

void UserController::handleRegister(const server::http::request<server::http::string_body>& request, 
                                     server::http::response<server::http::string_body>& response) {
    try {
        // 解析请求体中的 JSON 数据
        json request_body = json::parse(request.body());

        // 验证请求参数
        if (!request_body.contains("username") || !request_body["username"].is_string()) {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "INVALID_PARAMS",
                "message": "Username is required and must be a string"
            })";
            return;
        }

        if (!request_body.contains("password") || !request_body["password"].is_string()) {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "INVALID_PARAMS",
                "message": "Password is required and must be a string"
            })";
            return;
        }

        std::string username = request_body["username"];
        std::string password = request_body["password"];

        // 调用 UserService 中的 registerUser 方法
        model::User user = user_service_->registerUser(username, password);

        // 生成响应体
        json response_body;
        response_body["id"] = user.id();
        response_body["username"] = user.username();
        response_body["created_at"] = user.created_at().time_since_epoch().count();

        // 设置响应
        response.result(server::http::status::created);
        response.set(server::http::field::content_type, "application/json");
        response.body() = response_body.dump();
    } catch (const json::parse_error& e) {
        // 处理 JSON 解析错误
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INVALID_JSON",
            "message": "Failed to parse request body"
        })";
    } catch (const service::ServiceException& e) {
        // 处理业务逻辑错误
        if (e.what() == std::string("Username already exists")) {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "USER_ALREADY_EXISTS",
                "message": "Username already taken"
            })";
        } else {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = std::string(R"({
                "error": "BAD_REQUEST",
                "message": ")" ) + e.what() + R"("
            })";
        }
    } catch (const std::exception& e) {
        // 处理其他异常
        std::cerr << "Error handling register request: " << e.what() << std::endl;
        response.result(server::http::status::internal_server_error);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INTERNAL_SERVER_ERROR",
            "message": "An internal server error occurred"
        })";
    }
}

void UserController::handleLogin(const server::http::request<server::http::string_body>& request, 
                                  server::http::response<server::http::string_body>& response) {
    try {
        // 解析请求体中的 JSON 数据
        json request_body = json::parse(request.body());

        // 验证请求参数
        if (!request_body.contains("username") || !request_body["username"].is_string()) {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "INVALID_PARAMS",
                "message": "Username is required and must be a string"
            })";
            return;
        }

        if (!request_body.contains("password") || !request_body["password"].is_string()) {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "INVALID_PARAMS",
                "message": "Password is required and must be a string"
            })";
            return;
        }

        std::string username = request_body["username"];
        std::string password = request_body["password"];

        // 调用 UserService 中的 loginUser 方法
        std::string token = user_service_->loginUser(username, password);

        // 生成响应体
        json response_body;
        response_body["token"] = token;

        // 设置响应
        response.result(server::http::status::ok);
        response.set(server::http::field::content_type, "application/json");
        response.body() = response_body.dump();
    } catch (const json::parse_error& e) {
        // 处理 JSON 解析错误
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INVALID_JSON",
            "message": "Failed to parse request body"
        })";
    } catch (const service::ServiceException& e) {
        // 处理业务逻辑错误
        if (e.what() == std::string("Invalid username or password")) {
            response.result(server::http::status::unauthorized);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "UNAUTHORIZED",
                "message": "Invalid username or password"
            })";
        } else {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = std::string(R"({
                "error": "BAD_REQUEST",
                "message": ")" ) + e.what() + R"("
            })";
        }
    } catch (const std::exception& e) {
        // 处理其他异常
        std::cerr << "Error handling login request: " << e.what() << std::endl;
        response.result(server::http::status::internal_server_error);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INTERNAL_SERVER_ERROR",
            "message": "An internal server error occurred"
        })";
    }
}

void UserController::handleLogout(const server::http::request<server::http::string_body>& request, 
                                   server::http::response<server::http::string_body>& response) {
    try {
        // 从请求头中获取 Authorization 字段
        auto authorization_it = request.find(server::http::field::authorization);
        if (authorization_it == request.end()) {
            response.result(server::http::status::unauthorized);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "UNAUTHORIZED",
                "message": "Authorization header is required"
            })";
            return;
        }

        std::string authorization_header = authorization_it->value();
        // 验证 Authorization 字段的格式
        if (authorization_header.substr(0, 7) != "Bearer ") {
            response.result(server::http::status::unauthorized);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "UNAUTHORIZED",
                "message": "Invalid authorization header format"
            })";
            return;
        }

        // 提取 token
        std::string token = authorization_header.substr(7);

        // 调用 UserService 中的 logoutUser 方法
        user_service_->logoutUser(token);

        // 设置响应
        response.result(server::http::status::ok);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "message": "Logout successful"
        })";
    } catch (const service::ServiceException& e) {
        // 处理业务逻辑错误
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        response.body() = std::string(R"({
            "error": "BAD_REQUEST",
            "message": ")" ) + e.what() + R"("
        })";
    } catch (const std::exception& e) {
        // 处理其他异常
        std::cerr << "Error handling logout request: " << e.what() << std::endl;
        response.result(server::http::status::internal_server_error);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INTERNAL_SERVER_ERROR",
            "message": "An internal server error occurred"
        })";
    }
}

} // namespace controller