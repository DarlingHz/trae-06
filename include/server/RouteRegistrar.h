#pragma once

#include "server/HTTPServer.h"

namespace pet_hospital {

class RouteRegistrar {
public:
    RouteRegistrar(HTTPServer& server);
    ~RouteRegistrar();

    // 注册所有路由
    void register_all_routes();

private:
    // 注册用户相关路由
    void register_user_routes();

    // 注册宠物相关路由
    void register_pet_routes();

    // 注册医生相关路由
    void register_doctor_routes();

    // 注册预约相关路由
    void register_appointment_routes();

    // 注册病例记录相关路由
    void register_record_routes();

    // 注册健康检查路由
    void register_health_check_routes();

private:
    HTTPServer& server_;
};

} // namespace pet_hospital
