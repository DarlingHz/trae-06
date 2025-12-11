#pragma once

#include "models/Bookmark.h"
#include "models/Stats.h"
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include "DatabasePool.h"

namespace repository {

struct BookmarkQuery {
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

class BookmarkRepository {
public:
    virtual ~BookmarkRepository() = default;

    virtual std::shared_ptr<models::Bookmark> create(int user_id, const std::string& url, 
                                                    const std::string& title, const std::string& description, 
                                                    const std::vector<std::string>& tags, const std::string& folder, 
                                                    bool is_favorite, models::ReadStatus read_status) = 0;
    
    virtual std::shared_ptr<models::Bookmark> find_by_id(int id) = 0;
    
    virtual std::vector<std::shared_ptr<models::Bookmark>> find_by_user(int user_id) = 0;
    
    virtual std::vector<std::shared_ptr<models::Bookmark>> query(int user_id, const BookmarkQuery& query) = 0;
    
    virtual int count(int user_id, const BookmarkQuery& query) = 0;
    
    virtual bool update(std::shared_ptr<models::Bookmark> bookmark) = 0;
    
    virtual bool delete_by_id(int id) = 0;
    
    virtual bool batch_update_read_status(int user_id, const std::vector<int>& ids, models::ReadStatus status) = 0;
    
    virtual bool batch_move_to_folder(int user_id, const std::vector<int>& ids, const std::string& folder) = 0;
    
    virtual bool batch_delete(int user_id, const std::vector<int>& ids) = 0;
    
    virtual bool increment_click_count(int id) = 0;
    
    virtual bool update_last_accessed(int id) = 0;
    
    // Stats
    virtual models::UserStats get_user_stats(int user_id) = 0;
    
    virtual std::vector<models::DailyStats> get_daily_stats(int user_id, int days = 14) = 0;
    
    virtual std::vector<models::DomainStats> get_top_domains(int user_id, int limit = 10) = 0;
    
    virtual std::vector<models::TagStats> get_user_tags(int user_id) = 0;
    
    virtual std::vector<models::FolderStats> get_user_folders(int user_id) = 0;
    
    virtual bool rename_tag(int user_id, const std::string& old_tag, const std::string& new_tag) = 0;
    
    virtual bool delete_tag(int user_id, const std::string& tag, bool remove_from_bookmarks = true) = 0;
    
    virtual bool rename_folder(int user_id, const std::string& old_name, const std::string& new_name) = 0;
    
    virtual bool delete_folder(int user_id, const std::string& folder_name, bool remove_bookmarks = true) = 0;
};

std::unique_ptr<BookmarkRepository> create_bookmark_repository(DatabasePool& db_pool);

} // namespace repository
