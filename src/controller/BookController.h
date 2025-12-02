#ifndef BOOK_CONTROLLER_H
#define BOOK_CONTROLLER_H

#include <string>
#include <memory>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include "Controller.h"
#include "../service/BookService.h"
#include "../service/UserService.h"
#include "../service/BorrowService.h"
#include "../service/ReservationService.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class BookController : public Controller {
public:
    /**
     * 构造函数
     * @param address HTTP服务器地址
     */
    BookController(const utility::string_t& url);
    
    /**
     * 析构函数
     */
    ~BookController();
    
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
     * 图书服务类实例
     */
    std::shared_ptr<BookService> book_service;
    
    /**
     * 用户服务类实例
     */
    std::shared_ptr<UserService> user_service;
    
    /**
     * 借阅服务类实例
     */
    std::shared_ptr<BorrowService> borrow_service;
    
    /**
     * 预约服务类实例
     */
    std::shared_ptr<ReservationService> reservation_service;
    
    /**
     * 处理GET请求
     * @param request HTTP请求
     */
    void handleGet(http_request request);
    
    /**
     * 处理POST请求
     * @param request HTTP请求
     */
    void handlePost(http_request request);
    
    /**
     * 处理PUT请求
     * @param request HTTP请求
     */
    void handlePut(http_request request);
    
    /**
     * 处理DELETE请求
     * @param request HTTP请求
     */
    void handleDelete(http_request request);
    
    /**
     * 处理获取所有图书请求
     * @param request HTTP请求
     */
    void handleGetAllBooks(http_request request);
    
    /**
     * 处理根据ID获取图书请求
     * @param request HTTP请求
     * @param book_id 图书ID
     */
    void handleGetBookById(http_request request, const utility::string_t& book_id);
    
    /**
     * 处理新增图书请求
     * @param request HTTP请求
     */
    void handleAddBook(http_request request);
    
    /**
     * 处理更新图书请求
     * @param request HTTP请求
     * @param book_id 图书ID
     */
    void handleUpdateBook(http_request request, const utility::string_t& book_id);
    
    /**
     * 处理删除图书请求
     * @param request HTTP请求
     * @param book_id 图书ID
     */
    void handleDeleteBook(http_request request, const utility::string_t& book_id);
    
    /**
     * 处理搜索图书请求
     * @param request HTTP请求
     */
    void handleSearchBooks(http_request request);
    
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
     * 处理预约图书请求
     * @param request HTTP请求
     */
    void handleReserveBook(http_request request);
    
    /**
     * 序列化图书对象为JSON
     * @param book 图书对象
     * @return JSON对象
     */
    web::json::value serializeBook(const std::shared_ptr<Book>& book);
    
    /**
     * 发送HTTP响应
     * @param request HTTP请求
     * @param status HTTP状态码
     * @param code 自定义响应码
     * @param message 响应消息
     * @param data 响应数据（可选）
     */
    void sendResponse(http_request request, status_code status, int code, const std::string& message, web::json::value data = web::json::value::null());
};

#endif // BOOK_CONTROLLER_H