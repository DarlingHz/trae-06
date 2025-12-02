#ifndef USER_REPOSITORY_H
#define USER_REPOSITORY_H

#include "BaseRepository.h"
#include "../model/User.h"

namespace repository {

// UserRepository接口，定义用户相关的数据访问方法
class UserRepository : public virtual BaseRepository<model::User> {
public:
    virtual ~UserRepository() = default;

    // 根据用户名查找用户
    virtual std::optional<model::User> findByUsername(const std::string& username) = 0;

    // 检查用户名是否存在
    virtual bool existsByUsername(const std::string& username) = 0;
};

// 创建UserRepository实例的工厂函数
std::unique_ptr<UserRepository> createUserRepository(const std::string& db_path);

} // namespace repository

#endif // USER_REPOSITORY_H