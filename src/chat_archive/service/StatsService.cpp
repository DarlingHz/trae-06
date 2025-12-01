#include "chat_archive/service/StatsService.h"
#include "chat_archive/Logger.h"
#include <chrono>

namespace chat_archive {
namespace service {

int64_t StatsService::get_total_users() {
    int64_t total_users = user_dao_.get_total_users();
    CHAT_ARCHIVE_LOG_DEBUG("Total users: {}", total_users);
    return total_users;
}

int64_t StatsService::get_total_conversations() {
    int64_t total_conversations = conversation_dao_.get_total_conversations();
    CHAT_ARCHIVE_LOG_DEBUG("Total conversations: {}", total_conversations);
    return total_conversations;
}

MessageStats StatsService::get_message_stats() {
    MessageStats stats;
    
    // 获取总消息数
    stats.total_messages = message_dao_.get_total_messages();
    
    // 获取最近24小时内的消息数
    stats.messages_last_24h = get_messages_last_24h();
    
    CHAT_ARCHIVE_LOG_DEBUG("Total messages: {}, messages in last 24h: {}", 
                           stats.total_messages, stats.messages_last_24h);
    
    return stats;
}

std::vector<SenderStats> StatsService::get_top_senders(int limit) {
    std::vector<SenderStats> top_senders;
    
    // 验证限制参数
    if (limit <= 0) {
        limit = 10;
    } else if (limit > 100) {
        limit = 100; // 限制最大返回数量
    }
    
    // 获取前N个发送者的统计信息
    auto sender_stats = message_dao_.get_top_senders(limit);
    
    // 为每个发送者获取用户信息
    for (const auto& [user, message_count] : sender_stats) {
        SenderStats stats;
        stats.user = user;
        stats.message_count = message_count;
        top_senders.push_back(stats);
    }
    
    CHAT_ARCHIVE_LOG_DEBUG("Top {} senders retrieved", top_senders.size());
    
    return top_senders;
}

int64_t StatsService::get_messages_last_24h() {
    // 获取最近24小时内的消息数量
    int64_t messages_last_24h = message_dao_.get_messages_last_24h();
    
    return messages_last_24h;
}

} // namespace service
} // namespace chat_archive