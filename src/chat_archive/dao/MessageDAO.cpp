#include "chat_archive/dao/MessageDAO.h"
#include "chat_archive/Logger.h"
#include "chat_archive/dao/UserDAO.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace chat_archive {
namespace dao {

std::optional<int64_t> MessageDAO::create_message(int64_t conversation_id, int64_t sender_id,
                                                       const std::string& content,
                                                       const std::optional<std::chrono::system_clock::time_point>& sent_at) {
    const std::string sql = sent_at ? 
        "INSERT INTO messages (conversation_id, sender_id, content, sent_at) VALUES (?, ?, ?, ?);" :
        "INSERT INTO messages (conversation_id, sender_id, content) VALUES (?, ?, ?);";
    
    auto conn = get_connection();
    if (!conn) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to get database connection for creating message");
        return std::nullopt;
    }
    
    DatabaseQuery query(conn);
    if (!query.prepare(sql)) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to prepare create message query");
        release_connection(conn);
        return std::nullopt;
    }
    
    // 绑定参数
    int param_index = 1;
    if (!query.bind_int64(param_index++, conversation_id)) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to bind conversation_id parameter for creating message");
        release_connection(conn);
        return std::nullopt;
    }
    
    if (!query.bind_int64(param_index++, sender_id)) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to bind sender_id parameter for creating message");
        release_connection(conn);
        return std::nullopt;
    }
    
    if (!query.bind_string(param_index++, content)) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to bind content parameter for creating message");
        release_connection(conn);
        return std::nullopt;
    }
    
    if (sent_at) {
        std::string sent_at_str = time_point_to_sql_string(*sent_at);
        if (!query.bind_string(param_index++, sent_at_str)) {
            CHAT_ARCHIVE_LOG_ERROR("Failed to bind sent_at parameter for creating message");
            release_connection(conn);
            return std::nullopt;
        }
    }
    
    if (!query.execute()) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to execute create message query");
        release_connection(conn);
        return std::nullopt;
    }
    
    int64_t message_id = query.get_last_insert_rowid();
    release_connection(conn);
    
    CHAT_ARCHIVE_LOG_INFO("Created message with ID: {} in conversation ID: {}", message_id, conversation_id);
    return message_id;
}

std::optional<model::Message> MessageDAO::get_message_by_id(int64_t id) {
    const std::string sql = "SELECT id, conversation_id, sender_id, content, sent_at, edited_at, deleted FROM messages WHERE id = ?;";
    
    auto result = execute_query(sql, {}, {}, {{1, id}});
    
    if (!result.next()) {
        CHAT_ARCHIVE_LOG_DEBUG("Message not found with ID: {}", id);
        return std::nullopt;
    }
    
    model::Message message = build_message_from_result(result);
    
    // 获取发送者信息
    UserDAO user_dao;
    auto sender = user_dao.get_user_by_id(message.get_sender_id());
    if (sender) {
        message.set_sender(*sender);
    }
    
    return message;
}

std::vector<model::Message> MessageDAO::get_conversation_messages(int64_t conversation_id,
                                                                         int limit, int offset,
                                                                         bool order_asc, bool include_deleted) {
    std::ostringstream sql; 
    sql << "SELECT id, conversation_id, sender_id, content, sent_at, edited_at, deleted " 
        << "FROM messages " 
        << "WHERE conversation_id = ?";
    
    if (!include_deleted) {
        sql << " AND deleted = 0";
    }
    
    sql << " ORDER BY sent_at " << (order_asc ? "ASC" : "DESC") 
        << " LIMIT ? OFFSET ?;";
    
    auto result = execute_query(sql.str(), {}, {{2, limit}, {3, offset}}, {{1, conversation_id}});
    
    std::vector<model::Message> messages;
    while (result.next()) {
        messages.push_back(build_message_from_result(result));
    }
    
    // 获取所有发送者信息
    UserDAO user_dao;
    std::unordered_map<int64_t, model::User> user_cache;
    
    for (auto& message : messages) {
        int64_t sender_id = message.get_sender_id();
        
        if (user_cache.find(sender_id) == user_cache.end()) {
            auto user = user_dao.get_user_by_id(sender_id);
            if (user) {
                user_cache[sender_id] = *user;
            }
        }
        
        if (user_cache.find(sender_id) != user_cache.end()) {
            message.set_sender(user_cache[sender_id]);
        }
    }
    
    CHAT_ARCHIVE_LOG_DEBUG("Retrieved {} messages from conversation ID: {}", messages.size(), conversation_id);
    return messages;
}

bool MessageDAO::update_message(int64_t id, const std::string& content) {
    const std::string sql = "UPDATE messages SET content = ?, edited_at = CURRENT_TIMESTAMP WHERE id = ?;";
    
    int64_t affected_rows = execute_update(sql, {{1, content}}, {}, {{2, id}});
    
    if (affected_rows == -1) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to update message with ID: {}", id);
        return false;
    } else if (affected_rows == 0) {
        CHAT_ARCHIVE_LOG_DEBUG("No message found with ID: {} to update", id);
        return false;
    }
    
    CHAT_ARCHIVE_LOG_INFO("Updated message with ID: {}", id);
    return true;
}

bool MessageDAO::delete_message(int64_t id) {
    const std::string sql = "UPDATE messages SET deleted = 1 WHERE id = ?;";
    
    int64_t affected_rows = execute_update(sql, {}, {}, {{1, id}});
    
    if (affected_rows == -1) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to delete message with ID: {}", id);
        return false;
    } else if (affected_rows == 0) {
        CHAT_ARCHIVE_LOG_DEBUG("No message found with ID: {} to delete", id);
        return false;
    }
    
    CHAT_ARCHIVE_LOG_INFO("Deleted message with ID: {}", id);
    return true;
}

MessageSearchResult MessageDAO::search_messages(const MessageSearchParams& params) {
    MessageSearchResult result;
    
    std::vector<std::pair<int, std::string>> string_params;
    std::vector<std::pair<int, int>> int_params;
    std::vector<std::pair<int, int64_t>> int64_params;
    std::vector<std::pair<int, double>> double_params;
    
    // 构建搜索SQL
    std::string sql = build_search_sql(params, string_params, int_params, int64_params, double_params);
    
    // 执行查询获取消息列表
    auto message_result = execute_query(sql, string_params, int_params, int64_params);
    
    std::vector<model::Message> messages;
    while (message_result.next()) {
        messages.push_back(build_message_from_result(message_result));
    }
    
    // 获取消息总数
    std::string count_sql = sql;
    size_t from_pos = count_sql.find("FROM");
    if (from_pos != std::string::npos) {
        count_sql = "SELECT COUNT(*) " + count_sql.substr(from_pos);
        
        // 移除LIMIT和OFFSET
        size_t limit_pos = count_sql.find("LIMIT");
        if (limit_pos != std::string::npos) {
            count_sql = count_sql.substr(0, limit_pos);
        }
    }
    
    auto count_result = execute_query(count_sql, string_params, int_params, int64_params);
    int64_t total_count = 0;
    if (count_result.next()) {
        total_count = count_result.get_int64(0);
    }
    
    // 获取所有发送者信息
    UserDAO user_dao;
    std::unordered_map<int64_t, model::User> user_cache;
    
    for (auto& message : messages) {
        int64_t sender_id = message.get_sender_id();
        
        if (user_cache.find(sender_id) == user_cache.end()) {
            auto user = user_dao.get_user_by_id(sender_id);
            if (user) {
                user_cache[sender_id] = *user;
            }
        }
        
        if (user_cache.find(sender_id) != user_cache.end()) {
            message.set_sender(user_cache[sender_id]);
        }
    }
    
    result.messages = messages;
    result.total_count = total_count;
    
    CHAT_ARCHIVE_LOG_DEBUG("Search returned {} messages out of total {} matching messages", messages.size(), total_count);
    return result;
}

int64_t MessageDAO::get_total_messages() {
    const std::string sql = "SELECT COUNT(*) FROM messages;";
    
    auto result = execute_query(sql);
    
    if (!result.next()) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to get total messages count");
        return 0;
    }
    
    return result.get_int64(0);
}

int64_t MessageDAO::get_messages_last_24h() {
    const std::string sql = "SELECT COUNT(*) FROM messages WHERE sent_at >= datetime('now', '-24 hours');";
    
    auto result = execute_query(sql);
    
    if (!result.next()) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to get messages last 24h count");
        return 0;
    }
    
    return result.get_int64(0);
}

std::vector<std::pair<model::User, int64_t>> MessageDAO::get_top_senders(int limit) {
    const std::string sql = R"(
        SELECT u.id, u.name, u.created_at, COUNT(m.id) as message_count 
        FROM users u 
        INNER JOIN messages m ON u.id = m.sender_id 
        WHERE m.deleted = 0 
        GROUP BY u.id 
        ORDER BY message_count DESC 
        LIMIT ?;
    )";
    
    auto result = execute_query(sql, {}, {{1, limit}}, {});
    
    std::vector<std::pair<model::User, int64_t>> top_senders;
    while (result.next()) {
        int64_t id = result.get_int64(0);
        std::string name = result.get_string(1);
        std::string created_at_str = result.get_string(2);
        int64_t message_count = result.get_int64(3);
        
        // 解析时间字符串
        std::tm tm = {};
        std::istringstream iss(created_at_str);
        iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        
        auto created_at = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        
        model::User user(id, name, created_at);
        top_senders.emplace_back(user, message_count);
    }
    
    CHAT_ARCHIVE_LOG_DEBUG("Retrieved top {} senders", top_senders.size());
    return top_senders;
}

model::Message MessageDAO::build_message_from_result(const DatabaseResult& result) {
    int64_t id = result.get_int64(0);
    int64_t conversation_id = result.get_int64(1);
    int64_t sender_id = result.get_int64(2);
    std::string content = result.get_string(3);
    std::string sent_at_str = result.get_string(4);
    std::string edited_at_str = result.get_string(5);
    bool deleted = result.get_int(6) == 1;
    
    // 解析时间字符串
    std::tm tm = {};
    std::istringstream iss(sent_at_str);
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto sent_at = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    
    std::optional<std::chrono::system_clock::time_point> edited_at;
    if (!edited_at_str.empty()) {
        std::tm edited_tm = {};
        std::istringstream edited_iss(edited_at_str);
        edited_iss >> std::get_time(&edited_tm, "%Y-%m-%d %H:%M:%S");
        edited_at = std::chrono::system_clock::from_time_t(std::mktime(&edited_tm));
    }
    
    return model::Message(id, conversation_id, sender_id, content, sent_at, edited_at, deleted);
}

std::string MessageDAO::build_search_sql(const MessageSearchParams& params, 
                                            std::vector<std::pair<int, std::string>>& string_params,
                                            std::vector<std::pair<int, int>>& int_params,
                                            std::vector<std::pair<int, int64_t>>& int64_params,
                                            std::vector<std::pair<int, double>>& double_params) {
    std::ostringstream sql;
    int param_index = 1;
    
    sql << "SELECT id, conversation_id, sender_id, content, sent_at, edited_at, deleted " 
        << "FROM messages " 
        << "WHERE deleted = 0";
    
    // 关键字搜索
    if (params.keyword && !params.keyword->empty()) {
        sql << " AND content LIKE ?";
        string_params.emplace_back(param_index++, "%" + *params.keyword + "%");
    }
    
    // 用户ID过滤
    if (params.user_id) {
        sql << " AND sender_id = ?";
        int64_params.emplace_back(param_index++, *params.user_id);
    }
    
    // 会话ID过滤
    if (params.conversation_id) {
        sql << " AND conversation_id = ?";
        int64_params.emplace_back(param_index++, *params.conversation_id);
    }
    
    // 时间范围过滤
    if (params.from) {
        sql << " AND sent_at >= ?";
        string_params.emplace_back(param_index++, time_point_to_sql_string(*params.from));
    }
    
    if (params.to) {
        sql << " AND sent_at <= ?";
        string_params.emplace_back(param_index++, time_point_to_sql_string(*params.to));
    }
    
    // 排序
    sql << " ORDER BY sent_at DESC";
    
    // 分页
    sql << " LIMIT ? OFFSET ?";
    int_params.emplace_back(param_index++, params.limit);
    int_params.emplace_back(param_index++, params.offset);
    
    return sql.str();
}

std::string MessageDAO::time_point_to_sql_string(const std::chrono::system_clock::time_point& time_point) {
    auto time_t = std::chrono::system_clock::to_time_t(time_point);
    std::tm tm = {};
    
    // 使用线程安全的gmtime_r函数
    gmtime_r(&time_t, &tm);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    return oss.str();
}

} // namespace dao
} // namespace chat_archive