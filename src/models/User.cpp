#include "models/User.h"
#include <nlohmann/json.hpp>

namespace models {

User::User() 
    : id_(0), created_at_(0), updated_at_(0) {}

User::User(int id, const std::string& email, const std::string& password_hash, 
           const std::string& nickname, std::time_t created_at, std::time_t updated_at)
    : id_(id), email_(email), password_hash_(password_hash), nickname_(nickname),
      created_at_(created_at), updated_at_(updated_at) {}

nlohmann::json to_json(const User& user) {
    nlohmann::json j;
    j["id"] = user.id();
    j["email"] = user.email();
    j["nickname"] = user.nickname();
    j["created_at"] = user.created_at();
    j["updated_at"] = user.updated_at();
    return j;
}

} // namespace models
