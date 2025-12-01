#include "chat_archive/service/ConversationService.h"
#include "chat_archive/Logger.h"
#include <algorithm>

namespace chat_archive {
namespace service {

std::optional<int64_t> ConversationService::create_conversation(const std::optional<std::string>& title,
                                                                     const std::vector<int64_t>& participant_ids) {
    // 验证会话标题
    if (!validate_conversation_title(title)) {
        CHAT_ARCHIVE_LOG_WARN("Invalid conversation title");
        return std::nullopt;
    }
    
    // 验证参与者列表
    if (!validate_participants(participant_ids)) {
        CHAT_ARCHIVE_LOG_WARN("Invalid conversation participants");
        return std::nullopt;
    }
    
    // 创建会话
    auto conversation_id = conversation_dao_.create_conversation(title, participant_ids);
    if (!conversation_id) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to create conversation");
        return std::nullopt;
    }
    
    CHAT_ARCHIVE_LOG_INFO("Successfully created conversation with ID: {}, title: {}", 
                           *conversation_id, title ? *title : "(no title)");
    return conversation_id;
}

std::optional<model::Conversation> ConversationService::get_conversation_by_id(int64_t id) {
    if (id <= 0) {
        CHAT_ARCHIVE_LOG_WARN("Invalid conversation ID: {}", id);
        return std::nullopt;
    }
    
    auto conversation = conversation_dao_.get_conversation_by_id(id);
    if (!conversation) {
        CHAT_ARCHIVE_LOG_DEBUG("Conversation not found with ID: {}", id);
        return std::nullopt;
    }
    
    return conversation;
}

std::vector<model::Conversation> ConversationService::get_conversations(int limit, int offset) {
    // 验证分页参数
    if (limit <= 0) {
        limit = 100;
    } else if (limit > 1000) {
        limit = 1000; // 限制最大返回数量
    }
    
    if (offset < 0) {
        offset = 0;
    }
    
    auto conversations = conversation_dao_.get_conversations(limit, offset);
    return conversations;
}

int64_t ConversationService::get_total_conversations() {
    return conversation_dao_.get_total_conversations();
}

std::vector<model::User> ConversationService::get_conversation_participants(int64_t conversation_id) {
    if (conversation_id <= 0) {
        CHAT_ARCHIVE_LOG_WARN("Invalid conversation ID: {}", conversation_id);
        return {};
    }
    
    auto participants = conversation_dao_.get_conversation_participants(conversation_id);
    return participants;
}

bool ConversationService::validate_conversation_title(const std::optional<std::string>& title) {
    // 标题可以为空
    if (!title) {
        return true;
    }
    
    // 标题长度检查
    const std::string& title_str = *title;
    if (title_str.length() > 200) {
        CHAT_ARCHIVE_LOG_WARN("Conversation title is too long: {}", title_str);
        return false;
    }
    
    return true;
}

bool ConversationService::validate_participants(const std::vector<int64_t>& participant_ids) {
    // 参与者数量检查
    if (participant_ids.empty()) {
        CHAT_ARCHIVE_LOG_WARN("Conversation must have at least one participant");
        return false;
    }
    
    // 参与者ID有效性检查
    for (int64_t user_id : participant_ids) {
        if (user_id <= 0) {
            CHAT_ARCHIVE_LOG_WARN("Invalid participant ID: {}", user_id);
            return false;
        }
        
        // 检查用户是否存在
        auto user = user_dao_.get_user_by_id(user_id);
        if (!user) {
            CHAT_ARCHIVE_LOG_WARN("Participant not found with ID: {}", user_id);
            return false;
        }
    }
    
    // 检查是否有重复的参与者
    std::vector<int64_t> unique_ids = participant_ids;
    std::sort(unique_ids.begin(), unique_ids.end());
    auto last = std::unique(unique_ids.begin(), unique_ids.end());
    
    if (last != unique_ids.end()) {
        CHAT_ARCHIVE_LOG_WARN("Conversation has duplicate participants");
        return false;
    }
    
    return true;
}

} // namespace service
} // namespace chat_archive