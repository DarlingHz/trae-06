#include "models/Bookmark.h"
#include "models/Stats.h"
#include <stdexcept>

namespace models {

Bookmark::Bookmark() 
    : id_(0), user_id_(0), is_favorite_(false), read_status_(ReadStatus::UNREAD),
      created_at_(0), updated_at_(0), last_accessed_at_(0), click_count_(0) {}

Bookmark::Bookmark(int id, int user_id, const std::string& url, const std::string& title, 
                   const std::string& description, const std::vector<std::string>& tags, 
                   const std::string& folder, bool is_favorite, ReadStatus read_status, 
                   std::time_t created_at, std::time_t updated_at, std::time_t last_accessed_at, 
                   int click_count)
    : id_(id), user_id_(user_id), url_(url), title_(title), description_(description),
      tags_(tags), folder_(folder), is_favorite_(is_favorite), read_status_(read_status),
      created_at_(created_at), updated_at_(updated_at), last_accessed_at_(last_accessed_at),
      click_count_(click_count) {}

std::string Bookmark::read_status_to_string(ReadStatus status) {
    switch(status) {
        case ReadStatus::UNREAD: return "unread";
        case ReadStatus::READING: return "reading";
        case ReadStatus::READ: return "read";
        default: return "unread";
    }
}

ReadStatus Bookmark::read_status_from_string(const std::string& str) {
    if(str == "read") {
        return ReadStatus::READ;
    } else if(str == "unread") {
        return ReadStatus::UNREAD;
    } else {
        return ReadStatus::UNREAD;
    }
}

models::ReadStatus models::Bookmark::string_to_read_status(const std::string& str) {
    if(str == "reading") return models::ReadStatus::READING;
    if(str == "read") return models::ReadStatus::READ;
    return models::ReadStatus::UNREAD;
}

// Convert Bookmark to JSON
nlohmann::json models::to_json(const models::Bookmark& bookmark) {
    nlohmann::json j;
    j["id"] = bookmark.id();
    j["user_id"] = bookmark.user_id();
    j["url"] = bookmark.url();
    j["title"] = bookmark.title();
    j["description"] = bookmark.description();
    j["tags"] = bookmark.tags();
    j["folder"] = bookmark.folder();
    j["is_favorite"] = bookmark.is_favorite();
    j["read_status"] = models::Bookmark::read_status_to_string(bookmark.read_status());
    j["created_at"] = bookmark.created_at();
    j["updated_at"] = bookmark.updated_at();
    j["last_accessed_at"] = bookmark.last_accessed_at();
    j["click_count"] = bookmark.click_count();
    return j;
}

// Convert User to JSON


// Convert UserStats to JSON
nlohmann::json models::to_json(const models::UserStats& stats) {
    nlohmann::json j;
    j["total_bookmarks"] = stats.total_bookmarks();
    j["unread_count"] = stats.unread_count();
    j["reading_count"] = stats.reading_count();
    j["read_count"] = stats.read_count();
    j["favorite_count"] = stats.favorite_count();
    return j;
}

// Convert DomainStats to JSON
nlohmann::json models::to_json(const models::DomainStats& stats) {
    nlohmann::json j;
    j["domain"] = stats.domain();
    j["click_count"] = stats.click_count();
    return j;
}

// Convert DailyStats to JSON
nlohmann::json models::to_json(const models::DailyStats& stats) {
    nlohmann::json j;
    j["date"] = stats.date();
    j["count"] = stats.count();
    return j;
}

// Convert TagStats to JSON
nlohmann::json models::to_json(const models::TagStats& stats) {
    nlohmann::json j;
    j["tag"] = stats.tag();
    j["count"] = stats.count();
    return j;
}

// Convert FolderStats to JSON
nlohmann::json models::to_json(const models::FolderStats& stats) {
    nlohmann::json j;
    j["folder"] = stats.folder();
    j["count"] = stats.count();
    return j;
}

// Convert vector<shared_ptr<Bookmark>> to JSON
nlohmann::json models::to_json(const std::vector<std::shared_ptr<models::Bookmark>>& bookmarks) {
    nlohmann::json j;
    for (const auto& bookmark : bookmarks) {
        j.push_back(models::to_json(*bookmark));
    }
    return j;
}

// Convert vector<DailyStats> to JSON
nlohmann::json models::to_json(const std::vector<models::DailyStats>& daily_stats) {
    nlohmann::json j;
    for (const auto& stat : daily_stats) {
        j.push_back(models::to_json(stat));
    }
    return j;
}

// Convert vector<DomainStats> to JSON
nlohmann::json models::to_json(const std::vector<models::DomainStats>& domains) {
    nlohmann::json j;
    for (const auto& domain : domains) {
        j.push_back(models::to_json(domain));
    }
    return j;
}

// Convert vector<TagStats> to JSON
nlohmann::json models::to_json(const std::vector<models::TagStats>& tags) {
    nlohmann::json j;
    for (const auto& tag : tags) {
        j.push_back(models::to_json(tag));
    }
    return j;
}

// Convert vector<FolderStats> to JSON
nlohmann::json models::to_json(const std::vector<models::FolderStats>& folders) {
    nlohmann::json j;
    for (const auto& folder : folders) {
        j.push_back(models::to_json(folder));
    }
    return j;
}

} // namespace models
