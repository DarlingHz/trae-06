#ifndef RESERVATION_CONTROLLER_H
#define RESERVATION_CONTROLLER_H

#include <string>
#include <memory>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include "Controller.h"
#include "../service/ReservationService.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class ReservationController : public Controller {
public:
    /**
     * 构造函数
     * @param address HTTP服务器地址
     */
    ReservationController(const std::string& address);
    
    /**
     * 析构函数
     */
    ~ReservationController();
    
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
     * 预约服务类实例
     */
    std::shared_ptr<ReservationService> reservation_service;
    
    /**
     * 处理预约图书请求
     * @param request HTTP请求
     */
    void handleReserveBook(http_request request);
    
    /**
     * 处理取消预约请求
     * @param request HTTP请求
     */
    void handleCancelReservation(http_request request);
    
    /**
     * 处理获取用户预约记录请求
     * @param request HTTP请求
     */
    void handleGetUserReservations(http_request request);
    
    /**
     * 处理获取图书预约记录请求
     * @param request HTTP请求
     */
    void handleGetBookReservations(http_request request);
    
    /**
     * 处理获取所有预约记录请求
     * @param request HTTP请求
     */
    void handleGetAllReservations(http_request request);
    
    /**
     * 处理确认预约请求
     * @param request HTTP请求
     */
    void handleConfirmReservation(http_request request);
    
    /**
     * 处理获取图书预约队列长度请求
     * @param request HTTP请求
     */
    void handleGetBookReservationQueueLength(http_request request);
    
    /**
     * 处理获取用户预约队列位置请求
     * @param request HTTP请求
     */
    void handleGetUserReservationQueuePosition(http_request request);
    
    /**
     * 处理获取预约记录请求
     * @param request HTTP请求
     */
    void handleGetReservationRecord(http_request request);
    
    /**
     * 处理扫描过期预约记录请求
     * @param request HTTP请求
     */
    void handleScanExpiredReservationRecords(http_request request);
    
    /**
     * 验证用户身份
     * @param request HTTP请求
     * @param user_id 用户ID
     * @param role 用户角色
     * @return 验证是否通过
     */
    bool authenticateUser(http_request request, int& user_id, std::string& role);
    
    /**
     * 发送HTTP响应
     * @param request HTTP请求
     * @param status HTTP状态码
     * @param code 业务代码
     * @param message 业务消息
     * @param data 业务数据
     */
    void sendResponse(http_request request, http::status_code status, int code, const std::string& message, const web::json::value& data = web::json::value::object());
};

#endif // RESERVATION_CONTROLLER_H
