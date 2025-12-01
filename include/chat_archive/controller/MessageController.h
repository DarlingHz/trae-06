#pragma once

#include "chat_archive/service/MessageService.h"
#include <httplib.h>
#include <nlohmann/json.hpp>

namespace chat_archive {
namespace controller {

using json = nlohmann::json;

// 消息控制器类
class MessageController {
public:
    explicit MessageController(service::MessageService& message_service)
        : message_service_(message_service) {}
    ~MessageController() = default;
    
    // 初始化路由
    void init_routes(httplib::Server& server);
    
private:
    service::MessageService& message_service_;
    // 处理创建消息请求
    void handle_create_message(const httplib::Request& req, httplib::Response& res);
    
    // 处理获取会话消息列表请求
    void handle_get_conversation_messages(const httplib::Request& req, httplib::Response& res);
    
    // 处理获取单个消息请求
    void handle_get_message(const httplib::Request& req, httplib::Response& res);
    
    // 处理更新消息请求
    void handle_update_message(const httplib::Request& req, httplib::Response& res);
    
    // 处理删除消息请求
    void handle_delete_message(const httplib::Request& req, httplib::Response& res);
    
    // 处理搜索消息请求
    void handle_search_messages(const httplib::Request& req, httplib::Response& res);
    
    // 发送成功响应
    void send_success_response(httplib::Response& res, const json& data);
    
    // 发送错误响应
    void send_error_response(httplib::Response& res, int status_code, const std::string& error_code, const std::string& message);
    

};

} // namespace controller
} // namespace chat_archive