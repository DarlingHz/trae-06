#pragma once

#include "repository/BaseRepository.h"
#include "model/User.h"

namespace repository {

class UserRepository : public BaseRepository {
public:
    explicit UserRepository(const std::string& db_path) : BaseRepository(db_path) {
        createTable();
    }

    // 创建新用户
    model::User createUser(const std::string& username, const std::string& password_hash);

    // 根据用户名查找用户
    std::optional<model::User> findUserByUsername(const std::string& username);

    // 根据用户 ID 查找用户
    std::optional<model::User> findUserById(int user_id);

private:
    // 创建用户表
    void createTable();
};

} // namespace repository