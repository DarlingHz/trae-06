#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace pet_hospital {

class User {
public:
    User() = default;
    User(int id, const std::string& email, const std::string& password_hash, 
         const std::string& name, const std::optional<std::string>& phone, 
         const std::string& created_at, const std::string& updated_at)
        : id(id), email(email), password_hash(password_hash), name(name), 
          phone(phone), created_at(created_at), updated_at(updated_at) {}

    // Getters
    int get_id() const { return id; }
    const std::string& get_email() const { return email; }
    const std::string& get_password_hash() const { return password_hash; }
    const std::string& get_name() const { return name; }
    const std::optional<std::string>& get_phone() const { return phone; }
    const std::string& get_created_at() const { return created_at; }
    const std::string& get_updated_at() const { return updated_at; }

    // Setters
    void set_id(int id) { this->id = id; }
    void set_email(const std::string& email) { this->email = email; }
    void set_password_hash(const std::string& password_hash) { this->password_hash = password_hash; }
    void set_name(const std::string& name) { this->name = name; }
    void set_phone(const std::optional<std::string>& phone) { this->phone = phone; }
    void set_created_at(const std::string& created_at) { this->created_at = created_at; }
    void set_updated_at(const std::string& updated_at) { this->updated_at = updated_at; }

    // JSON 序列化和反序列化
    friend void to_json(json& j, const User& user) {
        j = json{
            {"id", user.id},
            {"email", user.email},
            {"name", user.name},
            {"phone", user.phone},
            {"created_at", user.created_at},
            {"updated_at", user.updated_at}
        };
    }

    friend void from_json(const json& j, User& user) {
        j.at("email").get_to(user.email);
        j.at("password").get_to(user.password_hash); // 注意：这里需要后续处理哈希
        j.at("name").get_to(user.name);
        if (j.contains("phone")) {
            user.phone = j.at("phone").get<std::string>();
        }
    }

private:
    int id = 0;
    std::string email;
    std::string password_hash;
    std::string name;
    std::optional<std::string> phone;
    std::string created_at;
    std::string updated_at;
};

} // namespace pet_hospital
