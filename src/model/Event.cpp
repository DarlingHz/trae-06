#include <model/Event.h>
#include <stdexcept>

namespace event_signup_service::model {

std::string Event::status_to_string() const {
    switch (status_) {
        case EventStatus::DRAFT: return "DRAFT";
        case EventStatus::PUBLISHED: return "PUBLISHED";
        case EventStatus::CLOSED: return "CLOSED";
        default: throw std::invalid_argument("Invalid EventStatus");
    }
}

EventStatus Event::string_to_status(const std::string& status) {
    if (status == "DRAFT") return EventStatus::DRAFT;
    if (status == "PUBLISHED") return EventStatus::PUBLISHED;
    if (status == "CLOSED") return EventStatus::CLOSED;
    throw std::invalid_argument("Invalid status string: " + status);
}

bool Event::can_modify() const {
    auto now = std::chrono::system_clock::now();
    // 活动已开始或已结束时，部分字段不可修改
    return now < start_time_;
}

void to_json(nlohmann::json& j, const Event& event) {
    j = nlohmann::json{
        {"id", event.id()},
        {"title", event.title()},
        {"description", event.description()},
        {"start_time", std::chrono::duration_cast<std::chrono::seconds>(event.start_time().time_since_epoch()).count()},
        {"end_time", std::chrono::duration_cast<std::chrono::seconds>(event.end_time().time_since_epoch()).count()},
        {"location", event.location()},
        {"capacity", event.capacity()},
        {"status", event.status_to_string()},
        {"created_at", std::chrono::duration_cast<std::chrono::seconds>(event.created_at().time_since_epoch()).count()},
        {"updated_at", std::chrono::duration_cast<std::chrono::seconds>(event.updated_at().time_since_epoch()).count()}
    };
}

void from_json(const nlohmann::json& j, Event& event) {
    j.at("title").get_to(event.title_);
    if (j.contains("description")) {
        j.at("description").get_to(event.description_);
    }
    if (j.contains("start_time")) {
        auto start_time_seconds = j.at("start_time").get<int64_t>();
        event.start_time_ = std::chrono::system_clock::time_point(std::chrono::seconds(start_time_seconds));
    }
    if (j.contains("end_time")) {
        auto end_time_seconds = j.at("end_time").get<int64_t>();
        event.end_time_ = std::chrono::system_clock::time_point(std::chrono::seconds(end_time_seconds));
    }
    if (j.contains("location")) {
        j.at("location").get_to(event.location_);
    }
    if (j.contains("capacity")) {
        j.at("capacity").get_to(event.capacity_);
    }
    if (j.contains("status")) {
        std::string status_str = j.at("status").get<std::string>();
        event.status_ = Event::string_to_status(status_str);
    }
}

} // namespace event_signup_service::model
