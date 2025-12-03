#include <model/CheckInLog.h>
#include <stdexcept>

namespace event_signup_service::model {

std::string CheckInLog::channel_to_string() const {
    switch (channel_) {
        case CheckInChannel::QR_CODE: return "QR_CODE";
        case CheckInChannel::MANUAL: return "MANUAL";
        case CheckInChannel::UNKNOWN: return "UNKNOWN";
        default: throw std::invalid_argument("Invalid CheckInChannel");
    }
}

CheckInChannel CheckInLog::string_to_channel(const std::string& channel) {
    if (channel == "QR_CODE") return CheckInChannel::QR_CODE;
    if (channel == "MANUAL") return CheckInChannel::MANUAL;
    if (channel == "UNKNOWN") return CheckInChannel::UNKNOWN;
    throw std::invalid_argument("Invalid channel string: " + channel);
}

void to_json(nlohmann::json& j, const CheckInLog& log) {
    j = nlohmann::json{
        {"id", log.id()},
        {"registration_id", log.registration_id()},
        {"check_in_time", std::chrono::duration_cast<std::chrono::seconds>(log.check_in_time().time_since_epoch()).count()},
        {"channel", log.channel_to_string()}
    };
}

void from_json(const nlohmann::json& j, CheckInLog& log) {
    j.at("registration_id").get_to(log.registration_id_);
    if (j.contains("channel")) {
        std::string channel_str = j.at("channel").get<std::string>();
        log.channel_ = CheckInLog::string_to_channel(channel_str);
    }
}

} // namespace event_signup_service::model
