#include "controllers/AuthController.hpp"
#include <stdexcept>
#include <ctime>

namespace controllers {

bool AuthController::validateRegistrationRequest(const nlohmann::json& request, std::string& error_message) {
    // Check if all required fields are present
    if (!request.contains("username") || !request.contains("email") || !request.contains("password")) {
        error_message = "Missing required fields: username, email, password";
        return false;
    }

    // Check if fields are not empty
    std::string username = request["username"];
    std::string email = request["email"];
    std::string password = request["password"];

    if (username.empty() || email.empty() || password.empty()) {
        error_message = "Username, email, and password cannot be empty";
        return false;
    }

    // Validate username format
    if (!utils::AuthUtils::validateUsername(username)) {
        error_message = "Invalid username format: must be 3-20 alphanumeric characters";
        return false;
    }

    // Validate email format
    if (!utils::AuthUtils::validateEmail(email)) {
        error_message = "Invalid email format";
        return false;
    }

    // Validate password strength
    if (!utils::AuthUtils::validatePasswordStrength(password)) {
        error_message = "Invalid password: must be at least 8 characters long";
        return false;
    }

    // Check if username already exists
    models::User existing_user_by_username = user_dao_.getUserByUsername(username);
    if (existing_user_by_username.getId() != 0) {
        error_message = "Username already exists";
        return false;
    }

    // Check if email already exists
    models::User existing_user_by_email = user_dao_.getUserByEmail(email);
    if (existing_user_by_email.getId() != 0) {
        error_message = "Email already exists";
        return false;
    }

    return true;
}

bool AuthController::validateLoginRequest(const nlohmann::json& request, std::string& error_message) {
    // Check if all required fields are present
    if (!request.contains("username_or_email") || !request.contains("password")) {
        error_message = "Missing required fields: username_or_email, password";
        return false;
    }

    // Check if fields are not empty
    std::string username_or_email = request["username_or_email"];
    std::string password = request["password"];

    if (username_or_email.empty() || password.empty()) {
        error_message = "Username/email and password cannot be empty";
        return false;
    }

    return true;
}

crow::response AuthController::registerUser(const crow::request& req) {
    try {
        // Parse request body to JSON
        nlohmann::json request_body = utils::JsonUtils::parse(req.body);

        // Validate registration request
        std::string error_message;
        if (!validateRegistrationRequest(request_body, error_message)) {
            return crow::response(400, utils::JsonUtils::createErrorResponse("BAD_REQUEST", error_message));
        }

        // Create new user object
        models::User new_user;
        new_user.setUsername(request_body["username"]);
        new_user.setEmail(request_body["email"]);

        // Hash password
        std::string password_hash = utils::AuthUtils::hashPassword(request_body["password"]);
        new_user.setPasswordHash(password_hash);

        // Set created_at timestamp
        std::time_t now = std::time(nullptr);
        char buf[20];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        new_user.setCreatedAt(buf);

        // Save user to database
        bool created = user_dao_.createUser(new_user);
        if (!created) {
            return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", "Failed to create user"));
        }

        // Get the created user (with ID)
        models::User created_user = user_dao_.getUserByUsername(new_user.getUsername());

        // Prepare response
        nlohmann::json response_body;
        response_body["id"] = created_user.getId();
        response_body["username"] = created_user.getUsername();
        response_body["email"] = created_user.getEmail();
        response_body["created_at"] = created_user.getCreatedAt();

        return crow::response(201, response_body.dump());

    } catch (const nlohmann::json::parse_error& e) {
        return crow::response(400, utils::JsonUtils::createErrorResponse("BAD_REQUEST", "Invalid JSON format"));
    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

crow::response AuthController::loginUser(const crow::request& req) {
    try {
        // Parse request body to JSON
        nlohmann::json request_body = utils::JsonUtils::parse(req.body);

        // Validate login request
        std::string error_message;
        if (!validateLoginRequest(request_body, error_message)) {
            return crow::response(400, utils::JsonUtils::createErrorResponse("BAD_REQUEST", error_message));
        }

        // Get user by username or email
        std::string username_or_email = request_body["username_or_email"];
        models::User user = user_dao_.getUserByUsernameOrEmail(username_or_email);

        // Check if user exists
        if (user.getId() == 0) {
            return crow::response(401, utils::JsonUtils::createErrorResponse("UNAUTHORIZED", "Invalid username/email or password"));
        }

        // Verify password
        bool password_valid = utils::AuthUtils::verifyPassword(request_body["password"], user.getPasswordHash());
        if (!password_valid) {
            return crow::response(401, utils::JsonUtils::createErrorResponse("UNAUTHORIZED", "Invalid username/email or password"));
        }

        // Generate access token
        std::string token = utils::AuthUtils::generateToken(user.getId(), user.getUsername());

        // Prepare response
        nlohmann::json response_body;
        response_body["token"] = token;

        nlohmann::json user_json;
        user_json["id"] = user.getId();
        user_json["username"] = user.getUsername();
        user_json["email"] = user.getEmail();
        user_json["created_at"] = user.getCreatedAt();

        response_body["user"] = user_json;

        return crow::response(200, response_body.dump());

    } catch (const nlohmann::json::parse_error& e) {
        return crow::response(400, utils::JsonUtils::createErrorResponse("BAD_REQUEST", "Invalid JSON format"));
    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

} // namespace controllers
