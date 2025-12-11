#ifndef SESSION_REPOSITORY_H
#define SESSION_REPOSITORY_H

#include "BaseRepository.h"
#include "../model/Session.h"

namespace repository {

// SessionRepository接口，定义会话相关的数据访问方法
class SessionRepository : public virtual BaseRepository<model::Session> {
public:
    virtual ~SessionRepository() = default;

    // 根据token查找会话
    virtual std::optional<model::Session> findByToken(const std::string& token) = 0;

    // 根据用户ID查找会话
    virtual std::vector<model::Session> findByUserId(int user_id) = 0;

    // 删除过期的会话
    virtual bool deleteExpired() = 0;

    // 删除某个用户的所有会话
    virtual bool deleteByUserId(int user_id) = 0;
};

// 创建SessionRepository实例的工厂函数
std::unique_ptr<SessionRepository> createSessionRepository(const std::string& db_path);

} // namespace repository

#endif // SESSION_REPOSITORY_H