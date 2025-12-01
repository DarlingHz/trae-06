#ifndef USERSETTINGCONTROLLER_H
#define USERSETTINGCONTROLLER_H

#include <nlohmann/json.hpp>
#include "dao/UserSettingDao.h"

using json = nlohmann::json;

namespace controller {

class UserSettingController {
public:
    UserSettingController(dao::UserSettingDao& user_setting_dao);
    ~UserSettingController();

    // 处理获取用户设置请求
    json handleGet(int user_id) const;
    // 处理更新用户设置请求
    json handleUpdate(const json& request, int user_id) const;

private:
    dao::UserSettingDao& user_setting_dao_;
};

} // namespace controller

#endif // USERSETTINGCONTROLLER_H
