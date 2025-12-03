#include "controllers/user_controller.h"
#include "services/user_service.h"
#include "utils/json.h"
#include "utils/logger.h"

UserController::UserController() : user_service_(UserService::getInstance()), json_(Json::getInstance()), logger_(Logger::getInstance()) {
}

UserController::~UserController() {
}

UserController& UserController::getInstance() {
    static UserController instance;
    return instance;
}

std::string UserController::registerUser(const std::string& request_body) {
    try {
        auto data = json_.deserialize(request_body);

        // 验证必填字段
        if (data.find("username") == data.end() || data.find("email") == data.end() || data.find("password") == data.end()) {
            return json_.createErrorResponse(400, "Missing required fields: username, email, password");
        }

        std::string username = std::any_cast<std::string>(data["username"]);
        std::string email = std::any_cast<std::string>(data["email"]);
        std::string password = std::any_cast<std::string>(data["password"]);

        // 验证用户名格式
        if (username.empty() || username.length() < 3 || username.length() > 20) {
            return json_.createErrorResponse(400, "Username must be 3-20 characters long");
        }

        // 验证邮箱格式（简单验证）
        if (email.empty() || email.find("@") == std::string::npos) {
            return json_.createErrorResponse(400, "Invalid email format");
        }

        // 验证密码强度
        if (password.empty() || password.length() < 6) {
            return json_.createErrorResponse(400, "Password must be at least 6 characters long");
        }

        // 调用服务层注册用户
        bool success = user_service_->registerUser(username, email, password);

        if (success) {
            logger_.log(Logger::LogLevel::INFO, "User registered successfully: " + username);
            return json_.createSuccessResponse(201, "User registered successfully");
        } else {
            logger_.log(Logger::LogLevel::WARN, "User registration failed: " + username + " or " + email + " already exists");
            return json_.createErrorResponse(400, "Username or email already exists");
        }
    } catch (const std::exception& e) {
        logger_.log(Logger::LogLevel::ERROR, "User registration error: " + std::string(e.what()));
        return json_.createErrorResponse(500, "Internal server error");
    }
}

std::string UserController::loginUser(const std::string& request_body) {
    try {
        auto data = json_.deserialize(request_body);

        // 验证必填字段
        if (data.find("email") == data.end() || data.find("password") == data.end()) {
            return json_.createErrorResponse(400, "Missing required fields: email, password");
        }

        std::string email = std::any_cast<std::string>(data["email"]);
        std::string password = std::any_cast<std::string>(data["password"]);

        // 调用服务层登录用户
        std::string token = user_service_->loginUser(email, password);

        if (!token.empty()) {
            logger_.log(Logger::LogLevel::INFO, "User logged in successfully: " + email);
            std::map<std::string, std::any> response_data;
            response_data["token"] = token;
            return json_.createSuccessResponse(200, "Login successful", response_data);
        } else {
            logger_.log(Logger::LogLevel::WARN, "User login failed: " + email + " - invalid credentials");
            return json_.createErrorResponse(401, "Invalid email or password");
        }
    } catch (const std::exception& e) {
        logger_.log(Logger::LogLevel::ERROR, "User login error: " + std::string(e.what()));
        return json_.createErrorResponse(500, "Internal server error");
    }
}

std::string UserController::getUserInfo(const std::string& token) {
    try {
        // 验证令牌
        if (!user_service_->verifyToken(token)) {
            return json_.createErrorResponse(401, "Invalid token");
        }

        // 获取用户ID
        int user_id = user_service_->getUserIdFromToken(token);

        // 调用服务层获取用户信息
        User user = user_service_->getUserById(user_id);

        if (user.id == 0) {
            return json_.createErrorResponse(404, "User not found");
        }

        std::map<std::string, std::any> response_data;
        response_data["id"] = user.id;
        response_data["username"] = user.username;
        response_data["email"] = user.email;
        response_data["created_at"] = user.created_at;
        response_data["question_count"] = user.question_count;
        response_data["answer_count"] = user.answer_count;

        return json_.createSuccessResponse(200, "User info retrieved successfully", response_data);
    } catch (const std::exception& e) {
        logger_.log(Logger::LogLevel::ERROR, "Get user info error: " + std::string(e.what()));
        return json_.createErrorResponse(500, "Internal server error");
    }
}
