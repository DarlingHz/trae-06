#include <model/User.h>

namespace event_signup_service::model {

void to_json(nlohmann::json& j, const User& user) {
    j = nlohmann::json{
        {"id", user.id()},
        {"name", user.name()},
        {"email", user.email()},
        {"phone", user.phone()},
        {"created_at", std::chrono::duration_cast<std::chrono::seconds>(user.created_at().time_since_epoch()).count()}
    };
}

void from_json(const nlohmann::json& j, User& user) {
    j.at("name").get_to(user.name_);
    j.at("email").get_to(user.email_);
    if (j.contains("phone")) {
        j.at("phone").get_to(user.phone_);
    }
}

} // namespace event_signup_service::model
