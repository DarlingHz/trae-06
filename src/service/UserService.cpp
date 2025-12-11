#include "service/UserService.h"
#include <regex>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace service {

UserService::UserService(std::unique_ptr<repository::UserRepository> user_repo,
                         std::shared_ptr<auth::JWT> jwt)
    : user_repo_(std::move(user_repo)), jwt_(jwt) {}

bool UserService::validate_email(const std::string& email) const {
    const std::regex email_regex(R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");
    return std::regex_match(email, email_regex);
}

bool UserService::validate_password(const std::string& password) const {
    return password.length() >= 6;
}

std::string UserService::hash_password(const std::string& password) const {
    unsigned char salt[16];
    if(!RAND_bytes(salt, sizeof(salt))) {
        throw std::runtime_error("Failed to generate salt");
    }
    
    unsigned char hash[32];
    if(PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), 
                        salt, sizeof(salt), 100000, EVP_sha256(), 
                        sizeof(hash), hash) != 1) {
        throw std::runtime_error("Failed to hash password");
    }
    
    std::string result;
    result.reserve(sizeof(salt) + sizeof(hash));
    result.append(reinterpret_cast<char*>(salt), sizeof(salt));
    result.append(reinterpret_cast<char*>(hash), sizeof(hash));
    
    return result;
}

bool UserService::verify_password(const std::string& password, const std::string& hash) const {
    if(hash.length() != sizeof(unsigned char)*16 + sizeof(unsigned char)*32) {
        return false;
    }
    
    unsigned char salt[16];
    std::copy(hash.begin(), hash.begin() + 16, salt);
    
    unsigned char computed_hash[32];
    if(PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), 
                        salt, sizeof(salt), 100000, EVP_sha256(), 
                        sizeof(computed_hash), computed_hash) != 1) {
        return false;
    }
    
    return std::equal(hash.begin() + 16, hash.end(), 
                      reinterpret_cast<char*>(computed_hash));
}

std::shared_ptr<models::User> UserService::register_user(const UserRegisterRequest& request) {
    if(!validate_email(request.email)) {
        throw std::invalid_argument("Invalid email format");
    }
    
    if(!validate_password(request.password)) {
        throw std::invalid_argument("Password must be at least 6 characters long");
    }
    
    if(request.nickname.empty()) {
        throw std::invalid_argument("Nickname is required");
    }
    
    auto existing_user = user_repo_->find_by_email(request.email);
    if(existing_user) {
        throw std::runtime_error("Email already exists");
    }
    
    std::string password_hash = hash_password(request.password);
    
    return user_repo_->create(request.email, password_hash, request.nickname);
}

std::optional<LoginResponse> UserService::login_user(const UserLoginRequest& request) {
    auto user = user_repo_->find_by_email(request.email);
    
    if(!user || !verify_password(request.password, user->password_hash())) {
        return std::nullopt;
    }
    
    std::string token = jwt_->generate_token(user->id());
    
    return LoginResponse{user, token};
}

std::shared_ptr<models::User> UserService::get_user_by_id(int id) {
    return user_repo_->find_by_id(id);
}

std::shared_ptr<models::User> UserService::get_user_by_email(const std::string& email) {
    return user_repo_->find_by_email(email);
}

bool UserService::update_user(std::shared_ptr<models::User> user) {
    if(!user) return false;
    
    if(!validate_email(user->email())) {
        throw std::invalid_argument("Invalid email format");
    }
    
    if(user->nickname().empty()) {
        throw std::invalid_argument("Nickname is required");
    }
    
    return user_repo_->update(user);
}

// JSON deserialization
void service::from_json(const nlohmann::json& j, UserRegisterRequest& req) {
    j["email"].get_to(req.email);
    j["password"].get_to(req.password);
    j["nickname"].get_to(req.nickname);
}

void service::from_json(const nlohmann::json& j, UserLoginRequest& req) {
    j["email"].get_to(req.email);
    j["password"].get_to(req.password);
}

} // namespace service
