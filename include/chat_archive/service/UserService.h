#pragma once

#include "chat_archive/model/User.h"
#include "chat_archive/dao/UserDAO.h"
#include <vector>
#include <optional>

namespace chat_archive {
namespace service {

// 用户服务类
class UserService {
public:
    UserService() = default;
    ~UserService() = default;
    
    // 创建用户
    std::optional<int64_t> create_user(const std::string& name);
    
    // 根据ID获取用户
    std::optional<model::User> get_user_by_id(int64_t id);
    
    // 根据名称获取用户
    std::optional<model::User> get_user_by_name(const std::string& name);
    
    // 分页获取用户列表
    std::vector<model::User> get_users(int limit = 100, int offset = 0);
    
    // 获取用户总数
    int64_t get_total_users();
    
private:
    // 验证用户名
    bool validate_username(const std::string& name);
    
    dao::UserDAO user_dao_;
};

} // namespace service
} // namespace chat_archive