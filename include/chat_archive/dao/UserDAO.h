#pragma once

#include "BaseDAO.h"
#include "chat_archive/model/User.h"
#include <vector>
#include <optional>

namespace chat_archive {
namespace dao {

class UserDAO : public BaseDAO {
public:
    UserDAO() = default;
    ~UserDAO() override = default;
    
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
    // 从数据库结果中构建用户对象
    model::User build_user_from_result(const DatabaseResult& result);
};

} // namespace dao
} // namespace chat_archive