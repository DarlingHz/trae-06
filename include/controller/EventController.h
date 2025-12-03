#ifndef EVENT_CONTROLLER_H
#define EVENT_CONTROLLER_H

#include <controller/BaseController.h>
#include <cpp-httplib/httplib.h>

namespace event_signup_service::controller {

class EventController : public BaseController {
public:
    using BaseController::BaseController;

    // 创建活动
    void create_event(const httplib::Request& req, httplib::Response& res);

    // 更新活动
    void update_event(const httplib::Request& req, httplib::Response& res);

    // 获取活动详情
    void get_event(const httplib::Request& req, httplib::Response& res);

    // 获取活动列表
    void get_events(const httplib::Request& req, httplib::Response& res);

    // 获取活动统计信息
    void get_event_stats(const httplib::Request& req, httplib::Response& res);

    // 获取活动报名列表
    void get_event_registrations(const httplib::Request& req, httplib::Response& res);

    // 活动报名
    void register_for_event(const httplib::Request& req, httplib::Response& res);

    // 取消报名
    void cancel_registration(const httplib::Request& req, httplib::Response& res);

    // 活动签到
    void check_in(const httplib::Request& req, httplib::Response& res);
};

} // namespace event_signup_service::controller

#endif // EVENT_CONTROLLER_H
