#ifndef SESSION_SERVICE_H
#define SESSION_SERVICE_H

#include <string>
#include <optional>
#include <vector>
#include <chrono>
#include "../model/Session.h"
#include "../repository/SessionRepository.h"
#include "UserService.h"

namespace service {

// SessionService接口，定义会话相关的业务逻辑
class SessionService {
public:
    virtual ~SessionService() = default;

    // 构造函数，依赖于SessionRepository接口和UserService接口
    explicit SessionService(
        std::unique_ptr<repository::SessionRepository> session_repository,
        std::shared_ptr<UserService> user_service)
        : session_repository_(std::move(session_repository)),
          user_service_(std::move(user_service)) {}

    // 创建新会话
    // 参数：user_id - 用户ID
    //      expire_duration - 会话过期时间（可选，默认30天）
    // 返回：成功则返回创建的会话对象，失败则返回std::nullopt
    // 失败原因可能包括：用户不存在、参数无效等
    std::optional<model::Session> createSession(
        int user_id,
        const std::chrono::system_clock::duration& expire_duration = std::chrono::hours(24 * 30));

    // 根据ID查找会话
    // 参数：id - 会话ID
    // 返回：成功则返回会话对象，失败则返回std::nullopt
    std::optional<model::Session> findSessionById(int id);

    // 根据token查找会话
    // 参数：token - 会话token
    // 返回：成功则返回会话对象，失败则返回std::nullopt
    std::optional<model::Session> findSessionByToken(const std::string& token);

    // 根据用户ID查找会话
    // 参数：user_id - 用户ID
    // 返回：成功则返回会话列表，失败则返回空列表
    std::vector<model::Session> findSessionsByUserId(int user_id);

    // 获取UserService实例
    std::shared_ptr<UserService> getUserService() const {
        return user_service_;
    }

    // 检查会话是否有效
    // 参数：token - 会话token
    // 返回：有效则返回true，否则返回false
    bool isSessionValid(const std::string& token);

    // 刷新会话过期时间
    // 参数：token - 会话token
    //      expire_duration - 新的会话过期时间（可选，默认30天）
    // 返回：成功则返回true，失败则返回false
    bool refreshSession(const std::string& token, const std::chrono::system_clock::duration& expire_duration = std::chrono::hours(24 * 30));

    // 删除会话
    // 参数：token - 会话token
    // 返回：成功则返回true，失败则返回false
    bool deleteSession(const std::string& token);

    // 根据用户ID删除会话
    // 参数：user_id - 用户ID
    // 返回：成功则返回true，失败则返回false
    bool deleteSessionsByUserId(int user_id);

    // 删除过期会话
    // 返回：成功则返回删除的会话数量，失败则返回0
    int deleteExpiredSessions();

private:
    // 生成唯一的会话token
    // 返回：生成的token字符串
    virtual std::string generateToken();

    // 依赖注入的SessionRepository接口
    std::unique_ptr<repository::SessionRepository> session_repository_;

    // 依赖注入的UserService接口
    std::shared_ptr<UserService> user_service_;
};

} // namespace service

#endif // SESSION_SERVICE_H