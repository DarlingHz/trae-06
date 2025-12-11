#pragma once

#include <string>
#include <ctime>

namespace models {

class User {
private:
    int id_;
    std::string email_;
    std::string password_hash_;
    std::string nickname_;
    std::time_t created_at_;
    std::time_t updated_at_;

public:
    User();
    User(int id, const std::string& email, const std::string& password_hash, 
         const std::string& nickname, std::time_t created_at, std::time_t updated_at);

    // Getters
    int id() const { return id_; }
    const std::string& email() const { return email_; }
    const std::string& password_hash() const { return password_hash_; }
    const std::string& nickname() const { return nickname_; }
    std::time_t created_at() const { return created_at_; }
    std::time_t updated_at() const { return updated_at_; }

    // Setters
    void set_email(const std::string& email) { email_ = email; }
    void set_password_hash(const std::string& password_hash) { password_hash_ = password_hash; }
    void set_nickname(const std::string& nickname) { nickname_ = nickname; }
    void set_updated_at(std::time_t updated_at) { updated_at_ = updated_at; }
};

} // namespace models
