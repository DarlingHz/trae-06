#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include <string>
#include <map>
#include <any>
#include "services/user_service.h"
#include "utils/json.h"
#include "utils/logger.h"
#include "models/user.h"

class UserController {
private:
    UserService& user_service_;
    Json& json_;
    Logger& logger_;

    UserController() : user_service_(UserService::getInstance()), json_(Json::getInstance()), logger_(Logger::getInstance()) {}
    ~UserController() = default;

public:
    static UserController& getInstance();

    // 用户注册
    std::string registerUser(const std::string& request_body);

    // 用户登录
    std::string loginUser(const std::string& request_body);

    // 获取用户信息
    std::string getUserInfo(const std::string& token);
};

#endif // USER_CONTROLLER_H
