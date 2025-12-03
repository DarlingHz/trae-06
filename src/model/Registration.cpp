#include <model/Registration.h>
#include <stdexcept>

namespace event_signup_service::model {

std::string Registration::status_to_string() const {
    switch (status_) {
        case RegistrationStatus::REGISTERED: return "REGISTERED";
        case RegistrationStatus::WAITING: return "WAITING";
        case RegistrationStatus::CANCELED: return "CANCELED";
        case RegistrationStatus::CHECKED_IN: return "CHECKED_IN";
        default: throw std::invalid_argument("Invalid RegistrationStatus");
    }
}

RegistrationStatus Registration::string_to_status(const std::string& status) {
    if (status == "REGISTERED") return RegistrationStatus::REGISTERED;
    if (status == "WAITING") return RegistrationStatus::WAITING;
    if (status == "CANCELED") return RegistrationStatus::CANCELED;
    if (status == "CHECKED_IN") return RegistrationStatus::CHECKED_IN;
    throw std::invalid_argument("Invalid status string: " + status);
}

void to_json(nlohmann::json& j, const Registration& registration) {
    j = nlohmann::json{
        {"id", registration.id()},
        {"user_id", registration.user_id()},
        {"event_id", registration.event_id()},
        {"status", registration.status_to_string()},
        {"created_at", std::chrono::duration_cast<std::chrono::seconds>(registration.created_at().time_since_epoch()).count()},
        {"updated_at", std::chrono::duration_cast<std::chrono::seconds>(registration.updated_at().time_since_epoch()).count()}
    };
}

void from_json(const nlohmann::json& j, Registration& registration) {
    if (j.contains("user_id")) {
        j.at("user_id").get_to(registration.user_id_);
    }
    if (j.contains("event_id")) {
        j.at("event_id").get_to(registration.event_id_);
    }
    if (j.contains("status")) {
        std::string status_str = j.at("status").get<std::string>();
        registration.status_ = Registration::string_to_status(status_str);
    }
}

} // namespace event_signup_service::model
