#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace pet_hospital {

class Token {
public:
    Token() = default;
    Token(int id, int user_id, const std::string& token, const std::string& expires_at, 
          const std::string& created_at)
        : id(id), user_id(user_id), token(token), expires_at(expires_at), created_at(created_at) {}

    // Getters
    int get_id() const { return id; }
    int get_user_id() const { return user_id; }
    const std::string& get_token() const { return token; }
    const std::string& get_expires_at() const { return expires_at; }
    const std::string& get_created_at() const { return created_at; }

    // Setters
    void set_id(int id) { this->id = id; }
    void set_user_id(int user_id) { this->user_id = user_id; }
    void set_token(const std::string& token) { this->token = token; }
    void set_expires_at(const std::string& expires_at) { this->expires_at = expires_at; }
    void set_created_at(const std::string& created_at) { this->created_at = created_at; }

    // JSON 序列化和反序列化
    friend void to_json(json& j, const Token& token) {
        j = json{
            {"id", token.id},
            {"user_id", token.user_id},
            {"token", token.token},
            {"expires_at", token.expires_at},
            {"created_at", token.created_at}
        };
    }

    friend void from_json(const json& j, Token& token) {
        j.at("user_id").get_to(token.user_id);
        j.at("token").get_to(token.token);
        j.at("expires_at").get_to(token.expires_at);
    }

private:
    int id = 0;
    int user_id = 0;
    std::string token;
    std::string expires_at;
    std::string created_at;
};

} // namespace pet_hospital
