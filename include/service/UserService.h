#pragma once

#include "models/User.h"
#include "repository/UserRepository.h"
#include "auth/JWT.h"
#include <memory>
#include <string>
#include <optional>

// JSON deserialization
#include <nlohmann/json.hpp>

namespace service {

struct UserRegisterRequest {
    std::string email;
    std::string password;
    std::string nickname;
};

struct UserLoginRequest {
    std::string email;
    std::string password;
};

void from_json(const nlohmann::json& j, UserRegisterRequest& req);
void from_json(const nlohmann::json& j, UserLoginRequest& req);

struct LoginResponse {
    std::shared_ptr<models::User> user;
    std::string token;
};

class UserService {
private:
    std::unique_ptr<repository::UserRepository> user_repo_;
    std::shared_ptr<auth::JWT> jwt_;

public:
    bool validate_email(const std::string& email) const;
    bool validate_password(const std::string& password) const;
    std::string hash_password(const std::string& password) const;
    bool verify_password(const std::string& password, const std::string& hash) const;

    UserService(std::unique_ptr<repository::UserRepository> user_repo, 
                std::shared_ptr<auth::JWT> jwt);

    std::shared_ptr<models::User> register_user(const UserRegisterRequest& request);
    
    std::optional<LoginResponse> login_user(const UserLoginRequest& request);
    
    std::shared_ptr<models::User> get_user_by_id(int id);
    
    std::shared_ptr<models::User> get_user_by_email(const std::string& email);
    
    bool update_user(std::shared_ptr<models::User> user);
};

} // namespace service
