#include "chat_archive/controller/ConversationController.h"
#include "chat_archive/Logger.h"
#include <sstream>

namespace chat_archive {
namespace controller {

void ConversationController::init_routes(httplib::Server& server) {
    // 注册路由
    server.Post("/api/conversations", [this](const httplib::Request& req, httplib::Response& res) {
        this->handle_create_conversation(req, res);
    });
    
    server.Get("/api/conversations", [this](const httplib::Request& req, httplib::Response& res) {
        this->handle_get_conversations(req, res);
    });
    
    server.Get("/api/conversations/(\\d+)", [this](const httplib::Request& req, httplib::Response& res) {
        this->handle_get_conversation(req, res);
    });
    
    CHAT_ARCHIVE_LOG_INFO("ConversationController routes initialized");
}

void ConversationController::handle_create_conversation(const httplib::Request& req, httplib::Response& res) {
    CHAT_ARCHIVE_LOG_INFO("Received POST request for /api/conversations");
    
    try {
        // 解析JSON请求体
        json request_body = json::parse(req.body);
        
        // 验证请求体中的字段
        std::optional<std::string> title;
        if (request_body.contains("title")) {
            if (!request_body["title"].is_string()) {
                send_error_response(res, 400, "INVALID_REQUEST", "Invalid 'title' field");
                return;
            }
            title = request_body["title"];
        }
        
        if (!request_body.contains("participant_ids") || !request_body["participant_ids"].is_array()) {
            send_error_response(res, 400, "INVALID_REQUEST", "Missing or invalid 'participant_ids' field");
            return;
        }
        
        std::vector<int64_t> participant_ids;
        for (const auto& id : request_body["participant_ids"]) {
            if (!id.is_number_integer()) {
                send_error_response(res, 400, "INVALID_REQUEST", "Invalid participant ID");
                return;
            }
            participant_ids.push_back(id.get<int64_t>());
        }
        
        // 调用ConversationService创建会话
        auto conversation_id = conversation_service_.create_conversation(title, participant_ids);
        
        if (!conversation_id) {
            send_error_response(res, 400, "CONVERSATION_CREATION_FAILED", "Failed to create conversation");
            return;
        }
        
        // 构建成功响应
        json response_data;
        response_data["id"] = *conversation_id;
        if (title) {
            response_data["title"] = *title;
        }
        response_data["participant_ids"] = participant_ids;
        
        send_success_response(res, response_data);
        
        CHAT_ARCHIVE_LOG_INFO("Conversation created successfully with ID: {}", *conversation_id);
        
    } catch (const json::parse_error& e) {
        CHAT_ARCHIVE_LOG_WARN("Invalid JSON format for create conversation request: {}", e.what());
        send_error_response(res, 400, "INVALID_JSON", "Invalid JSON format");
    } catch (const std::exception& e) {
        CHAT_ARCHIVE_LOG_ERROR("Error handling create conversation request: {}", e.what());
        send_error_response(res, 500, "INTERNAL_ERROR", "Internal server error");
    }
}

void ConversationController::handle_get_conversations(const httplib::Request& req, httplib::Response& res) {
    CHAT_ARCHIVE_LOG_INFO("Received GET request for /api/conversations");
    
    try {
        // 解析查询参数
        int limit = 100;
        int offset = 0;
        
        if (req.has_param("limit")) {
            std::string limit_str = req.get_param_value("limit");
            limit = std::stoi(limit_str);
        }
        
        if (req.has_param("offset")) {
            std::string offset_str = req.get_param_value("offset");
            offset = std::stoi(offset_str);
        }
        
        // 调用ConversationService获取会话列表
        auto conversations = conversation_service_.get_conversations(limit, offset);
        
        // 构建成功响应
        json response_data = json::array();
        
        for (const auto& conversation : conversations) {
            json conversation_data;
            conversation_data["id"] = conversation.get_id();
            if (conversation.get_title()) {
                conversation_data["title"] = *conversation.get_title();
            }
            // 将time_point转换为字符串
            auto created_at = conversation.get_created_at();
            auto time_t_created_at = std::chrono::system_clock::to_time_t(created_at);
            std::tm tm_created_at = *std::localtime(&time_t_created_at);
            std::stringstream ss;
            ss << std::put_time(&tm_created_at, "%Y-%m-%d %H:%M:%S");
            conversation_data["created_at"] = ss.str();
            
            // 获取会话参与者
            auto participants = conversation_service_.get_conversation_participants(conversation.get_id());
            json participant_data = json::array();
            for (const auto& participant : participants) {
                json user_data;
                user_data["id"] = participant.get_id();
                user_data["name"] = participant.get_name();
                participant_data.push_back(user_data);
            }
            conversation_data["participants"] = participant_data;
            
            response_data.push_back(conversation_data);
        }
        
        send_success_response(res, response_data);
        
        CHAT_ARCHIVE_LOG_INFO("Conversations retrieved successfully: {} conversations", conversations.size());
        
    } catch (const std::invalid_argument& e) {
        CHAT_ARCHIVE_LOG_WARN("Invalid query parameters for get conversations request: {}", e.what());
        send_error_response(res, 400, "INVALID_PARAMETERS", "Invalid query parameters");
    } catch (const std::exception& e) {
        CHAT_ARCHIVE_LOG_ERROR("Error handling get conversations request: {}", e.what());
        send_error_response(res, 500, "INTERNAL_ERROR", "Internal server error");
    }
}

void ConversationController::handle_get_conversation(const httplib::Request& req, httplib::Response& res) {
    CHAT_ARCHIVE_LOG_INFO("Received GET request for /api/conversations/{}", std::string(req.matches[1]));
    
    try {
        // 解析URL参数中的会话ID
        int64_t conversation_id = std::stoll(req.matches[1]);
        
        // 调用ConversationService获取会话
        auto conversation = conversation_service_.get_conversation_by_id(conversation_id);
        
        if (!conversation) {
            send_error_response(res, 404, "CONVERSATION_NOT_FOUND", "Conversation not found");
            return;
        }
        
        // 构建成功响应
        json response_data;
        response_data["id"] = conversation->get_id();
        if (conversation->get_title()) {
            response_data["title"] = *conversation->get_title();
        }
        // 将time_point转换为字符串
        auto created_at = conversation->get_created_at();
        auto time_t_created_at = std::chrono::system_clock::to_time_t(created_at);
        std::tm tm_created_at = *std::localtime(&time_t_created_at);
        std::stringstream ss;
        ss << std::put_time(&tm_created_at, "%Y-%m-%d %H:%M:%S");
        response_data["created_at"] = ss.str();
        
        // 获取会话参与者
        auto participants = conversation_service_.get_conversation_participants(conversation->get_id());
        json participant_data = json::array();
        for (const auto& participant : participants) {
            json user_data;
            user_data["id"] = participant.get_id();
            user_data["name"] = participant.get_name();
            participant_data.push_back(user_data);
        }
        response_data["participants"] = participant_data;
        
        send_success_response(res, response_data);
        
        CHAT_ARCHIVE_LOG_INFO("Conversation retrieved successfully with ID: {}", conversation_id);
        
    } catch (const std::invalid_argument& e) {
        CHAT_ARCHIVE_LOG_WARN("Invalid conversation ID format: {}", std::string(req.matches[1]));
        send_error_response(res, 400, "INVALID_CONVERSATION_ID", "Invalid conversation ID format");
    } catch (const std::exception& e) {
        CHAT_ARCHIVE_LOG_ERROR("Error handling get conversation request: {}", e.what());
        send_error_response(res, 500, "INTERNAL_ERROR", "Internal server error");
    }
}

void ConversationController::send_success_response(httplib::Response& res, const json& data) {
    json response;
    response["data"] = data;
    
    res.status = 200;
    res.set_content(response.dump(), "application/json");
}

void ConversationController::send_error_response(httplib::Response& res, int status_code, const std::string& error_code, const std::string& message) {
    json response;
    response["error_code"] = error_code;
    response["message"] = message;
    
    res.status = status_code;
    res.set_content(response.dump(), "application/json");
}

} // namespace controller
} // namespace chat_archive