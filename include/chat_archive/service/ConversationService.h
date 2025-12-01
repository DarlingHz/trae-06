#pragma once

#include "chat_archive/model/Conversation.h"
#include "chat_archive/model/User.h"
#include "chat_archive/dao/ConversationDAO.h"
#include "chat_archive/dao/UserDAO.h"
#include <vector>
#include <optional>

namespace chat_archive {
namespace service {

// 会话服务类
class ConversationService {
public:
    ConversationService() = default;
    ~ConversationService() = default;
    
    // 创建会话
    std::optional<int64_t> create_conversation(const std::optional<std::string>& title,
                                                   const std::vector<int64_t>& participant_ids);
    
    // 根据ID获取会话
    std::optional<model::Conversation> get_conversation_by_id(int64_t id);
    
    // 分页获取会话列表
    std::vector<model::Conversation> get_conversations(int limit = 100, int offset = 0);
    
    // 获取会话总数
    int64_t get_total_conversations();
    
    // 获取会话的参与者
    std::vector<model::User> get_conversation_participants(int64_t conversation_id);
    
private:
    // 验证会话标题
    bool validate_conversation_title(const std::optional<std::string>& title);
    
    // 验证参与者列表
    bool validate_participants(const std::vector<int64_t>& participant_ids);
    
    dao::ConversationDAO conversation_dao_;
    dao::UserDAO user_dao_;
};

} // namespace service
} // namespace chat_archive