#pragma once

#include "BaseDAO.h"
#include "chat_archive/model/Conversation.h"
#include "chat_archive/model/User.h"
#include <vector>
#include <optional>

namespace chat_archive {
namespace dao {

class ConversationDAO : public BaseDAO {
public:
    ConversationDAO() = default;
    ~ConversationDAO() override = default;
    
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
    // 从数据库结果中构建会话对象
    model::Conversation build_conversation_from_result(const DatabaseResult& result);
    
    // 添加会话参与者
    bool add_conversation_participants(int64_t conversation_id, const std::vector<int64_t>& participant_ids);
};

} // namespace dao
} // namespace chat_archive