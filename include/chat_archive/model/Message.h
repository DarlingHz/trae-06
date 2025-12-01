#pragma once

#include <string>
#include <optional>
#include <chrono>
#include "User.h"

namespace chat_archive {
namespace model {

class Message {
public:
    Message() = default;
    
    Message(int64_t id, int64_t conversation_id, int64_t sender_id,
            const std::string& content, const std::chrono::system_clock::time_point& sent_at,
            const std::optional<std::chrono::system_clock::time_point>& edited_at,
            bool deleted)
        : id_(id), conversation_id_(conversation_id), sender_id_(sender_id), content_(content),
          sent_at_(sent_at), edited_at_(edited_at), deleted_(deleted) {}
    
    // Getters
    int64_t get_id() const { return id_; }
    int64_t get_conversation_id() const { return conversation_id_; }
    int64_t get_sender_id() const { return sender_id_; }
    const std::string& get_content() const { return content_; }
    const std::chrono::system_clock::time_point& get_sent_at() const { return sent_at_; }
    const std::optional<std::chrono::system_clock::time_point>& get_edited_at() const { return edited_at_; }
    bool is_deleted() const { return deleted_; }
    const User& get_sender() const { return sender_; }
    
    // Setters
    void set_id(int64_t id) { id_ = id; }
    void set_conversation_id(int64_t conversation_id) { conversation_id_ = conversation_id; }
    void set_sender_id(int64_t sender_id) { sender_id_ = sender_id; }
    void set_content(const std::string& content) { content_ = content; }
    void set_sent_at(const std::chrono::system_clock::time_point& sent_at) { sent_at_ = sent_at; }
    void set_edited_at(const std::optional<std::chrono::system_clock::time_point>& edited_at) { edited_at_ = edited_at; }
    void set_deleted(bool deleted) { deleted_ = deleted; }
    void set_sender(const User& sender) { sender_ = sender; }
    
private:
    int64_t id_ = 0;
    int64_t conversation_id_ = 0;
    int64_t sender_id_ = 0;
    std::string content_;
    std::chrono::system_clock::time_point sent_at_;
    std::optional<std::chrono::system_clock::time_point> edited_at_;
    bool deleted_ = false;
    User sender_; // 可选：缓存发送者信息
};

} // namespace model
} // namespace chat_archive