#include "SessionService.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <functional>

namespace service {

// 创建新会话
std::optional<model::Session> SessionService::createSession(
    int user_id,
    const std::chrono::system_clock::duration& expire_duration) {
    try {
        // 业务逻辑验证
        if (user_id <= 0) {
            spdlog::error("Invalid user ID: {}", user_id);
            return std::nullopt;
        }

        // 检查用户是否存在
        std::optional<model::User> user = user_service_->findUserById(user_id);
        if (!user) {
            spdlog::error("User not found: ID = {}", user_id);
            return std::nullopt;
        }

        // 生成唯一的会话token
        std::string token = generateToken();

        // 创建会话对象
        model::Session session;
        session.setUserId(user_id);
        session.setToken(token);
        session.setExpireAt(std::chrono::system_clock::now() + expire_duration);
        session.setCreatedAt(std::chrono::system_clock::now());

        // 保存会话到数据库
        int session_id = session_repository_->create(session);
        if (session_id <= 0) {
            spdlog::error("Failed to create session for user ID = {}", user_id);
            return std::nullopt;
        }

        // 设置会话ID
        session.setId(session_id);

        spdlog::info("Session created successfully for user ID = {}", user_id);
        return session;
    } catch (const std::exception& e) {
        spdlog::error("Error creating session: {}", e.what());
        return std::nullopt;
    }
}

// 根据ID查找会话
std::optional<model::Session> SessionService::findSessionById(int id) {
    try {
        if (id <= 0) {
            spdlog::error("Invalid session ID: {}", id);
            return std::nullopt;
        }

        std::optional<model::Session> session = session_repository_->findById(id);
        if (!session) {
            spdlog::error("Session not found: ID = {}", id);
            return std::nullopt;
        }

        // 检查会话是否过期
        if (session->isExpired()) {
            spdlog::error("Session expired: ID = {}", id);
            return std::nullopt;
        }

        return session;
    } catch (const std::exception& e) {
        spdlog::error("Error finding session by ID: {}", e.what());
        return std::nullopt;
    }
}

// 根据token查找会话
std::optional<model::Session> SessionService::findSessionByToken(const std::string& token) {
    try {
        if (token.empty()) {
            spdlog::error("Session token cannot be empty");
            return std::nullopt;
        }

        std::optional<model::Session> session = session_repository_->findByToken(token);
        if (!session) {
            spdlog::error("Session not found: Token = {}", token);
            return std::nullopt;
        }

        // 检查会话是否过期
        if (session->isExpired()) {
            spdlog::error("Session expired: Token = {}", token);
            return std::nullopt;
        }

        return session;
    } catch (const std::exception& e) {
        spdlog::error("Error finding session by token: {}", e.what());
        return std::nullopt;
    }
}

// 根据用户ID查找会话
std::vector<model::Session> SessionService::findSessionsByUserId(int user_id) {
    try {
        if (user_id <= 0) {
            spdlog::error("Invalid user ID: {}", user_id);
            return std::vector<model::Session>();
        }

        // 检查用户是否存在
        std::optional<model::User> user = user_service_->findUserById(user_id);
        if (!user) {
            spdlog::error("User not found: ID = {}", user_id);
            return std::vector<model::Session>();
        }

        std::vector<model::Session> sessions = session_repository_->findByUserId(user_id);

        // 过滤掉过期的会话
        std::vector<model::Session> valid_sessions;
        for (const auto& session : sessions) {
            if (!session.isExpired()) {
                valid_sessions.push_back(session);
            }
        }

        spdlog::info("Retrieved valid sessions for user ID = {}: Total = {}", user_id, valid_sessions.size());
        return valid_sessions;
    } catch (const std::exception& e) {
        spdlog::error("Error retrieving sessions by user ID: {}", e.what());
        return std::vector<model::Session>();
    }
}

// 检查会话是否有效
bool SessionService::isSessionValid(const std::string& token) {
    try {
        if (token.empty()) {
            spdlog::error("Session token cannot be empty");
            return false;
        }

        std::optional<model::Session> session = session_repository_->findByToken(token);
        if (!session) {
            spdlog::error("Session not found: Token = {}", token);
            return false;
        }

        // 检查会话是否过期
        if (session->isExpired()) {
            spdlog::error("Session expired: Token = {}", token);
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error checking session validity: {}", e.what());
        return false;
    }
}

// 刷新会话过期时间
bool SessionService::refreshSession(const std::string& token, const std::chrono::system_clock::duration& expire_duration) {
    try {
        if (token.empty()) {
            spdlog::error("Session token cannot be empty");
            return false;
        }

        std::optional<model::Session> session = session_repository_->findByToken(token);
        if (!session) {
            spdlog::error("Session not found: Token = {}", token);
            return false;
        }

        // 检查会话是否过期
        if (session->isExpired()) {
            spdlog::error("Session expired: Token = {}", token);
            return false;
        }

        // 更新会话过期时间
        model::Session updated_session = *session;
        updated_session.setExpireAt(std::chrono::system_clock::now() + expire_duration);

        if (!session_repository_->update(updated_session)) {
            spdlog::error("Failed to refresh session: Token = {}", token);
            return false;
        }

        spdlog::info("Session refreshed successfully: Token = {}", token);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error refreshing session: {}", e.what());
        return false;
    }
}

// 删除会话
bool SessionService::deleteSession(const std::string& token) {
    try {
        if (token.empty()) {
            spdlog::error("Session token cannot be empty");
            return false;
        }

        std::optional<model::Session> session = session_repository_->findByToken(token);
        if (!session) {
            spdlog::error("Session not found: Token = {}", token);
            return false;
        }

        if (!session_repository_->deleteById(session->getId())) {
            spdlog::error("Failed to delete session: Token = {}", token);
            return false;
        }

        spdlog::info("Session deleted successfully: Token = {}", token);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error deleting session: {}", e.what());
        return false;
    }
}

// 根据用户ID删除会话
bool SessionService::deleteSessionsByUserId(int user_id) {
    try {
        if (user_id <= 0) {
            spdlog::error("Invalid user ID: {}", user_id);
            return false;
        }

        // 检查用户是否存在
        std::optional<model::User> user = user_service_->findUserById(user_id);
        if (!user) {
            spdlog::error("User not found: ID = {}", user_id);
            return false;
        }

        if (!session_repository_->deleteByUserId(user_id)) {
            spdlog::error("Failed to delete sessions for user ID = {}", user_id);
            return false;
        }

        spdlog::info("Sessions deleted successfully for user ID = {}", user_id);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error deleting sessions by user ID: {}", e.what());
        return false;
    }
}

// 删除过期会话
int SessionService::deleteExpiredSessions() {
    try {
        int deleted_count = session_repository_->deleteExpired();
        spdlog::info("Expired sessions deleted successfully: Total = {}", deleted_count);
        return deleted_count;
    } catch (const std::exception& e) {
        spdlog::error("Error deleting expired sessions: {}", e.what());
        return 0;
    }
}

// 生成唯一的会话token
std::string SessionService::generateToken() {
    // 使用C++标准库中的随机数生成器和哈希函数来生成唯一的token
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    // 生成32字节的随机数据
    std::vector<uint8_t> random_data(32);
    for (int i = 0; i < 32; ++i) {
        random_data[i] = dis(gen);
    }

    // 使用SHA-1哈希函数对随机数据进行哈希（为了简单起见，使用C++标准库中的hash函数）
    std::hash<std::string> hash_fn;
    size_t hash_value = hash_fn(std::string(random_data.begin(), random_data.end()));

    // 将哈希值转换为十六进制字符串
    std::ostringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash_value;
    return ss.str();
}

} // namespace service