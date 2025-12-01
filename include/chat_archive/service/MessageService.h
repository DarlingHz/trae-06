#pragma once

#include "chat_archive/model/Message.h"
#include "chat_archive/dao/MessageDAO.h"
#include "chat_archive/dao/ConversationDAO.h"
#include "chat_archive/dao/UserDAO.h"
#include <vector>
#include <optional>
#include <string>

namespace chat_archive {
namespace service {

// 消息搜索参数
struct MessageSearchParams {
    std::optional<std::string> keyword;
    std::optional<int64_t> user_id;
    std::optional<int64_t> conversation_id;
    std::optional<std::string> from;
    std::optional<std::string> to;
    int limit = 100;
    int offset = 0;
};

// 消息搜索结果
struct MessageSearchResult {
    std::vector<model::Message> messages;
    int64_t total_count = 0;
};

// 消息服务类
class MessageService {
public:
    MessageService() = default;
    ~MessageService() = default;
    
    // 创建消息
    std::optional<int64_t> create_message(int64_t conversation_id,
                                             int64_t sender_id,
                                             const std::string& content,
                                             const std::optional<std::string>& sent_at = std::nullopt);
    
    // 根据ID获取消息
    std::optional<model::Message> get_message_by_id(int64_t id);
    
    // 获取会话中的消息
    std::vector<model::Message> get_conversation_messages(int64_t conversation_id,
                                                               int limit = 100,
                                                               int offset = 0,
                                                               const std::string& order = "asc");
    
    // 更新消息
    bool update_message(int64_t id, const std::string& content);
    
    // 删除消息（软删除）
    bool delete_message(int64_t id);
    
    // 搜索消息
    MessageSearchResult search_messages(const MessageSearchParams& params);
    
private:
    // 验证消息内容
    bool validate_message_content(const std::string& content);
    
    // 验证发送者是否是会话参与者
    bool validate_sender_in_conversation(int64_t conversation_id, int64_t sender_id);
    
    // 解析时间字符串为time_point
    std::optional<std::chrono::system_clock::time_point> parse_time_string(const std::string& time_str);
    
    dao::MessageDAO message_dao_;
    dao::ConversationDAO conversation_dao_;
    dao::UserDAO user_dao_;
};

} // namespace service
} // namespace chat_archive