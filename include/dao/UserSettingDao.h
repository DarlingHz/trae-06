#ifndef USERSETTINGDAO_H
#define USERSETTINGDAO_H

#include <sqlite3.h>
#include <string>
#include "model/UserSetting.h"

namespace dao {

class UserSettingDao {
public:
    UserSettingDao(sqlite3* db);
    ~UserSettingDao();

    // 创建用户设置表
    bool createTable() const;
    // 插入或更新用户设置
    bool upsertUserSetting(const model::UserSetting& setting) const;
    // 根据用户ID查询用户设置
    model::UserSetting findUserSettingByUserId(int user_id) const;

private:
    sqlite3* db_;
};

} // namespace dao

#endif // USERSETTINGDAO_H
