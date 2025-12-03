#include "utils/jwt.h"
#include <jwt/jwt.h>
#include <string>
#include <stdexcept>
#include <chrono>

JWT::JWT() {
    // 从配置文件读取密钥和过期时间
    config_ = Config::getInstance();
    secret_key_ = config_->getString("jwt.secret_key", "your-secret-key-change-this");
    token_expiry_ = config_->getInt("jwt.token_expiry_hours", 24);
}

JWT& JWT::getInstance() {
    static JWT instance;
    return instance;
}

JWT::~JWT() {}

std::string JWT::generateToken(int user_id, const std::string& username) {
    try {
        auto now = std::chrono::system_clock::now();
        auto expiry = now + std::chrono::hours(token_expiry_);

        auto token = jwt::create()
            .set_issuer("tech_qa_backend")
            .set_issued_at(now)
            .set_expires_at(expiry)
            .set_payload_claim("user_id", jwt::claim(std::to_string(user_id)))
            .set_payload_claim("username", jwt::claim(username))
            .sign(jwt::algorithm::hs256(secret_key_));

        return token;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to generate token: " + std::string(e.what()));
    }
}

bool JWT::verifyToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .with_issuer("tech_qa_backend")
            .allow_algorithm(jwt::algorithm::hs256(secret_key_));

        verifier.verify(decoded);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

int JWT::getUserIdFromToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto user_id_claim = decoded.get_payload_claim("user_id");
        return std::stoi(user_id_claim.as_string());
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get user ID from token: " + std::string(e.what()));
    }
}

std::string JWT::getUsernameFromToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto username_claim = decoded.get_payload_claim("username");
        return username_claim.as_string();
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get username from token: " + std::string(e.what()));
    }
}
