#pragma once

#include <string>
#include <optional>
#include <chrono>

namespace chat_archive {
namespace model {

class User {
public:
    User() = default;
    
    User(int64_t id, const std::string& name, const std::chrono::system_clock::time_point& created_at)
        : id_(id), name_(name), created_at_(created_at) {}
    
    // Getters
    int64_t get_id() const { return id_; }
    const std::string& get_name() const { return name_; }
    const std::chrono::system_clock::time_point& get_created_at() const { return created_at_; }
    
    // Setters
    void set_id(int64_t id) { id_ = id; }
    void set_name(const std::string& name) { name_ = name; }
    void set_created_at(const std::chrono::system_clock::time_point& created_at) { created_at_ = created_at; }
    
private:
    int64_t id_ = 0;
    std::string name_;
    std::chrono::system_clock::time_point created_at_;
};

} // namespace model
} // namespace chat_archive