#include "parking/services.h"
#include "parking/utils.h"
#include "parking/config.h"
#include <stdexcept>
#include <string>

// UserService实现
User UserService::register_user(const std::string& name, const std::string& email, const std::string& password) {
    // 参数验证
    if (name.empty()) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Name cannot be empty");
    }
    if (email.empty()) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Email cannot be empty");
    }
    if (password.empty()) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Password cannot be empty");
    }

    // 检查用户是否已存在
    if (user_dao_->find_by_email(email)) {
        throw ServiceError(ServiceError::Type::USER_ALREADY_EXISTS, "Email already registered");
    }

    // 创建用户
    User user;
    user.name = name;
    user.email = email;
    user.password_hash = PasswordHasher::hash(password);
    user.status = UserStatus::ACTIVE;
    user.created_at = std::time(nullptr);
    user.updated_at = user.created_at;

    try {
        user.id = user_dao_->create(user);
        return user;
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to create user: " + std::string(e.what()));
    }
}

std::pair<User, std::string> UserService::login(const std::string& email, const std::string& password) {
    // 参数验证
    if (email.empty()) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Email cannot be empty");
    }
    if (password.empty()) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Password cannot be empty");
    }

    // 查找用户
    auto user_opt = user_dao_->find_by_email(email);
    if (!user_opt) {
        throw ServiceError(ServiceError::Type::USER_NOT_FOUND, "User not found");
    }

    User user = *user_opt;

    // 验证密码
    if (!PasswordHasher::verify(password, user.password_hash)) {
        throw ServiceError(ServiceError::Type::INVALID_CREDENTIALS, "Invalid password");
    }

    // 清理过期会话
    session_dao_->cleanup_expired();

    // 创建新会话
    std::string token = TokenGenerator::generate();
    Session session;
    session.token = token;
    session.user_id = user.id;
    session.expires_at = std::time(nullptr) + Config::instance().token_expiration();
    session.created_at = std::time(nullptr);

    try {
        session_dao_->create(session);
        return {user, token};
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to create session: " + std::string(e.what()));
    }
}

std::optional<User> UserService::validate_token(const std::string& token) {
    if (token.empty()) {
        return std::nullopt;
    }

    try {
        // 清理过期会话
        session_dao_->cleanup_expired();

        // 查找会话
        auto session_opt = session_dao_->find_by_token(token);
        if (!session_opt) {
            return std::nullopt;
        }

        Session session = *session_opt;

        // 检查Token是否过期
        std::time_t now = std::time(nullptr);
        if (session.expires_at < now) {
            // 删除过期会话
            session_dao_->delete_by_token(token);
            return std::nullopt;
        }

        // 查找用户
        auto user_opt = user_dao_->find_by_id(session.user_id);
        return user_opt;
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to validate token: " + std::string(e.what()));
    }
}

void UserService::logout(const std::string& token) {
    if (token.empty()) {
        return;
    }

    try {
        session_dao_->delete_by_token(token);
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to logout: " + std::string(e.what()));
    }
}

std::optional<User> UserService::get_user(int id) const {
    try {
        return user_dao_->find_by_id(id);
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to get user: " + std::string(e.what()));
    }
}
