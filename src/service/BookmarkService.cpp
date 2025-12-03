#include "service/BookmarkService.h"
#include <regex>
#include <stdexcept>
#include <algorithm>
#include <nlohmann/json.hpp>

namespace service {

BookmarkService::BookmarkService(std::unique_ptr<repository::BookmarkRepository> bookmark_repo)
    : bookmark_repo_(std::move(bookmark_repo)) {}

bool BookmarkService::validate_url(const std::string& url) const {
    const std::regex url_regex(R"(^(http|https)://[^\s]+$)");
    return std::regex_match(url, url_regex);
}

std::shared_ptr<models::Bookmark> BookmarkService::create_bookmark(int user_id, const BookmarkCreateRequest& request) {
    if(request.url.empty()) {
        throw std::invalid_argument("URL is required");
    }
    
    if(request.title.empty()) {
        throw std::invalid_argument("Title is required");
    }
    
    if(!validate_url(request.url)) {
        throw std::invalid_argument("Invalid URL format");
    }
    
    return bookmark_repo_->create(user_id, request.url, request.title, request.description, 
                                 request.tags, request.folder, request.is_favorite, request.read_status);
}

std::shared_ptr<models::Bookmark> BookmarkService::get_bookmark(int id, int user_id) {
    auto bookmark = bookmark_repo_->find_by_id(id);
    
    if(!bookmark || bookmark->user_id() != user_id) {
        return nullptr;
    }
    
    return bookmark;
}

bool BookmarkService::update_bookmark(int id, int user_id, const BookmarkUpdateRequest& request) {
    auto bookmark = get_bookmark(id, user_id);
    
    if(!bookmark) {
        return false;
    }
    
    if(request.url) {
        if(!validate_url(*request.url)) {
            throw std::invalid_argument("Invalid URL format");
        }
        bookmark->set_url(*request.url);
    }
    
    if(request.title) {
        if(request.title->empty()) {
            throw std::invalid_argument("Title cannot be empty");
        }
        bookmark->set_title(*request.title);
    }
    
    if(request.description) {
        bookmark->set_description(*request.description);
    }
    
    if(request.tags) {
        bookmark->set_tags(*request.tags);
    }
    
    if(request.folder) {
        bookmark->set_folder(*request.folder);
    }
    
    if(request.is_favorite) {
        bookmark->set_is_favorite(*request.is_favorite);
    }
    
    if(request.read_status) {
        bookmark->set_read_status(*request.read_status);
    }
    
    return bookmark_repo_->update(bookmark);
}

bool BookmarkService::mark_as_read(int id, int user_id) {
    auto bookmark = bookmark_repo_->find_by_id(id);
    if(!bookmark || bookmark->user_id() != user_id) {
        return false;
    }

    bookmark->set_read_status(models::ReadStatus::READ);
    bookmark->set_updated_at(std::time(nullptr));

    return bookmark_repo_->update(bookmark);
}

bool BookmarkService::delete_bookmark(int id, int user_id) {
    auto bookmark = get_bookmark(id, user_id);
    
    if(!bookmark) {
        return false;
    }
    
    return bookmark_repo_->delete_by_id(id);
}

BookmarkQueryResult BookmarkService::query_bookmarks(int user_id, const BookmarkQueryRequest& request) {
    repository::BookmarkQuery query;
    query.tags = request.tags;
    query.folder = request.folder;
    query.read_status = request.read_status;
    query.is_favorite = request.is_favorite;
    query.search_keyword = request.search_keyword;
    query.page = request.page;
    query.page_size = request.page_size;
    query.sort_by = request.sort_by;
    query.sort_desc = request.sort_desc;
    
    auto bookmarks = bookmark_repo_->query(user_id, query);
    int total = bookmark_repo_->count(user_id, query);
    int total_pages = (total + request.page_size - 1) / request.page_size;
    
    return {bookmarks, total, request.page, request.page_size, total_pages};
}

bool BookmarkService::batch_update_read_status(int user_id, const BatchUpdateRequest& request, models::ReadStatus status) {
    if(request.ids.empty()) {
        return true;
    }
    
    return bookmark_repo_->batch_update_read_status(user_id, request.ids, status);
}

bool BookmarkService::batch_move_to_folder(int user_id, const BatchMoveRequest& request) {
    if(request.ids.empty()) {
        return true;
    }
    
    if(request.folder.empty()) {
        throw std::invalid_argument("Folder is required");
    }
    
    return bookmark_repo_->batch_move_to_folder(user_id, request.ids, request.folder);
}

bool BookmarkService::batch_delete(int user_id, const BatchUpdateRequest& request) {
    if(request.ids.empty()) {
        return true;
    }
    
    return bookmark_repo_->batch_delete(user_id, request.ids);
}

bool BookmarkService::record_click(int id, int user_id) {
    auto bookmark = get_bookmark(id, user_id);
    
    if(!bookmark) {
        return false;
    }
    
    bool success = bookmark_repo_->increment_click_count(id);
    if(success) {
        success = bookmark_repo_->update_last_accessed(id);
    }
    
    return success;
}

models::UserStats BookmarkService::get_user_stats(int user_id) {
    return bookmark_repo_->get_user_stats(user_id);
}

std::vector<models::DailyStats> BookmarkService::get_daily_stats(int user_id, int days) {
    if(days <= 0) {
        days = 14;
    }
    return bookmark_repo_->get_daily_stats(user_id, days);
}

std::vector<models::DomainStats> BookmarkService::get_top_domains(int user_id, int limit) {
    if(limit <= 0) {
        limit = 10;
    }
    return bookmark_repo_->get_top_domains(user_id, limit);
}

std::vector<models::TagStats> BookmarkService::get_user_tags(int user_id) {
    return bookmark_repo_->get_user_tags(user_id);
}

std::vector<models::FolderStats> BookmarkService::get_user_folders(int user_id) {
    return bookmark_repo_->get_user_folders(user_id);
}

bool BookmarkService::rename_tag(int user_id, const std::string& old_tag, const std::string& new_tag) {
    if(old_tag.empty() || new_tag.empty()) {
        throw std::invalid_argument("Tag names cannot be empty");
    }
    
    return bookmark_repo_->rename_tag(user_id, old_tag, new_tag);
}

bool BookmarkService::delete_tag(int user_id, const std::string& tag, bool remove_from_bookmarks) {
    if(tag.empty()) {
        throw std::invalid_argument("Tag name cannot be empty");
    }
    
    return bookmark_repo_->delete_tag(user_id, tag, remove_from_bookmarks);
}

bool BookmarkService::rename_folder(int user_id, const std::string& old_name, const std::string& new_name) {
    if(old_name.empty() || new_name.empty()) {
        throw std::invalid_argument("Folder names cannot be empty");
    }
    
    return bookmark_repo_->rename_folder(user_id, old_name, new_name);
}

bool BookmarkService::delete_folder(int user_id, const std::string& folder_name, bool remove_bookmarks) {
    if(folder_name.empty()) {
        throw std::invalid_argument("Folder name cannot be empty");
    }
    
    return bookmark_repo_->delete_folder(user_id, folder_name, remove_bookmarks);
}

// JSON deserialization
void from_json(const nlohmann::json& j, BookmarkUpdateRequest& req) {
    if(j.contains("url")) {
        if(!j["url"].is_null()) req.url = j["url"].template get<std::string>();
    }
    if(j.contains("title")) {
        if(!j["title"].is_null()) req.title = j["title"].template get<std::string>();
    }
    if(j.contains("description")) {
        if(!j["description"].is_null()) req.description = j["description"].template get<std::string>();
    }
    if(j.contains("tags")) {
        if(!j["tags"].is_null()) req.tags = j["tags"].template get<std::vector<std::string>>();
    }
    if(j.contains("folder")) {
        if(!j["folder"].is_null()) req.folder = j["folder"].template get<std::string>();
    }
    if(j.contains("is_favorite")) {
        if(!j["is_favorite"].is_null()) req.is_favorite = j["is_favorite"].template get<bool>();
    }
    if(j.contains("read_status")) {
        std::string status_str = j["read_status"];
        req.read_status = models::Bookmark::string_to_read_status(status_str);
    }
}

void from_json(const nlohmann::json& j, BatchUpdateRequest& req) {
    j["ids"].get_to(req.ids);
}

void from_json(const nlohmann::json& j, BatchMoveRequest& req) {
    j["ids"].get_to(req.ids);
    j["folder"].get_to(req.folder);
}

void from_json(const nlohmann::json& j, BookmarkCreateRequest& req) {
    j["url"].get_to(req.url);
    j["title"].get_to(req.title);
    if(j.contains("description")) j["description"].get_to(req.description);
    if(j.contains("tags")) j["tags"].get_to(req.tags);
    if(j.contains("folder")) j["folder"].get_to(req.folder);
    if(j.contains("is_favorite")) j["is_favorite"].get_to(req.is_favorite);
    if(j.contains("read_status")) {
        std::string status_str = j["read_status"];
        req.read_status = models::Bookmark::string_to_read_status(status_str);
    }
}

void from_json(const nlohmann::json& j, BookmarkQueryRequest& req) {
    if(j.contains("tags")) j["tags"].get_to(req.tags);
    if(j.contains("folder")) req.folder = j["folder"];
    if(j.contains("read_status")) {
        std::string status_str = j["read_status"];
        req.read_status = models::Bookmark::string_to_read_status(status_str);
    }
    if(j.contains("is_favorite")) req.is_favorite = j["is_favorite"];
    if(j.contains("search_keyword")) req.search_keyword = j["search_keyword"];
    if(j.contains("page")) j["page"].get_to(req.page);
    if(j.contains("page_size")) j["page_size"].get_to(req.page_size);
    if(j.contains("sort_by")) j["sort_by"].get_to(req.sort_by);
    if(j.contains("sort_desc")) j["sort_desc"].get_to(req.sort_desc);
}

} // namespace service
