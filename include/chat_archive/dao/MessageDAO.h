#pragma once

#include "BaseDAO.h"
#include "chat_archive/model/Message.h"
#include "chat_archive/model/User.h"
#include <vector>
#include <optional>
#include <chrono>

namespace chat_archive {
namespace dao {

// 消息搜索参数
struct MessageSearchParams {
    std::optional<std::string> keyword;
    std::optional<int64_t> user_id;
    std::optional<int64_t> conversation_id;
    std::optional<std::chrono::system_clock::time_point> from;
    std::optional<std::chrono::system_clock::time_point> to;
    int limit = 100;
    int offset = 0;
};

// 消息搜索结果
struct MessageSearchResult {
    std::vector<model::Message> messages;
    int64_t total_count = 0;
};

class MessageDAO : public BaseDAO {
public:
    MessageDAO() = default;
    ~MessageDAO() override = default;
    
    // 创建消息
    std::optional<int64_t> create_message(int64_t conversation_id, int64_t sender_id,
                                             const std::string& content,
                                             const std::optional<std::chrono::system_clock::time_point>& sent_at = std::nullopt);
    
    // 根据ID获取消息
    std::optional<model::Message> get_message_by_id(int64_t id);
    
    // 获取会话中的消息列表
    std::vector<model::Message> get_conversation_messages(int64_t conversation_id,
                                                               int limit = 100, int offset = 0,
                                                               bool order_asc = true, bool include_deleted = false);
    
    // 更新消息
    bool update_message(int64_t id, const std::string& content);
    
    // 软删除消息
    bool delete_message(int64_t id);
    
    // 搜索消息
    MessageSearchResult search_messages(const MessageSearchParams& params);
    
    // 获取消息总数
    int64_t get_total_messages();
    
    // 获取最近24小时的消息数量
    int64_t get_messages_last_24h();
    
    // 获取消息数量最多的前N个用户
    std::vector<std::pair<model::User, int64_t>> get_top_senders(int limit = 10);
    
private:
    // 从数据库结果中构建消息对象
    model::Message build_message_from_result(const DatabaseResult& result);
    
    // 构建消息搜索的SQL查询
    std::string build_search_sql(const MessageSearchParams& params, std::vector<std::pair<int, std::string>>& string_params,
                                  std::vector<std::pair<int, int>>& int_params, std::vector<std::pair<int, int64_t>>& int64_params,
                                  std::vector<std::pair<int, double>>& double_params);
    
    // 将时间点转换为SQLite支持的时间字符串
    std::string time_point_to_sql_string(const std::chrono::system_clock::time_point& time_point);
};

} // namespace dao
} // namespace chat_archive