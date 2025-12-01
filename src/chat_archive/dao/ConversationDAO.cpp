#include "chat_archive/dao/ConversationDAO.h"
#include "chat_archive/Logger.h"
#include "chat_archive/dao/UserDAO.h"
#include <sstream>
#include <iomanip>

namespace chat_archive {
namespace dao {

std::optional<int64_t> ConversationDAO::create_conversation(const std::optional<std::string>& title,
                                                                 const std::vector<int64_t>& participant_ids) {
    auto conn = get_connection();
    if (!conn) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to get database connection for creating conversation");
        return std::nullopt;
    }
    
    // 开始事务
    DatabaseTransaction tx(conn);
    
    const std::string sql = "INSERT INTO conversations (title) VALUES (?);";
    
    DatabaseQuery query(conn);
    if (!query.prepare(sql)) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to prepare create conversation query");
        return std::nullopt;
    }
    
    if (title) {
        if (!query.bind_string(1, *title)) {
            CHAT_ARCHIVE_LOG_ERROR("Failed to bind title parameter for creating conversation");
            return std::nullopt;
        }
    } else {
        if (!query.bind_null(1)) {
            CHAT_ARCHIVE_LOG_ERROR("Failed to bind null title parameter for creating conversation");
            return std::nullopt;
        }
    }
    
    if (!query.execute()) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to execute create conversation query");
        return std::nullopt;
    }
    
    int64_t conversation_id = query.get_last_insert_rowid();
    
    // 添加会话参与者
    if (!add_conversation_participants(conversation_id, participant_ids)) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to add conversation participants");
        return std::nullopt;
    }
    
    // 提交事务
    if (!tx.commit()) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to commit transaction for creating conversation");
        return std::nullopt;
    }
    
    release_connection(conn);
    
    CHAT_ARCHIVE_LOG_INFO("Created conversation with ID: {}, title: {}", conversation_id, title ? *title : "(no title)");
    return conversation_id;
}

std::optional<model::Conversation> ConversationDAO::get_conversation_by_id(int64_t id) {
    const std::string sql = "SELECT id, title, created_at FROM conversations WHERE id = ?;";
    
    auto result = execute_query(sql, {}, {}, {{1, id}});
    
    if (!result.next()) {
        CHAT_ARCHIVE_LOG_DEBUG("Conversation not found with ID: {}", id);
        return std::nullopt;
    }
    
    model::Conversation conversation = build_conversation_from_result(result);
    
    // 获取会话参与者
    std::vector<model::User> participants = get_conversation_participants(id);
    conversation.set_participants(participants);
    
    return conversation;
}

std::vector<model::Conversation> ConversationDAO::get_conversations(int limit, int offset) {
    const std::string sql = "SELECT id, title, created_at FROM conversations ORDER BY created_at DESC LIMIT ? OFFSET ?;";
    
    auto result = execute_query(sql, {}, {{1, limit}, {2, offset}}, {});
    
    std::vector<model::Conversation> conversations;
    while (result.next()) {
        model::Conversation conversation = build_conversation_from_result(result);
        
        // 获取会话参与者（只获取ID，不获取详细信息）
        std::vector<model::User> participants = get_conversation_participants(conversation.get_id());
        conversation.set_participants(participants);
        
        conversations.push_back(conversation);
    }
    
    CHAT_ARCHIVE_LOG_DEBUG("Retrieved {} conversations from database", conversations.size());
    return conversations;
}

int64_t ConversationDAO::get_total_conversations() {
    const std::string sql = "SELECT COUNT(*) FROM conversations;";
    
    auto result = execute_query(sql);
    
    if (!result.next()) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to get total conversations count");
        return 0;
    }
    
    return result.get_int64(0);
}

std::vector<model::User> ConversationDAO::get_conversation_participants(int64_t conversation_id) {
    const std::string sql = R"(
        SELECT u.id, u.name, u.created_at 
        FROM users u 
        INNER JOIN conversation_participants cp ON u.id = cp.user_id 
        WHERE cp.conversation_id = ? 
        ORDER BY u.name;
    )";
    
    auto result = execute_query(sql, {}, {}, {{1, conversation_id}});
    
    std::vector<model::User> participants;
    while (result.next()) {
        int64_t id = result.get_int64(0);
        std::string name = result.get_string(1);
        std::string created_at_str = result.get_string(2);
        
        // 解析时间字符串
        std::tm tm = {};
        std::istringstream iss(created_at_str);
        iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        
        auto created_at = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        
        participants.emplace_back(id, name, created_at);
    }
    
    CHAT_ARCHIVE_LOG_DEBUG("Retrieved {} participants for conversation ID: {}", participants.size(), conversation_id);
    return participants;
}

model::Conversation ConversationDAO::build_conversation_from_result(const DatabaseResult& result) {
    int64_t id = result.get_int64(0);
    std::string title_str = result.get_string(1);
    std::optional<std::string> title = title_str.empty() ? std::nullopt : std::optional<std::string>(title_str);
    std::string created_at_str = result.get_string(2);
    
    // 解析时间字符串
    std::tm tm = {};
    std::istringstream iss(created_at_str);
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    auto created_at = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    
    return model::Conversation(id, title, created_at);
}

bool ConversationDAO::add_conversation_participants(int64_t conversation_id, const std::vector<int64_t>& participant_ids) {
    if (participant_ids.empty()) {
        CHAT_ARCHIVE_LOG_WARN("No participants provided for conversation ID: {}", conversation_id);
        return true;
    }
    
    auto conn = get_connection();
    if (!conn) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to get database connection for adding conversation participants");
        return false;
    }
    
    const std::string sql = "INSERT INTO conversation_participants (conversation_id, user_id) VALUES (?, ?);";
    
    DatabaseQuery query(conn);
    if (!query.prepare(sql)) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to prepare add conversation participants query");
        release_connection(conn);
        return false;
    }
    
    // 绑定conversation_id参数（参数1）
    if (!query.bind_int64(1, conversation_id)) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to bind conversation_id parameter for adding participants");
        release_connection(conn);
        return false;
    }
    
    // 批量插入参与者
    int count = 0;
    for (int64_t user_id : participant_ids) {
        // 重置参数（除了第一个参数）
        if (!query.bind_int64(2, user_id)) {
            CHAT_ARCHIVE_LOG_ERROR("Failed to bind user_id parameter for adding participants");
            release_connection(conn);
            return false;
        }
        
        if (!query.execute()) {
            CHAT_ARCHIVE_LOG_ERROR("Failed to execute add conversation participant query for user ID: {}", user_id);
            release_connection(conn);
            return false;
        }
        
        count++;
        
        // 重置查询状态，以便再次执行
        query.prepare(sql);
        query.bind_int64(1, conversation_id);
    }
    
    release_connection(conn);
    
    CHAT_ARCHIVE_LOG_DEBUG("Added {} participants to conversation ID: {}", count, conversation_id);
    return true;
}

} // namespace dao
} // namespace chat_archive