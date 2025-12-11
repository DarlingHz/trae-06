#pragma once

#include "daos/UserDAO.hpp"
#include "utils/AuthUtils.hpp"
#include "utils/JsonUtils.hpp"
#include <crow.h>

namespace controllers {

class AuthController {
public:
    // Constructor
    explicit AuthController(daos::UserDAO& user_dao) : user_dao_(user_dao) {}

    // Destructor
    ~AuthController() = default;

    // Handle user registration
    crow::response registerUser(const crow::request& req);

    // Handle user login
    crow::response loginUser(const crow::request& req);

private:
    daos::UserDAO& user_dao_;

    // Validate registration request
    bool validateRegistrationRequest(const nlohmann::json& request, std::string& error_message);

    // Validate login request
    bool validateLoginRequest(const nlohmann::json& request, std::string& error_message);
};

} // namespace controllers
