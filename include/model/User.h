#pragma once

#include <string>
#include <chrono>

namespace model {

class User {
public:
    User() = default;
    User(int id, const std::string& username, const std::string& password_hash, 
         const std::chrono::system_clock::time_point& created_at)
        : id_(id), username_(username), password_hash_(password_hash), created_at_(created_at) {}

    // Getters
    int id() const { return id_; }
    const std::string& username() const { return username_; }
    const std::string& password_hash() const { return password_hash_; }
    const std::chrono::system_clock::time_point& created_at() const { return created_at_; }

    // Setters
    void set_id(int id) { id_ = id; }
    void set_username(const std::string& username) { username_ = username; }
    void set_password_hash(const std::string& password_hash) { password_hash_ = password_hash; }
    void set_created_at(const std::chrono::system_clock::time_point& created_at) { created_at_ = created_at; }

private:
    int id_ = 0;
    std::string username_;
    std::string password_hash_;
    std::chrono::system_clock::time_point created_at_;
};

} // namespace model