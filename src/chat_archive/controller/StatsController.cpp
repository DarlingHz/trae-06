#include "chat_archive/controller/StatsController.h"
#include "chat_archive/Logger.h"

namespace chat_archive {
namespace controller {

void StatsController::init_routes(httplib::Server& server) {
    // 注册路由
    server.Get("/api/stats/overview", [this](const httplib::Request& req, httplib::Response& res) {
        this->handle_get_stats_overview(req, res);
    });
    
    CHAT_ARCHIVE_LOG_INFO("StatsController routes initialized");
}

void StatsController::handle_get_stats_overview(const httplib::Request& req, httplib::Response& res) {
    CHAT_ARCHIVE_LOG_INFO("Received GET request for /api/stats/overview");
    
    try {
        // 调用StatsService获取概览统计信息
        auto stats = stats_service_.get_message_stats();
        auto total_users = stats_service_.get_total_users();
        auto total_conversations = stats_service_.get_total_conversations();
        auto top_senders = stats_service_.get_top_senders(10);
        
        // 构建成功响应
        nlohmann::json response_data;
        response_data["total_users"] = total_users;
        response_data["total_conversations"] = total_conversations;
        response_data["total_messages"] = stats.total_messages;
        response_data["messages_last_24h"] = stats.messages_last_24h;
        
        // 处理top_senders
        nlohmann::json top_senders_data = nlohmann::json::array();
        for (const auto& sender : top_senders) {
            nlohmann::json sender_data;
            sender_data["user_id"] = sender.user.get_id();
            sender_data["message_count"] = sender.message_count;
            top_senders_data.push_back(sender_data);
        }
        
        response_data["top_senders"] = top_senders_data;
        
        send_success_response(res, response_data);
        
        CHAT_ARCHIVE_LOG_INFO("Stats overview retrieved successfully");
        
    } catch (const std::exception& e) {
        CHAT_ARCHIVE_LOG_ERROR("Error handling get stats overview request: {}", e.what());
        send_error_response(res, 500, "INTERNAL_ERROR", "Internal server error");
    }
}

void StatsController::send_success_response(httplib::Response& res, const nlohmann::json& data) {
    nlohmann::json response;
    response["data"] = data;
    
    res.status = 200;
    res.set_content(response.dump(), "application/json");
}

void StatsController::send_error_response(httplib::Response& res, int status_code, 
                                             const std::string& error_code, const std::string& message) {
    nlohmann::json response;
    response["error_code"] = error_code;
    response["message"] = message;
    
    res.status = status_code;
    res.set_content(response.dump(), "application/json");
}

} // namespace controller
} // namespace chat_archive