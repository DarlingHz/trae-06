#include "User.h"

User::User(int id, const std::string& username, const std::string& nickname, const std::string& email,
           const std::string& password_hash, const std::string& role, const std::string& status,
           const std::string& created_at, const std::string& updated_at) {
    id_ = id;
    username_ = username;
    nickname_ = nickname;
    email_ = email;
    password_hash_ = password_hash;
    role_ = role;
    status_ = status;
    created_at_ = created_at;
    updated_at_ = updated_at;
}

int User::getId() const {
    return id_;
}

void User::setId(int id) {
    id_ = id;
}

std::string User::getUsername() const {
    return username_;
}

void User::setUsername(const std::string& username) {
    username_ = username;
}

std::string User::getNickname() const {
    return nickname_;
}

void User::setNickname(const std::string& nickname) {
    nickname_ = nickname;
}

std::string User::getEmail() const {
    return email_;
}

void User::setEmail(const std::string& email) {
    email_ = email;
}

std::string User::getPasswordHash() const {
    return password_hash_;
}

void User::setPasswordHash(const std::string& password_hash) {
    password_hash_ = password_hash;
}

std::string User::getRole() const {
    return role_;
}

void User::setRole(const std::string& role) {
    role_ = role;
}

std::string User::getStatus() const {
    return status_;
}

void User::setStatus(const std::string& status) {
    status_ = status;
}

std::string User::getCreatedAt() const {
    return created_at_;
}

void User::setCreatedAt(const std::string& created_at) {
    created_at_ = created_at;
}

std::string User::getUpdatedAt() const {
    return updated_at_;
}

void User::setUpdatedAt(const std::string& updated_at) {
    updated_at_ = updated_at;
}

nlohmann::json User::toJson() const {
    nlohmann::json json_obj;
    json_obj["id"] = id_;
    json_obj["username"] = username_;
    json_obj["nickname"] = nickname_;
    json_obj["email"] = email_;
    json_obj["password_hash"] = password_hash_;
    json_obj["role"] = role_;
    json_obj["status"] = status_;
    json_obj["created_at"] = created_at_;
    json_obj["updated_at"] = updated_at_;
    return json_obj;
}

User User::fromJson(const nlohmann::json& json_obj) {
    User user;
    if (json_obj.contains("id")) {
        user.setId(json_obj["id"]);
    }
    if (json_obj.contains("username")) {
        user.setUsername(json_obj["username"]);
    }
    if (json_obj.contains("nickname")) {
        user.setNickname(json_obj["nickname"]);
    }
    if (json_obj.contains("email")) {
        user.setEmail(json_obj["email"]);
    }
    if (json_obj.contains("password_hash")) {
        user.setPasswordHash(json_obj["password_hash"]);
    }
    if (json_obj.contains("role")) {
        user.setRole(json_obj["role"]);
    }
    if (json_obj.contains("status")) {
        user.setStatus(json_obj["status"]);
    }
    if (json_obj.contains("created_at")) {
        user.setCreatedAt(json_obj["created_at"]);
    }
    if (json_obj.contains("updated_at")) {
        user.setUpdatedAt(json_obj["updated_at"]);
    }
    return user;
}