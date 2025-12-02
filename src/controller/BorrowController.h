#ifndef BORROW_CONTROLLER_H
#define BORROW_CONTROLLER_H

#include <string>
#include <memory>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include "Controller.h"
#include "../service/BorrowService.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class BorrowController : public Controller {
public:
    /**
     * 构造函数
     * @param address HTTP服务器地址
     */
    BorrowController(const std::string& address);
    
    /**
     * 析构函数
     */
    ~BorrowController();
    
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
     * 借阅服务类实例
     */
    std::shared_ptr<BorrowService> borrow_service;
    
    /**
     * 处理借阅图书请求
     * @param request HTTP请求
     */
    void handleBorrowBook(http_request request);
    
    /**
     * 处理归还图书请求
     * @param request HTTP请求
     */
    void handleReturnBook(http_request request);
    
    /**
     * 处理获取借阅记录请求
     * @param request HTTP请求
     */
    void handleGetBorrowRecord(http_request request);
    
    /**
     * 处理获取用户借阅记录请求
     * @param request HTTP请求
     */
    void handleGetUserBorrowRecords(http_request request);
    
    /**
     * 处理获取图书借阅记录请求
     * @param request HTTP请求
     */
    void handleGetBookBorrowRecords(http_request request);
    
    /**
     * 处理获取逾期借阅记录请求
     * @param request HTTP请求
     */
    void handleGetOverdueBorrowRecords(http_request request);
    
    /**
     * 处理扫描逾期借阅记录请求
     * @param request HTTP请求
     */
    void handleScanOverdueBorrowRecords(http_request request);
    
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

#endif // BORROW_CONTROLLER_H