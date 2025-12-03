#ifndef PARKING_CONTROLLERS_H
#define PARKING_CONTROLLERS_H

#include "services.h"
#include <httplib.h>
#include <nlohmann/json.hpp>

using namespace httplib;
using json = nlohmann::json;

// HTTP响应工具类
class ResponseUtils {
public:
    // 成功响应
    static void send_success(Response& res, int code = 200, const json& data = nullptr);

    // 错误响应
    static void send_error(Response& res, int status_code, const std::string& message, int error_code = -1);

    // 服务错误响应
    static void send_service_error(Response& res, const ServiceError& err);

    // 参数验证错误响应
    static void send_validation_error(Response& res, const std::string& message);
};

// 认证中间件
class AuthMiddleware {
private:
    UserService& user_service_;

public:
    AuthMiddleware(UserService& user_service) : user_service_(user_service) {}

    // 验证Token，返回用户ID或空
    std::optional<int> authenticate(const Request& req) const;

    // 需要登录的中间件处理
    bool require_auth(const Request& req, Response& res) const;
};

// 用户控制器
class UserController {
private:
    UserService& user_service_;
    AuthMiddleware& auth_middleware_;

public:
    UserController(UserService& user_service, AuthMiddleware& auth_middleware)
        : user_service_(user_service), auth_middleware_(auth_middleware) {}

    // 注册路由
    void register_routes(Server& server);

    // 用户注册
    void register_user(const Request& req, Response& res);

    // 用户登录
    void login(const Request& req, Response& res);

    // 获取当前用户信息
    void get_current_user(const Request& req, Response& res);

    // 用户登出
    void logout(const Request& req, Response& res);
};

// 停车位控制器
class ParkingSpotController {
private:
    ParkingSpotService& spot_service_;
    AuthMiddleware& auth_middleware_;

public:
    ParkingSpotController(ParkingSpotService& spot_service, AuthMiddleware& auth_middleware)
        : spot_service_(spot_service), auth_middleware_(auth_middleware) {}

    // 注册路由
    void register_routes(Server& server);

    // 创建停车位
    void create_spot(const Request& req, Response& res);

    // 获取当前用户的停车位
    void get_my_spots(const Request& req, Response& res);

    // 获取停车位详情
    void get_spot(const Request& req, Response& res);

    // 更新停车位
    void update_spot(const Request& req, Response& res);

    // 停用停车位
    void deactivate_spot(const Request& req, Response& res);

    // 搜索停车位
    void search_spots(const Request& req, Response& res);
};

// 预约控制器
class ReservationController {
private:
    ReservationService& reservation_service_;
    AuthMiddleware& auth_middleware_;

public:
    ReservationController(ReservationService& reservation_service, AuthMiddleware& auth_middleware)
        : reservation_service_(reservation_service), auth_middleware_(auth_middleware) {}

    // 注册路由
    void register_routes(Server& server);

    // 创建预约
    void create_reservation(const Request& req, Response& res);

    // 获取我的预约（作为租客）
    void get_my_reservations(const Request& req, Response& res);

    // 获取我收到的预约（作为车主）
    void get_owner_reservations(const Request& req, Response& res);

    // 取消预约
    void cancel_reservation(const Request& req, Response& res);

    // 完成预约
    void finish_reservation(const Request& req, Response& res);
};

#endif // PARKING_CONTROLLERS_H
