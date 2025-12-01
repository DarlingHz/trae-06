#ifndef USERDAO_H
#define USERDAO_H

#include <sqlite3.h>
#include <string>
#include "model/User.h"

namespace dao {

class UserDao {
public:
    UserDao(sqlite3* db);
    ~UserDao();

    // 创建用户表
    bool createTable() const;
    // 插入新用户
    bool insertUser(const model::User& user) const;
    // 根据邮箱查询用户
    model::User findUserByEmail(const std::string& email) const;
    // 根据ID查询用户
    model::User findUserById(int id) const;

private:
    sqlite3* db_;
};

} // namespace dao

#endif // USERDAO_H
