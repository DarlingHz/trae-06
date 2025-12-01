#include "chat_archive/service/MessageService.h"
#include "chat_archive/Logger.h"
#include <regex>
#include <sstream>
#include <iomanip>

namespace chat_archive {
namespace service {

std::optional<int64_t> MessageService::create_message(int64_t conversation_id,
                                                           int64_t sender_id,
                                                           const std::string& content,
                                                           const std::optional<std::string>& sent_at) {
    // 验证会话ID
    if (conversation_id <= 0) {
        CHAT_ARCHIVE_LOG_WARN("Invalid conversation ID: {}", conversation_id);
        return std::nullopt;
    }
    
    // 验证发送者ID
    if (sender_id <= 0) {
        CHAT_ARCHIVE_LOG_WARN("Invalid sender ID: {}", sender_id);
        return std::nullopt;
    }
    
    // 验证消息内容
    if (!validate_message_content(content)) {
        CHAT_ARCHIVE_LOG_WARN("Invalid message content");
        return std::nullopt;
    }
    
    // 验证发送者是否是会话参与者
    if (!validate_sender_in_conversation(conversation_id, sender_id)) {
        CHAT_ARCHIVE_LOG_WARN("Sender is not a participant in the conversation");
        return std::nullopt;
    }
    
    // 解析发送时间
    std::optional<std::chrono::system_clock::time_point> sent_at_time;
    if (sent_at) {
        sent_at_time = parse_time_string(*sent_at);
        if (!sent_at_time) {
            CHAT_ARCHIVE_LOG_WARN("Invalid sent_at time format: {}", *sent_at);
            return std::nullopt;
        }
    }
    
    // 创建消息
    auto message_id = message_dao_.create_message(conversation_id, sender_id, content, sent_at_time);
    if (!message_id) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to create message");
        return std::nullopt;
    }
    
    CHAT_ARCHIVE_LOG_INFO("Successfully created message with ID: {}, conversation ID: {}", 
                           *message_id, conversation_id);
    return message_id;
}

std::optional<model::Message> MessageService::get_message_by_id(int64_t id) {
    if (id <= 0) {
        CHAT_ARCHIVE_LOG_WARN("Invalid message ID: {}", id);
        return std::nullopt;
    }
    
    auto message = message_dao_.get_message_by_id(id);
    if (!message) {
        CHAT_ARCHIVE_LOG_DEBUG("Message not found with ID: {}", id);
        return std::nullopt;
    }
    
    // 不返回已删除的消息
    if (message->is_deleted()) {
        CHAT_ARCHIVE_LOG_DEBUG("Message is deleted with ID: {}", id);
        return std::nullopt;
    }
    
    return message;
}

std::vector<model::Message> MessageService::get_conversation_messages(int64_t conversation_id,
                                                                           int limit,
                                                                           int offset,
                                                                           const std::string& order) {
    // 验证会话ID
    if (conversation_id <= 0) {
        CHAT_ARCHIVE_LOG_WARN("Invalid conversation ID: {}", conversation_id);
        return {};
    }
    
    // 验证分页参数
    if (limit <= 0) {
        limit = 100;
    } else if (limit > 1000) {
        limit = 1000; // 限制最大返回数量
    }
    
    if (offset < 0) {
        offset = 0;
    }
    
    // 验证排序参数
    bool valid_order = (order != "desc"); // true表示升序，false表示降序
    
    auto messages = message_dao_.get_conversation_messages(conversation_id, limit, offset, valid_order);
    return messages;
}

bool MessageService::update_message(int64_t id, const std::string& content) {
    // 验证消息ID
    if (id <= 0) {
        CHAT_ARCHIVE_LOG_WARN("Invalid message ID: {}", id);
        return false;
    }
    
    // 验证消息内容
    if (!validate_message_content(content)) {
        CHAT_ARCHIVE_LOG_WARN("Invalid message content");
        return false;
    }
    
    // 检查消息是否存在且未被删除
    auto message = message_dao_.get_message_by_id(id);
    if (!message || message->is_deleted()) {
        CHAT_ARCHIVE_LOG_WARN("Message not found or deleted with ID: {}", id);
        return false;
    }
    
    // 更新消息
    bool success = message_dao_.update_message(id, content);
    if (!success) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to update message with ID: {}", id);
        return false;
    }
    
    CHAT_ARCHIVE_LOG_INFO("Successfully updated message with ID: {}", id);
    return true;
}

bool MessageService::delete_message(int64_t id) {
    // 验证消息ID
    if (id <= 0) {
        CHAT_ARCHIVE_LOG_WARN("Invalid message ID: {}", id);
        return false;
    }
    
    // 检查消息是否存在且未被删除
    auto message = message_dao_.get_message_by_id(id);
    if (!message || message->is_deleted()) {
        CHAT_ARCHIVE_LOG_WARN("Message not found or already deleted with ID: {}", id);
        return false;
    }
    
    // 删除消息（软删除）
    bool success = message_dao_.delete_message(id);
    if (!success) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to delete message with ID: {}", id);
        return false;
    }
    
    CHAT_ARCHIVE_LOG_INFO("Successfully deleted message with ID: {}", id);
    return true;
}

MessageSearchResult MessageService::search_messages(const MessageSearchParams& params) {
    MessageSearchResult result;
    
    // 验证搜索参数
    MessageSearchParams valid_params = params;
    
    // 验证分页参数
    if (valid_params.limit <= 0) {
        valid_params.limit = 100;
    } else if (valid_params.limit > 1000) {
        valid_params.limit = 1000; // 限制最大返回数量
    }
    
    if (valid_params.offset < 0) {
        valid_params.offset = 0;
    }
    
    // 验证时间参数
    std::optional<std::chrono::system_clock::time_point> from_time;
    if (valid_params.from) {
        from_time = parse_time_string(*valid_params.from);
        if (!from_time) {
            CHAT_ARCHIVE_LOG_WARN("Invalid from time format: {}", *valid_params.from);
            return result;
        }
    }
    
    std::optional<std::chrono::system_clock::time_point> to_time;
    if (valid_params.to) {
        to_time = parse_time_string(*valid_params.to);
        if (!to_time) {
            CHAT_ARCHIVE_LOG_WARN("Invalid to time format: {}", *valid_params.to);
            return result;
        }
    }
    
    // 执行搜索
    chat_archive::dao::MessageSearchParams search_params;
    search_params.keyword = valid_params.keyword;
    search_params.user_id = valid_params.user_id;
    search_params.conversation_id = valid_params.conversation_id;
    search_params.from = from_time;
    search_params.to = to_time;
    search_params.limit = valid_params.limit;
    search_params.offset = valid_params.offset;
    auto search_result = message_dao_.search_messages(search_params);
    
    result.messages = search_result.messages;
    result.total_count = search_result.total_count;
    
    CHAT_ARCHIVE_LOG_INFO("Message search completed, found {} messages", search_result.total_count);
    return result;
}

bool MessageService::validate_message_content(const std::string& content) {
    // 消息内容不能为空
    if (content.empty()) {
        CHAT_ARCHIVE_LOG_WARN("Message content is empty");
        return false;
    }
    
    // 消息内容长度检查
    if (content.length() > 10000) {
        CHAT_ARCHIVE_LOG_WARN("Message content is too long: {} characters", content.length());
        return false;
    }
    
    return true;
}

bool MessageService::validate_sender_in_conversation(int64_t conversation_id, int64_t sender_id) {
    // 获取会话的参与者
    auto participants = conversation_dao_.get_conversation_participants(conversation_id);
    
    // 检查发送者是否在参与者列表中
    for (const auto& participant : participants) {
        if (participant.get_id() == sender_id) {
            return true;
        }
    }
    
    return false;
}

std::optional<std::chrono::system_clock::time_point> MessageService::parse_time_string(const std::string& time_str) {
    // 支持ISO 8601格式：YYYY-MM-DDTHH:MM:SSZ
    std::regex time_regex("^(\\d{4})-(\\d{2})-(\\d{2})T(\\d{2}):(\\d{2}):(\\d{2})Z$");
    std::smatch match;
    
    if (std::regex_match(time_str, match, time_regex)) {
        try {
            int year = std::stoi(match[1]);
            int month = std::stoi(match[2]);
            int day = std::stoi(match[3]);
            int hour = std::stoi(match[4]);
            int minute = std::stoi(match[5]);
            int second = std::stoi(match[6]);
            
            // 创建tm结构体
            std::tm tm = {};
            tm.tm_year = year - 1900; // 年份从1900开始
            tm.tm_mon = month - 1;     // 月份从0开始
            tm.tm_mday = day;
            tm.tm_hour = hour;
            tm.tm_min = minute;
            tm.tm_sec = second;
            
            // 转换为time_t
            std::time_t t = std::mktime(&tm);
            if (t != -1) {
                // 转换为system_clock::time_point
                return std::chrono::system_clock::from_time_t(t);
            }
        } catch (const std::exception& e) {
            CHAT_ARCHIVE_LOG_WARN("Failed to parse time string: {}", e.what());
        }
    }
    
    // 如果解析失败，返回空
    return std::nullopt;
}

} // namespace service
} // namespace chat_archive