#pragma once

#include "chat_archive/Database.h"
#include <memory>

namespace chat_archive {
namespace dao {

class BaseDAO {
public:
    BaseDAO() = default;
    virtual ~BaseDAO() = default;
    
    // 获取数据库连接
    std::shared_ptr<sqlite3> get_connection() {
        return DatabasePool::get().get_connection();
    }
    
    // 释放数据库连接
    void release_connection(std::shared_ptr<sqlite3> conn) {
        DatabasePool::get().release_connection(conn);
    }
    
protected:
    // 执行查询并返回结果
    DatabaseResult execute_query(const std::string& sql, 
                                 const std::vector<std::pair<int, std::string>>& string_params = {},
                                 const std::vector<std::pair<int, int>>& int_params = {},
                                 const std::vector<std::pair<int, int64_t>>& int64_params = {});
    
    // 执行更新（插入、更新、删除）并返回受影响的行数
    int64_t execute_update(const std::string& sql, 
                           const std::vector<std::pair<int, std::string>>& string_params = {},
                           const std::vector<std::pair<int, int>>& int_params = {},
                           const std::vector<std::pair<int, int64_t>>& int64_params = {});
};

} // namespace dao
} // namespace chat_archive