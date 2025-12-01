#include "chat_archive/controller/MessageController.h"
#include "chat_archive/Logger.h"
#include <sstream>

namespace chat_archive {
namespace controller {

void MessageController::init_routes(httplib::Server& server) {
    // 注册路由
    server.Post("/api/conversations/(\\d+)/messages", [this](const httplib::Request& req, httplib::Response& res) {
        this->handle_create_message(req, res);
    });
    
    server.Get("/api/conversations/(\\d+)/messages", [this](const httplib::Request& req, httplib::Response& res) {
        this->handle_get_conversation_messages(req, res);
    });
    
    server.Get("/api/messages/(\\d+)", [this](const httplib::Request& req, httplib::Response& res) {
        this->handle_get_message(req, res);
    });
    
    server.Put("/api/messages/(\\d+)", [this](const httplib::Request& req, httplib::Response& res) {
        this->handle_update_message(req, res);
    });
    
    server.Delete("/api/messages/(\\d+)", [this](const httplib::Request& req, httplib::Response& res) {
        this->handle_delete_message(req, res);
    });
    
    server.Get("/api/search/messages", [this](const httplib::Request& req, httplib::Response& res) {
        this->handle_search_messages(req, res);
    });
    
    CHAT_ARCHIVE_LOG_INFO("MessageController routes initialized");
}

void MessageController::handle_create_message(const httplib::Request& req, httplib::Response& res) {
    CHAT_ARCHIVE_LOG_INFO("Received POST request for /api/conversations/{}/messages", std::string(req.matches[1]));
    
    try {
        // 解析URL参数中的会话ID
        int64_t conversation_id = std::stoll(req.matches[1]);
        
        // 解析JSON请求体
        json request_body = json::parse(req.body);
        
        // 验证请求体中的字段
        if (!request_body.contains("sender_id") || !request_body["sender_id"].is_number_integer()) {
            send_error_response(res, 400, "INVALID_REQUEST", "Missing or invalid 'sender_id' field");
            return;
        }
        
        int64_t sender_id = request_body["sender_id"].get<int64_t>();
        
        if (!request_body.contains("content") || !request_body["content"].is_string()) {
            send_error_response(res, 400, "INVALID_REQUEST", "Missing or invalid 'content' field");
            return;
        }
        
        std::string content = request_body["content"];
        
        std::optional<std::string> sent_at;
        if (request_body.contains("sent_at")) {
            if (!request_body["sent_at"].is_string()) {
                send_error_response(res, 400, "INVALID_REQUEST", "Invalid 'sent_at' field");
                return;
            }
            sent_at = request_body["sent_at"];
        }
        
        // 调用MessageService创建消息
        auto message_id = message_service_.create_message(conversation_id, sender_id, content, sent_at);
        
        if (!message_id) {
            send_error_response(res, 400, "MESSAGE_CREATION_FAILED", "Failed to create message");
            return;
        }
        
        // 构建成功响应
        json response_data;
        response_data["id"] = *message_id;
        response_data["conversation_id"] = conversation_id;
        response_data["sender_id"] = sender_id;
        response_data["content"] = content;
        
        send_success_response(res, response_data);
        
        CHAT_ARCHIVE_LOG_INFO("Message created successfully with ID: {}", *message_id);
        
    } catch (const json::parse_error& e) {
        CHAT_ARCHIVE_LOG_WARN("Invalid JSON format for create message request: {}", e.what());
        send_error_response(res, 400, "INVALID_JSON", "Invalid JSON format");
    } catch (const std::invalid_argument& e) {
        CHAT_ARCHIVE_LOG_WARN("Invalid conversation ID format: {}", std::string(req.matches[1]));
        send_error_response(res, 400, "INVALID_CONVERSATION_ID", "Invalid conversation ID format");
    } catch (const std::exception& e) {
        CHAT_ARCHIVE_LOG_ERROR("Error handling create message request: {}", e.what());
        send_error_response(res, 500, "INTERNAL_ERROR", "Internal server error");
    }
}

void MessageController::handle_get_conversation_messages(const httplib::Request& req, httplib::Response& res) {
    CHAT_ARCHIVE_LOG_INFO("Received GET request for /api/conversations/{}/messages", std::string(req.matches[1]));
    
    try {
        // 解析URL参数中的会话ID
        int64_t conversation_id = std::stoll(req.matches[1]);
        
        // 解析查询参数
        int limit = 100;
        int offset = 0;
        std::string order = "asc";
        
        if (req.has_param("limit")) {
            std::string limit_str = req.get_param_value("limit");
            limit = std::stoi(limit_str);
        }
        
        if (req.has_param("offset")) {
            std::string offset_str = req.get_param_value("offset");
            offset = std::stoi(offset_str);
        }
        
        if (req.has_param("order")) {
            std::string order_str = req.get_param_value("order");
            if (order_str == "desc") {
                order = "desc";
            }
        }
        
        // 调用MessageService获取会话消息列表
        auto messages = message_service_.get_conversation_messages(conversation_id, limit, offset, order);
        
        // 构建成功响应
        json response_data = json::array();
        
        for (const auto& message : messages) {
            json message_data;
            message_data["id"] = message.get_id();
            message_data["conversation_id"] = message.get_conversation_id();
            message_data["sender_id"] = message.get_sender_id();
            message_data["content"] = message.get_content();
            std::time_t sent_at_time = std::chrono::system_clock::to_time_t(message.get_sent_at());
            std::tm sent_at_tm = *std::localtime(&sent_at_time);
            std::stringstream sent_at_ss;
            sent_at_ss << std::put_time(&sent_at_tm, "%Y-%m-%d %H:%M:%S");
            message_data["sent_at"] = sent_at_ss.str();
            if (message.get_edited_at()) {
                  std::time_t edited_at_time = std::chrono::system_clock::to_time_t(*message.get_edited_at());
                  std::tm edited_at_tm = *std::localtime(&edited_at_time);
                  std::stringstream edited_at_ss;
                  edited_at_ss << std::put_time(&edited_at_tm, "%Y-%m-%d %H:%M:%S");
                  message_data["edited_at"] = edited_at_ss.str();
              }
            
            response_data.push_back(message_data);
        }
        
        send_success_response(res, response_data);
        
        CHAT_ARCHIVE_LOG_INFO("Conversation messages retrieved successfully: {} messages", messages.size());
        
    } catch (const std::invalid_argument& e) {
        CHAT_ARCHIVE_LOG_WARN("Invalid conversation ID format: {}", std::string(req.matches[1]));
        send_error_response(res, 400, "INVALID_CONVERSATION_ID", "Invalid conversation ID format");
    } catch (const std::exception& e) {
        CHAT_ARCHIVE_LOG_ERROR("Error handling get conversation messages request: {}", e.what());
        send_error_response(res, 500, "INTERNAL_ERROR", "Internal server error");
    }
}

void MessageController::handle_get_message(const httplib::Request& req, httplib::Response& res) {
    CHAT_ARCHIVE_LOG_INFO("Received GET request for /api/messages/{}", std::string(req.matches[1]));
    
    try {
        // 解析URL参数中的消息ID
        int64_t message_id = std::stoll(req.matches[1]);
        
        // 调用MessageService获取消息
        auto message = message_service_.get_message_by_id(message_id);
        
        if (!message) {
            send_error_response(res, 404, "MESSAGE_NOT_FOUND", "Message not found");
            return;
        }
        
        // 构建成功响应
        json response_data;
        response_data["id"] = message->get_id();
        response_data["conversation_id"] = message->get_conversation_id();
        response_data["sender_id"] = message->get_sender_id();
        response_data["content"] = message->get_content();
        std::time_t sent_at_time = std::chrono::system_clock::to_time_t(message->get_sent_at());
        std::tm sent_at_tm = *std::localtime(&sent_at_time);
        std::stringstream sent_at_ss;
        sent_at_ss << std::put_time(&sent_at_tm, "%Y-%m-%d %H:%M:%S");
        response_data["sent_at"] = sent_at_ss.str();
        if (message->get_edited_at()) {
              std::time_t edited_at_time = std::chrono::system_clock::to_time_t(*message->get_edited_at());
              std::tm edited_at_tm = *std::localtime(&edited_at_time);
              std::stringstream edited_at_ss;
              edited_at_ss << std::put_time(&edited_at_tm, "%Y-%m-%d %H:%M:%S");
              response_data["edited_at"] = edited_at_ss.str();
          }
        
        send_success_response(res, response_data);
        
        CHAT_ARCHIVE_LOG_INFO("Message retrieved successfully with ID: {}", message_id);
        
    } catch (const std::invalid_argument& e) {
        CHAT_ARCHIVE_LOG_WARN("Invalid message ID format: {}", std::string(req.matches[1]));
        send_error_response(res, 400, "INVALID_MESSAGE_ID", "Invalid message ID format");
    } catch (const std::exception& e) {
        CHAT_ARCHIVE_LOG_ERROR("Error handling get message request: {}", e.what());
        send_error_response(res, 500, "INTERNAL_ERROR", "Internal server error");
    }
}

void MessageController::handle_update_message(const httplib::Request& req, httplib::Response& res) {
    CHAT_ARCHIVE_LOG_INFO("Received PUT request for /api/messages/{}", std::string(req.matches[1]));
    
    try {
        // 解析URL参数中的消息ID
        int64_t message_id = std::stoll(req.matches[1]);
        
        // 解析JSON请求体
        json request_body = json::parse(req.body);
        
        // 验证请求体中的字段
        if (!request_body.contains("content") || !request_body["content"].is_string()) {
            send_error_response(res, 400, "INVALID_REQUEST", "Missing or invalid 'content' field");
            return;
        }
        
        std::string content = request_body["content"];
        
        // 调用MessageService更新消息
        bool success = message_service_.update_message(message_id, content);
        
        if (!success) {
            send_error_response(res, 400, "MESSAGE_UPDATE_FAILED", "Failed to update message");
            return;
        }
        
        // 构建成功响应
        json response_data;
        response_data["id"] = message_id;
        response_data["content"] = content;
        
        send_success_response(res, response_data);
        
        CHAT_ARCHIVE_LOG_INFO("Message updated successfully with ID: {}", message_id);
        
    } catch (const json::parse_error& e) {
        CHAT_ARCHIVE_LOG_WARN("Invalid JSON format for update message request: {}", e.what());
        send_error_response(res, 400, "INVALID_JSON", "Invalid JSON format");
    } catch (const std::invalid_argument& e) {
        CHAT_ARCHIVE_LOG_WARN("Invalid message ID format: {}", std::string(req.matches[1]));
        send_error_response(res, 400, "INVALID_MESSAGE_ID", "Invalid message ID format");
    } catch (const std::exception& e) {
        CHAT_ARCHIVE_LOG_ERROR("Error handling update message request: {}", e.what());
        send_error_response(res, 500, "INTERNAL_ERROR", "Internal server error");
    }
}

void MessageController::handle_delete_message(const httplib::Request& req, httplib::Response& res) {
    CHAT_ARCHIVE_LOG_INFO("Received DELETE request for /api/messages/{}", std::string(req.matches[1]));
    
    try {
        // 解析URL参数中的消息ID
        int64_t message_id = std::stoll(req.matches[1]);
        
        // 调用MessageService删除消息
        bool success = message_service_.delete_message(message_id);
        
        if (!success) {
            send_error_response(res, 400, "MESSAGE_DELETE_FAILED", "Failed to delete message");
            return;
        }
        
        // 构建成功响应
        json response_data;
        response_data["id"] = message_id;
        response_data["deleted"] = true;
        
        send_success_response(res, response_data);
        
        CHAT_ARCHIVE_LOG_INFO("Message deleted successfully with ID: {}", message_id);
        
    } catch (const std::invalid_argument& e) {
        CHAT_ARCHIVE_LOG_WARN("Invalid message ID format: {}", std::string(req.matches[1]));
        send_error_response(res, 400, "INVALID_MESSAGE_ID", "Invalid message ID format");
    } catch (const std::exception& e) {
        CHAT_ARCHIVE_LOG_ERROR("Error handling delete message request: {}", e.what());
        send_error_response(res, 500, "INTERNAL_ERROR", "Internal server error");
    }
}

void MessageController::handle_search_messages(const httplib::Request& req, httplib::Response& res) {
    CHAT_ARCHIVE_LOG_INFO("Received GET request for /api/search/messages");
    
    try {
        // 解析查询参数
        service::MessageSearchParams params;
        
        if (req.has_param("keyword")) {
            params.keyword = req.get_param_value("keyword");
        }
        
        if (req.has_param("user_id")) {
            std::string user_id_str = req.get_param_value("user_id");
            params.user_id = std::stoll(user_id_str);
        }
        
        if (req.has_param("conversation_id")) {
            std::string conversation_id_str = req.get_param_value("conversation_id");
            params.conversation_id = std::stoll(conversation_id_str);
        }
        
        if (req.has_param("from")) {
            params.from = req.get_param_value("from");
        }
        
        if (req.has_param("to")) {
            params.to = req.get_param_value("to");
        }
        
        if (req.has_param("limit")) {
            std::string limit_str = req.get_param_value("limit");
            params.limit = std::stoi(limit_str);
        }
        
        if (req.has_param("offset")) {
            std::string offset_str = req.get_param_value("offset");
            params.offset = std::stoi(offset_str);
        }
        
        // 调用MessageService搜索消息
        auto search_result = message_service_.search_messages(params);
        
        // 构建成功响应
        json response_data;
        json messages_data = json::array();
        
        for (const auto& message : search_result.messages) {
            json message_data;
            message_data["id"] = message.get_id();
            message_data["conversation_id"] = message.get_conversation_id();
            message_data["sender_id"] = message.get_sender_id();
            message_data["content"] = message.get_content();
            std::time_t sent_at_time = std::chrono::system_clock::to_time_t(message.get_sent_at());
            std::tm sent_at_tm = *std::localtime(&sent_at_time);
            std::stringstream sent_at_ss;
            sent_at_ss << std::put_time(&sent_at_tm, "%Y-%m-%d %H:%M:%S");
            message_data["sent_at"] = sent_at_ss.str();
            if (message.get_edited_at()) {
                  std::time_t edited_at_time = std::chrono::system_clock::to_time_t(*message.get_edited_at());
                  std::tm edited_at_tm = *std::localtime(&edited_at_time);
                  std::stringstream edited_at_ss;
                  edited_at_ss << std::put_time(&edited_at_tm, "%Y-%m-%d %H:%M:%S");
                  message_data["edited_at"] = edited_at_ss.str();
              }
            
            messages_data.push_back(message_data);
        }
        
        response_data["messages"] = messages_data;
        response_data["total_count"] = search_result.total_count;
        
        send_success_response(res, response_data);
        
        CHAT_ARCHIVE_LOG_INFO("Message search completed, found {} messages", search_result.total_count);
        
    } catch (const std::invalid_argument& e) {
        CHAT_ARCHIVE_LOG_WARN("Invalid query parameters for search messages request: {}", e.what());
        send_error_response(res, 400, "INVALID_PARAMETERS", "Invalid query parameters");
    } catch (const std::exception& e) {
        CHAT_ARCHIVE_LOG_ERROR("Error handling search messages request: {}", e.what());
        send_error_response(res, 500, "INTERNAL_ERROR", "Internal server error");
    }
}

void MessageController::send_success_response(httplib::Response& res, const json& data) {
    json response;
    response["data"] = data;
    
    res.status = 200;
    res.set_content(response.dump(), "application/json");
}

void MessageController::send_error_response(httplib::Response& res, int status_code, const std::string& error_code, const std::string& message) {
    json response;
    response["error_code"] = error_code;
    response["message"] = message;
    
    res.status = status_code;
    res.set_content(response.dump(), "application/json");
}

} // namespace controller
} // namespace chat_archive