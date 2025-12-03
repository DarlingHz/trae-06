#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include <controller/BaseController.h>
#include <cpp-httplib/httplib.h>

namespace event_signup_service::controller {

class UserController : public BaseController {
public:
    using BaseController::BaseController;

    // 创建用户
    void create_user(const httplib::Request& req, httplib::Response& res);

    // 获取用户详情
    void get_user(const httplib::Request& req, httplib::Response& res);

    // 获取用户报名记录
    void get_user_registrations(const httplib::Request& req, httplib::Response& res);
};

} // namespace event_signup_service::controller

#endif // USER_CONTROLLER_H
