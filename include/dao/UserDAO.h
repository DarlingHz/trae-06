#pragma once

#include <string>
#include <optional>
#include "BaseDAO.h"
#include "models/User.h"

namespace pet_hospital {

class UserDAO : public BaseDAO {
public:
    UserDAO() = default;
    ~UserDAO() override = default;

    // 创建用户
    bool create_user(const User& user);

    // 根据 ID 查询用户
    std::optional<User> get_user_by_id(int user_id);

    // 根据邮箱查询用户
    std::optional<User> get_user_by_email(const std::string& email);

    // 更新用户信息
    bool update_user(const User& user);

    // 删除用户
    bool delete_user(int user_id);
};

} // namespace pet_hospital
