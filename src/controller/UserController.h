#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include <string>
#include <memory>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include "Controller.h"
#include "../service/UserService.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class UserController : public Controller {
public:
    /**
     * 构造函数
     * @param address HTTP服务器地址
     */
    UserController(const std::string& address);
    
    /**
     * 析构函数
     */
    ~UserController();
    
    /**
     * 启动HTTP服务器
     */
    void start();
    
    /**
     * 停止HTTP服务器
     */
    void stop();
    
private:
    /**
     * HTTP服务器监听器
     */
    http_listener listener;
    
    /**
     * 用户服务类实例
     */
    std::shared_ptr<UserService> user_service;
    
    /**
     * 处理用户注册请求
     * @param request HTTP请求
     */
    void handleRegister(http_request request);
    
    /**
     * 处理用户登录请求
     * @param request HTTP请求
     */
    void handleLogin(http_request request);
    
    /**
     * 处理更新用户信息请求
     * @param request HTTP请求
     */
    void handleUpdateUserInfo(http_request request);
    
    /**
     * 处理更新用户密码请求
     * @param request HTTP请求
     */
    void handleUpdateUserPassword(http_request request);
    
    /**
     * 处理获取用户信息请求
     * @param request HTTP请求
     */
    void handleGetUserInfo(http_request request);
    
    /**
     * 处理获取所有用户请求
     * @param request HTTP请求
     */
    void handleGetAllUsers(http_request request);
    
    /**
     * 处理切换用户状态请求
     * @param request HTTP请求
     */
    void handleToggleUserStatus(http_request request);
    
    /**
     * 验证用户身份
     * @param request HTTP请求
     * @param user_id 用户ID（输出参数）
     * @param role 用户角色（输出参数）
     * @return 验证成功返回true，否则返回false
     */
    bool authenticateUser(http_request request, int& user_id, std::string& role);
    
    /**
     * 发送HTTP响应
     * @param request HTTP请求
     * @param status HTTP状态码
     * @param code 业务错误码
     * @param message 业务错误信息
     * @param data 业务数据
     */
    void sendResponse(http_request request, http::status_code status, int code, const std::string& message, const web::json::value& data = web::json::value::object());
};

#endif // USER_CONTROLLER_H