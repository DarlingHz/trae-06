#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <nlohmann/json.hpp>
#include "models/Stats.h"
#include "models/User.h"

namespace models {

enum class ReadStatus {
    UNREAD,
    READING,
    READ
};

class Bookmark {
private:
    int id_;
    int user_id_;
    std::string url_;
    std::string title_;
    std::string description_;
    std::vector<std::string> tags_;
    std::string folder_;
    bool is_favorite_;
    ReadStatus read_status_;
    std::time_t created_at_;
    std::time_t updated_at_;
    std::time_t last_accessed_at_;
    int click_count_;

public:
    Bookmark();
    Bookmark(int id, int user_id, const std::string& url, const std::string& title, 
             const std::string& description, const std::vector<std::string>& tags, 
             const std::string& folder, bool is_favorite, ReadStatus read_status, 
             std::time_t created_at, std::time_t updated_at, std::time_t last_accessed_at, 
             int click_count);

    // Getters
    int id() const { return id_; }
    int user_id() const { return user_id_; }
    const std::string& url() const { return url_; }
    const std::string& title() const { return title_; }
    const std::string& description() const { return description_; }
    const std::vector<std::string>& tags() const { return tags_; }
    const std::string& folder() const { return folder_; }
    bool is_favorite() const { return is_favorite_; }
    ReadStatus read_status() const { return read_status_; }
    std::time_t created_at() const { return created_at_; }
    std::time_t updated_at() const { return updated_at_; }
    std::time_t last_accessed_at() const { return last_accessed_at_; }
    int click_count() const { return click_count_; }

    // Setters
    void set_url(const std::string& url) { url_ = url; }
    void set_title(const std::string& title) { title_ = title; }
    void set_description(const std::string& description) { description_ = description; }
    void set_tags(const std::vector<std::string>& tags) { tags_ = tags; }
    void set_folder(const std::string& folder) { folder_ = folder; }
    void set_is_favorite(bool is_favorite) { is_favorite_ = is_favorite; }
    void set_read_status(ReadStatus read_status) { read_status_ = read_status; }
    void set_updated_at(std::time_t updated_at) { updated_at_ = updated_at; }
    void set_last_accessed_at(std::time_t last_accessed_at) { last_accessed_at_ = last_accessed_at; }
    void set_click_count(int click_count) { click_count_ = click_count; }
    void increment_click_count() { click_count_++; }

    // Helper methods
    static std::string read_status_to_string(ReadStatus status);
    static ReadStatus read_status_from_string(const std::string& str);
    static ReadStatus string_to_read_status(const std::string& str);
};

// JSON serialization
nlohmann::json to_json(const Bookmark& bookmark);
nlohmann::json to_json(const User& user);
nlohmann::json to_json(const UserStats& stats);
nlohmann::json to_json(const DomainStats& stats);
nlohmann::json to_json(const DailyStats& stats);
nlohmann::json to_json(const TagStats& stats);
nlohmann::json to_json(const FolderStats& stats);
nlohmann::json to_json(const std::vector<std::shared_ptr<Bookmark>>& bookmarks);
nlohmann::json to_json(const std::vector<DailyStats>& daily_stats);
nlohmann::json to_json(const std::vector<DomainStats>& domains);
nlohmann::json to_json(const std::vector<TagStats>& tags);
nlohmann::json to_json(const std::vector<FolderStats>& folders);

} // namespace models
