#include "models/announcement.h"

namespace models {

Announcement::Announcement(int id, const std::string& title, const std::string& content,
                           const std::string& category, bool mandatory, int publisher_id,
                           std::time_t publish_time, const std::optional<std::time_t>& expire_time,
                           std::time_t created_at, std::time_t updated_at, Status status)
    : id_(id), title_(title), content_(content), category_(category), mandatory_(mandatory),
      publisher_id_(publisher_id), publish_time_(publish_time), expire_time_(expire_time),
      created_at_(created_at), updated_at_(updated_at), status_(status) {
}

Announcement::Status Announcement::status_from_string(const std::string& status_str) {
    if (status_str == "normal") {
        return Status::NORMAL;
    } else if (status_str == "withdrawn") {
        return Status::WITHDRAWN;
    } else if (status_str == "deleted") {
        return Status::DELETED;
    }
    throw std::invalid_argument("Invalid status string: " + status_str);
}

std::string Announcement::status_to_string(Status status) {
    switch (status) {
        case Status::NORMAL: return "normal";
        case Status::WITHDRAWN: return "withdrawn";
        case Status::DELETED: return "deleted";
        default: return "unknown";
    }
}

} // namespace models
