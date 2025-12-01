#pragma once

#include <string>
#include <optional>
#include <chrono>
#include <vector>
#include "User.h"

namespace chat_archive {
namespace model {

class Conversation {
public:
    Conversation() = default;
    
    Conversation(int64_t id, const std::optional<std::string>& title,
                 const std::chrono::system_clock::time_point& created_at)
        : id_(id), title_(title), created_at_(created_at) {}
    
    // Getters
    int64_t get_id() const { return id_; }
    const std::optional<std::string>& get_title() const { return title_; }
    const std::chrono::system_clock::time_point& get_created_at() const { return created_at_; }
    const std::vector<User>& get_participants() const { return participants_; }
    const std::vector<int64_t>& get_participant_ids() const { return participant_ids_; }
    
    // Setters
    void set_id(int64_t id) { id_ = id; }
    void set_title(const std::optional<std::string>& title) { title_ = title; }
    void set_created_at(const std::chrono::system_clock::time_point& created_at) { created_at_ = created_at; }
    void set_participants(const std::vector<User>& participants) { participants_ = participants; }
    void set_participant_ids(const std::vector<int64_t>& participant_ids) { participant_ids_ = participant_ids; }
    
    // 添加参与者
    void add_participant(const User& user) {
        participants_.push_back(user);
        participant_ids_.push_back(user.get_id());
    }
    
    void add_participant_id(int64_t user_id) {
        participant_ids_.push_back(user_id);
    }
    
private:
    int64_t id_ = 0;
    std::optional<std::string> title_;
    std::chrono::system_clock::time_point created_at_;
    std::vector<User> participants_;
    std::vector<int64_t> participant_ids_;
};

} // namespace model
} // namespace chat_archive