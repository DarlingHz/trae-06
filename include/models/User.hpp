#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace models {

class User {
public:
    // Default constructor
    User() = default;

    // Constructor with parameters
    User(int id, const std::string& username, const std::string& email, const std::string& password_hash, const std::string& created_at)
        : id_(id), username_(username), email_(email), password_hash_(password_hash), created_at_(created_at) {}

    // Getters
    int getId() const { return id_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getEmail() const { return email_; }
    const std::string& getPasswordHash() const { return password_hash_; }
    const std::string& getCreatedAt() const { return created_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setUsername(const std::string& username) { username_ = username; }
    void setEmail(const std::string& email) { email_ = email; }
    void setPasswordHash(const std::string& password_hash) { password_hash_ = password_hash; }
    void setCreatedAt(const std::string& created_at) { created_at_ = created_at; }

    // Convert User object to JSON
    json toJson() const {
        json j;
        j["id"] = id_;
        j["username"] = username_;
        j["email"] = email_;
        j["created_at"] = created_at_;
        return j;
    }

    // Convert JSON to User object
    static User fromJson(const json& j) {
        User user;
        user.id_ = j.value("id", 0);
        user.username_ = j.value("username", "");
        user.email_ = j.value("email", "");
        user.password_hash_ = j.value("password_hash", "");
        user.created_at_ = j.value("created_at", "");
        return user;
    }

private:
    int id_ = 0;
    std::string username_;
    std::string email_;
    std::string password_hash_;
    std::string created_at_;
};

} // namespace models
