#pragma once

#include "chat_archive/service/ConversationService.h"
#include <httplib.h>
#include <nlohmann/json.hpp>

namespace chat_archive {
namespace controller {

using json = nlohmann::json;

// 会话控制器类
class ConversationController {
public:
    explicit ConversationController(service::ConversationService& conversation_service)
        : conversation_service_(conversation_service) {}
    ~ConversationController() = default;
    
    // 初始化路由
    void init_routes(httplib::Server& server);
    
private:
    service::ConversationService& conversation_service_;
    // 处理创建会话请求
    void handle_create_conversation(const httplib::Request& req, httplib::Response& res);
    
    // 处理获取会话列表请求
    void handle_get_conversations(const httplib::Request& req, httplib::Response& res);
    
    // 处理获取单个会话请求
    void handle_get_conversation(const httplib::Request& req, httplib::Response& res);
    
    // 发送成功响应
    void send_success_response(httplib::Response& res, const json& data);
    
    // 发送错误响应
    void send_error_response(httplib::Response& res, int status_code, const std::string& error_code, const std::string& message);
    

};

} // namespace controller
} // namespace chat_archive