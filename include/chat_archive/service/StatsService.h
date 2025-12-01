#pragma once

#include "chat_archive/model/User.h"
#include "chat_archive/dao/UserDAO.h"
#include "chat_archive/dao/ConversationDAO.h"
#include "chat_archive/dao/MessageDAO.h"
#include <vector>
#include <optional>

namespace chat_archive {
namespace service {

// 消息统计信息
struct MessageStats {
    int64_t total_messages = 0;
    int64_t messages_last_24h = 0;
};

// 发送者统计信息
struct SenderStats {
    model::User user;
    int64_t message_count = 0;
};

// 统计服务类
class StatsService {
public:
    StatsService() = default;
    ~StatsService() = default;
    
    // 获取总用户数
    int64_t get_total_users();
    
    // 获取总会话数
    int64_t get_total_conversations();
    
    // 获取消息统计信息
    MessageStats get_message_stats();
    
    // 获取消息数量最多的前N个用户
    std::vector<SenderStats> get_top_senders(int limit = 10);
    
private:
    // 获取最近24小时内的消息数量
    int64_t get_messages_last_24h();
    
    dao::UserDAO user_dao_;
    dao::ConversationDAO conversation_dao_;
    dao::MessageDAO message_dao_;
};

} // namespace service
} // namespace chat_archive