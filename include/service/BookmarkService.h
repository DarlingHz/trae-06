#pragma once

#include "models/Bookmark.h"
#include "models/Stats.h"
#include "repository/BookmarkRepository.h"
#include <memory>
#include <vector>
#include <string>
#include <optional>

namespace service {

struct BookmarkCreateRequest {
    std::string url;
    std::string title;
    std::string description;
    std::vector<std::string> tags;
    std::string folder;
    bool is_favorite = false;
    models::ReadStatus read_status = models::ReadStatus::UNREAD;
};

void from_json(const nlohmann::json& j, BookmarkCreateRequest& req);

struct BookmarkUpdateRequest {
    std::optional<std::string> url;
    std::optional<std::string> title;
    std::optional<std::string> description;
    std::optional<std::vector<std::string>> tags;
    std::optional<std::string> folder;
    std::optional<bool> is_favorite;
    std::optional<models::ReadStatus> read_status;
};

void from_json(const nlohmann::json& j, BookmarkUpdateRequest& req);

struct BookmarkQueryRequest {
    std::vector<std::string> tags;
    std::optional<std::string> folder;
    std::optional<models::ReadStatus> read_status;
    std::optional<bool> is_favorite;
    std::optional<std::string> search_keyword;
    int page = 1;
    int page_size = 20;
    std::string sort_by = "created_at";
    bool sort_desc = false;
};

struct BatchUpdateRequest {
    std::vector<int> ids;
};

struct BatchMoveRequest {
    std::vector<int> ids;
    std::string folder;
};

void from_json(const nlohmann::json& j, BatchUpdateRequest& req);
void from_json(const nlohmann::json& j, BatchMoveRequest& req);

// JSON deserialization
#include <nlohmann/json.hpp>

namespace service {
    void from_json(const nlohmann::json& j, BatchUpdateRequest& req);
    void from_json(const nlohmann::json& j, BatchMoveRequest& req);
    void from_json(const nlohmann::json& j, BookmarkCreateRequest& req);
    void from_json(const nlohmann::json& j, BookmarkUpdateRequest& req);
    void from_json(const nlohmann::json& j, BookmarkQueryRequest& req);
}

struct BookmarkQueryResult {
    std::vector<std::shared_ptr<models::Bookmark>> bookmarks;
    int total;
    int page;
    int page_size;
    int total_pages;
};

class BookmarkService {
private:
    std::unique_ptr<repository::BookmarkRepository> bookmark_repo_;

    bool validate_url(const std::string& url) const;

public:
    BookmarkService(std::unique_ptr<repository::BookmarkRepository> bookmark_repo);

    std::shared_ptr<models::Bookmark> create_bookmark(int user_id, const BookmarkCreateRequest& request);
    
    std::shared_ptr<models::Bookmark> get_bookmark(int id, int user_id);
    
    bool update_bookmark(int id, int user_id, const BookmarkUpdateRequest& request);
    bool mark_as_read(int id, int user_id);
    
    bool delete_bookmark(int id, int user_id);
    
    BookmarkQueryResult query_bookmarks(int user_id, const BookmarkQueryRequest& request);
    
    bool batch_update_read_status(int user_id, const BatchUpdateRequest& request, models::ReadStatus status);
    
    bool batch_move_to_folder(int user_id, const BatchMoveRequest& request);
    
    bool batch_delete(int user_id, const BatchUpdateRequest& request);
    
    bool record_click(int id, int user_id);
    
    // Stats
    models::UserStats get_user_stats(int user_id);
    
    std::vector<models::DailyStats> get_daily_stats(int user_id, int days = 14);
    
    std::vector<models::DomainStats> get_top_domains(int user_id, int limit = 10);
    
    std::vector<models::TagStats> get_user_tags(int user_id);
    
    std::vector<models::FolderStats> get_user_folders(int user_id);
    
    bool rename_tag(int user_id, const std::string& old_tag, const std::string& new_tag);
    
    bool delete_tag(int user_id, const std::string& tag, bool remove_from_bookmarks = true);
    
    bool rename_folder(int user_id, const std::string& old_name, const std::string& new_name);
    
    bool delete_folder(int user_id, const std::string& folder_name, bool remove_bookmarks = true);
};

} // namespace service
