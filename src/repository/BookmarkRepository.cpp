#include "repository/BookmarkRepository.h"
#include "repository/DatabasePool.h"
#include <memory>
#include <ctime>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

namespace repository {

std::vector<std::string> split_tags(const std::string& tags_str) {
    std::vector<std::string> tags;
    std::stringstream ss(tags_str);
    std::string tag;
    
    while(std::getline(ss, tag, ',')) {
        if(!tag.empty()) {
            tags.push_back(tag);
        }
    }
    
    return tags;
}

std::string join_tags(const std::vector<std::string>& tags) {
    if(tags.empty()) return "";
    
    std::stringstream ss;
    ss << tags[0];
    
    for(size_t i = 1; i < tags.size(); ++i) {
        ss << "," << tags[i];
    }
    
    return ss.str();
}

class SQLiteBookmarkRepository : public BookmarkRepository {
private:
    DatabasePool& db_pool_;

    std::shared_ptr<models::Bookmark> bookmark_from_row(sqlite3_stmt* stmt) {
        int id = sqlite3_column_int(stmt, 0);
        int user_id = sqlite3_column_int(stmt, 1);
        const char* url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        const char* tags_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char* folder = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        bool is_favorite = sqlite3_column_int(stmt, 7) == 1;
        const char* read_status_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        std::time_t created_at = sqlite3_column_int64(stmt, 9);
        std::time_t updated_at = sqlite3_column_int64(stmt, 10);
        std::time_t last_accessed_at = sqlite3_column_int64(stmt, 11);
        int click_count = sqlite3_column_int(stmt, 12);
        
        std::vector<std::string> tags = tags_str ? split_tags(tags_str) : std::vector<std::string>();
        models::ReadStatus read_status = models::Bookmark::string_to_read_status(read_status_str ? read_status_str : "");
        
        return std::make_shared<models::Bookmark>(id, user_id, 
                                                  url ? url : "", 
                                                  title ? title : "", 
                                                  description ? description : "", 
                                                  tags, 
                                                  folder ? folder : "", 
                                                  is_favorite, 
                                                  read_status, 
                                                  created_at, 
                                                  updated_at, 
                                                  last_accessed_at, 
                                                  click_count);
    }

    void build_query_sql(int user_id, const BookmarkQuery& query, 
                        std::string& sql, std::vector<std::string>& params) {
        std::ostringstream oss;
        
        oss << "SELECT id, user_id, url, title, description, tags, folder, is_favorite, read_status, created_at, updated_at, last_accessed_at, click_count FROM bookmarks WHERE user_id = ?";
        params.push_back(std::to_string(user_id));
        
        if(!query.tags.empty()) {
            for(const auto& tag : query.tags) {
                oss << " AND tags LIKE ?";
                params.push_back("%" + tag + "%");
            }
        }
        
        if(query.folder) {
            oss << " AND folder = ?";
            params.push_back(*query.folder);
        }
        
        if(query.read_status) {
            oss << " AND read_status = ?";
            params.push_back(models::Bookmark::read_status_to_string(*query.read_status));
        }
        
        if(query.is_favorite) {
            oss << " AND is_favorite = ?";
            params.push_back(*query.is_favorite ? "1" : "0");
        }
        
        if(query.search_keyword) {
            oss << " AND (title LIKE ? OR description LIKE ?)";
            params.push_back("%" + *query.search_keyword + "%");
            params.push_back("%" + *query.search_keyword + "%");
        }
        
        oss << " ORDER BY " << query.sort_by;
        if(query.sort_desc) {
            oss << " DESC";
        } else {
            oss << " ASC";
        }
        
        int offset = (query.page - 1) * query.page_size;
        oss << " LIMIT ? OFFSET ?";
        params.push_back(std::to_string(query.page_size));
        params.push_back(std::to_string(offset));
        
        sql = oss.str();
    }

public:
    SQLiteBookmarkRepository(DatabasePool& db_pool) : db_pool_(db_pool) {}

    std::shared_ptr<models::Bookmark> create(int user_id, const std::string& url, 
                                            const std::string& title, const std::string& description, 
                                            const std::vector<std::string>& tags, const std::string& folder, 
                                            bool is_favorite, models::ReadStatus read_status) override {
        auto conn = db_pool_.get_connection();
        
        const char* sql = "INSERT INTO bookmarks (user_id, url, title, description, tags, folder, is_favorite, read_status, created_at, updated_at, last_accessed_at, click_count) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        std::time_t now = std::time(nullptr);
        std::string tags_str = join_tags(tags);
        std::string status_str = models::Bookmark::read_status_to_string(read_status);
        
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_text(stmt, 2, url.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, title.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, description.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, tags_str.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 6, folder.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 7, is_favorite ? 1 : 0);
        sqlite3_bind_text(stmt, 8, status_str.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 9, now);
        sqlite3_bind_int64(stmt, 10, now);
        sqlite3_bind_int64(stmt, 11, now);
        sqlite3_bind_int(stmt, 12, 0);
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        int bookmark_id = sqlite3_last_insert_rowid(conn.get());
        sqlite3_finalize(stmt);
        
        return std::make_shared<models::Bookmark>(bookmark_id, user_id, url, title, description, tags, folder, is_favorite, read_status, now, now, now, 0);
    }

    std::shared_ptr<models::Bookmark> find_by_id(int id) override {
        auto conn = db_pool_.get_connection();
        
        const char* sql = "SELECT id, user_id, url, title, description, tags, folder, is_favorite, read_status, created_at, updated_at, last_accessed_at, click_count FROM bookmarks WHERE id = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        sqlite3_bind_int(stmt, 1, id);
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_ROW) {
            sqlite3_finalize(stmt);
            return nullptr;
        }
        
        auto bookmark = bookmark_from_row(stmt);
        sqlite3_finalize(stmt);
        
        return bookmark;
    }

    std::vector<std::shared_ptr<models::Bookmark>> find_by_user(int user_id) override {
        auto conn = db_pool_.get_connection();
        
        const char* sql = "SELECT id, user_id, url, title, description, tags, folder, is_favorite, read_status, created_at, updated_at, last_accessed_at, click_count FROM bookmarks WHERE user_id = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        sqlite3_bind_int(stmt, 1, user_id);
        
        std::vector<std::shared_ptr<models::Bookmark>> bookmarks;
        
        while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            bookmarks.push_back(bookmark_from_row(stmt));
        }
        
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        sqlite3_finalize(stmt);
        return bookmarks;
    }

    std::vector<std::shared_ptr<models::Bookmark>> query(int user_id, const BookmarkQuery& query) override {
        auto conn = db_pool_.get_connection();
        
        std::string sql;
        std::vector<std::string> params;
        build_query_sql(user_id, query, sql, params);
        
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(conn.get(), sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        for(size_t i = 0; i < params.size(); ++i) {
            sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_STATIC);
        }
        
        std::vector<std::shared_ptr<models::Bookmark>> bookmarks;
        
        while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            bookmarks.push_back(bookmark_from_row(stmt));
        }
        
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        sqlite3_finalize(stmt);
        return bookmarks;
    }

    int count(int user_id, const BookmarkQuery& query) override {
        auto conn = db_pool_.get_connection();
        
        std::string sql_base;
        std::vector<std::string> params;
        
        std::ostringstream oss;
        oss << "SELECT COUNT(*) FROM bookmarks WHERE user_id = ?";
        params.push_back(std::to_string(user_id));
        
        if(!query.tags.empty()) {
            for(const auto& tag : query.tags) {
                oss << " AND tags LIKE ?";
                params.push_back("%" + tag + "%");
            }
        }
        
        if(query.folder) {
            oss << " AND folder = ?";
            params.push_back(*query.folder);
        }
        
        if(query.read_status) {
            oss << " AND read_status = ?";
            params.push_back(models::Bookmark::read_status_to_string(*query.read_status));
        }
        
        if(query.is_favorite) {
            oss << " AND is_favorite = ?";
            params.push_back(*query.is_favorite ? "1" : "0");
        }
        
        if(query.search_keyword) {
            oss << " AND (title LIKE ? OR description LIKE ?)";
            params.push_back("%" + *query.search_keyword + "%");
            params.push_back("%" + *query.search_keyword + "%");
        }
        
        sql_base = oss.str();
        
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(conn.get(), sql_base.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        for(size_t i = 0; i < params.size(); ++i) {
            sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_STATIC);
        }
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_ROW) {
            sqlite3_finalize(stmt);
            return 0;
        }
        
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        
        return count;
    }

    bool update(std::shared_ptr<models::Bookmark> bookmark) override {
        if(!bookmark) return false;
        
        auto conn = db_pool_.get_connection();
        
        const char* sql = "UPDATE bookmarks SET url = ?, title = ?, description = ?, tags = ?, folder = ?, is_favorite = ?, read_status = ?, updated_at = ?, last_accessed_at = ?, click_count = ? WHERE id = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        std::time_t now = std::time(nullptr);
        std::string tags_str = join_tags(bookmark->tags());
        std::string status_str = models::Bookmark::read_status_to_string(bookmark->read_status());
        
        sqlite3_bind_text(stmt, 1, bookmark->url().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, bookmark->title().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, bookmark->description().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, tags_str.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, bookmark->folder().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 6, bookmark->is_favorite() ? 1 : 0);
        sqlite3_bind_text(stmt, 7, status_str.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 8, now);
        sqlite3_bind_int64(stmt, 9, bookmark->last_accessed_at());
        sqlite3_bind_int(stmt, 10, bookmark->click_count());
        sqlite3_bind_int(stmt, 11, bookmark->id());
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        int changes = sqlite3_changes(conn.get());
        sqlite3_finalize(stmt);
        
        if(changes > 0) {
            bookmark->set_updated_at(now);
        }
        
        return changes > 0;
    }

    bool delete_by_id(int id) override {
        auto conn = db_pool_.get_connection();
        
        const char* sql = "DELETE FROM bookmarks WHERE id = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        sqlite3_bind_int(stmt, 1, id);
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        int changes = sqlite3_changes(conn.get());
        sqlite3_finalize(stmt);
        
        return changes > 0;
    }

    bool batch_update_read_status(int user_id, const std::vector<int>& ids, models::ReadStatus status) override {
        if(ids.empty()) return true;
        
        auto conn = db_pool_.get_connection();
        
        std::ostringstream oss;
        oss << "UPDATE bookmarks SET read_status = ?, updated_at = ? WHERE user_id = ? AND id IN (";
        
        for(size_t i = 0; i < ids.size(); ++i) {
            if(i > 0) oss << ",";
            oss << "?";
        }
        oss << ")";
        
        std::string sql = oss.str();
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        std::time_t now = std::time(nullptr);
        std::string status_str = models::Bookmark::read_status_to_string(status);
        
        int param_idx = 1;
        sqlite3_bind_text(stmt, param_idx++, status_str.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, param_idx++, now);
        sqlite3_bind_int(stmt, param_idx++, user_id);
        
        for(int id : ids) {
            sqlite3_bind_int(stmt, param_idx++, id);
        }
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        int changes = sqlite3_changes(conn.get());
        sqlite3_finalize(stmt);
        
        return changes >= 0;
    }

    bool batch_move_to_folder(int user_id, const std::vector<int>& ids, const std::string& folder) override {
        if(ids.empty()) return true;
        
        auto conn = db_pool_.get_connection();
        
        std::ostringstream oss;
        oss << "UPDATE bookmarks SET folder = ?, updated_at = ? WHERE user_id = ? AND id IN (";
        
        for(size_t i = 0; i < ids.size(); ++i) {
            if(i > 0) oss << ",";
            oss << "?";
        }
        oss << ")";
        
        std::string sql = oss.str();
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        std::time_t now = std::time(nullptr);
        
        int param_idx = 1;
        sqlite3_bind_text(stmt, param_idx++, folder.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, param_idx++, now);
        sqlite3_bind_int(stmt, param_idx++, user_id);
        
        for(int id : ids) {
            sqlite3_bind_int(stmt, param_idx++, id);
        }
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        int changes = sqlite3_changes(conn.get());
        sqlite3_finalize(stmt);
        
        return changes >= 0;
    }

    bool batch_delete(int user_id, const std::vector<int>& ids) override {
        if(ids.empty()) return true;
        
        auto conn = db_pool_.get_connection();
        
        std::ostringstream oss;
        oss << "DELETE FROM bookmarks WHERE user_id = ? AND id IN (";
        
        for(size_t i = 0; i < ids.size(); ++i) {
            if(i > 0) oss << ",";
            oss << "?";
        }
        oss << ")";
        
        std::string sql = oss.str();
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        int param_idx = 1;
        sqlite3_bind_int(stmt, param_idx++, user_id);
        
        for(int id : ids) {
            sqlite3_bind_int(stmt, param_idx++, id);
        }
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        int changes = sqlite3_changes(conn.get());
        sqlite3_finalize(stmt);
        
        return changes >= 0;
    }

    bool increment_click_count(int id) override {
        auto conn = db_pool_.get_connection();
        
        const char* sql = "UPDATE bookmarks SET click_count = click_count + 1 WHERE id = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        sqlite3_bind_int(stmt, 1, id);
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        int changes = sqlite3_changes(conn.get());
        sqlite3_finalize(stmt);
        
        return changes > 0;
    }

    bool update_last_accessed(int id) override {
        auto conn = db_pool_.get_connection();
        
        const char* sql = "UPDATE bookmarks SET last_accessed_at = ?, updated_at = ? WHERE id = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        std::time_t now = std::time(nullptr);
        
        sqlite3_bind_int64(stmt, 1, now);
        sqlite3_bind_int64(stmt, 2, now);
        sqlite3_bind_int(stmt, 3, id);
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        int changes = sqlite3_changes(conn.get());
        sqlite3_finalize(stmt);
        
        return changes > 0;
    }

    models::UserStats get_user_stats(int user_id) override {
        auto conn = db_pool_.get_connection();
        
        const char* sql = "SELECT \
            COUNT(*) as total, \
            SUM(CASE WHEN read_status = 'unread' THEN 1 ELSE 0 END) as unread, \
            SUM(CASE WHEN read_status = 'reading' THEN 1 ELSE 0 END) as reading, \
            SUM(CASE WHEN read_status = 'read' THEN 1 ELSE 0 END) as read, \
            SUM(CASE WHEN is_favorite = 1 THEN 1 ELSE 0 END) as favorite \
            FROM bookmarks WHERE user_id = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        sqlite3_bind_int(stmt, 1, user_id);
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_ROW) {
            sqlite3_finalize(stmt);
            return models::UserStats(0, 0, 0, 0, 0);
        }
        
        int total = sqlite3_column_int(stmt, 0);
        int unread = sqlite3_column_int(stmt, 1);
        int reading = sqlite3_column_int(stmt, 2);
        int read = sqlite3_column_int(stmt, 3);
        int favorite = sqlite3_column_int(stmt, 4);
        
        sqlite3_finalize(stmt);
        
        return models::UserStats(total, unread, reading, read, favorite);
    }

    std::vector<models::DailyStats> get_daily_stats(int user_id, int days) override {
        auto conn = db_pool_.get_connection();
        
        std::ostringstream oss;
        oss << "SELECT DATE(created_at, 'unixepoch') as date, COUNT(*) as count FROM bookmarks WHERE user_id = ? AND created_at >= strftime('%s', 'now', '-" << days << " days') GROUP BY date ORDER BY date;";
        
        std::string sql = oss.str();
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        sqlite3_bind_int(stmt, 1, user_id);
        
        std::vector<models::DailyStats> stats;
        
        while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            const char* date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int count = sqlite3_column_int(stmt, 1);
            
            if(date) {
                stats.emplace_back(date, count);
            }
        }
        
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        sqlite3_finalize(stmt);
        return stats;
    }

    std::vector<models::DomainStats> get_top_domains(int user_id, int limit) override {
        auto conn = db_pool_.get_connection();
        
        std::ostringstream oss;
        oss << "SELECT SUBSTR(url, INSTR(url, '//') + 2, INSTR(SUBSTR(url, INSTR(url, '//') + 2), '/') - 1) as domain, SUM(click_count) as total_clicks FROM bookmarks WHERE user_id = ? GROUP BY domain ORDER BY total_clicks DESC LIMIT ?;";
        
        std::string sql = oss.str();
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_int(stmt, 2, limit);
        
        std::vector<models::DomainStats> stats;
        
        while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            const char* domain = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int count = sqlite3_column_int(stmt, 1);
            
            if(domain) {
                stats.emplace_back(domain, count);
            }
        }
        
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        sqlite3_finalize(stmt);
        return stats;
    }

    std::vector<models::TagStats> get_user_tags(int user_id) override {
        auto conn = db_pool_.get_connection();
        
        // First get all bookmarks and extract tags
        std::vector<std::shared_ptr<models::Bookmark>> bookmarks = find_by_user(user_id);
        
        std::map<std::string, int> tag_counts;
        
        for(const auto& bookmark : bookmarks) {
            for(const auto& tag : bookmark->tags()) {
                if(!tag.empty()) {
                    tag_counts[tag]++;
                }
            }
        }
        
        std::vector<models::TagStats> stats;
        for(const auto& pair : tag_counts) {
            stats.emplace_back(pair.first, pair.second);
        }
        
        return stats;
    }

    std::vector<models::FolderStats> get_user_folders(int user_id) override {
        auto conn = db_pool_.get_connection();
        
        const char* sql = "SELECT folder, COUNT(*) as count FROM bookmarks WHERE user_id = ? GROUP BY folder ORDER BY folder;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        sqlite3_bind_int(stmt, 1, user_id);
        
        std::vector<models::FolderStats> stats;
        
        while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            const char* folder = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int count = sqlite3_column_int(stmt, 1);
            
            if(folder) {
                stats.emplace_back(folder, count);
            }
        }
        
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        sqlite3_finalize(stmt);
        return stats;
    }

    bool rename_tag(int user_id, const std::string& old_tag, const std::string& new_tag) override {
        auto conn = db_pool_.get_connection();
        
        const char* sql = "UPDATE bookmarks SET tags = REPLACE(tags, ?, ?), updated_at = ? WHERE user_id = ? AND tags LIKE ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        std::time_t now = std::time(nullptr);
        std::string search_pattern = "%" + old_tag + "%";
        
        sqlite3_bind_text(stmt, 1, old_tag.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, new_tag.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 3, now);
        sqlite3_bind_int(stmt, 4, user_id);
        sqlite3_bind_text(stmt, 5, search_pattern.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        int changes = sqlite3_changes(conn.get());
        sqlite3_finalize(stmt);
        
        return changes >= 0;
    }

    bool delete_tag(int user_id, const std::string& tag, bool remove_from_bookmarks) override {
        if(!remove_from_bookmarks) {
            // Delete bookmarks with this tag
            auto conn = db_pool_.get_connection();
            
            const char* sql = "DELETE FROM bookmarks WHERE user_id = ? AND tags LIKE ?;";
            sqlite3_stmt* stmt;
            
            int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
            if(rc != SQLITE_OK) {
                throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
            }
            
            std::string search_pattern = "%" + tag + "%";
            
            sqlite3_bind_int(stmt, 1, user_id);
            sqlite3_bind_text(stmt, 2, search_pattern.c_str(), -1, SQLITE_STATIC);
            
            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
            }
            
            int changes = sqlite3_changes(conn.get());
            sqlite3_finalize(stmt);
            
            return changes >= 0;
        } else {
            // Remove tag from bookmarks but keep bookmarks
            auto conn = db_pool_.get_connection();
            
            // This is more complex - need to update each bookmark's tags
            std::vector<std::shared_ptr<models::Bookmark>> bookmarks = find_by_user(user_id);
            
            for(auto& bookmark : bookmarks) {
                auto tags = bookmark->tags();
                auto it = std::remove(tags.begin(), tags.end(), tag);
                if(it != tags.end()) {
                    tags.erase(it, tags.end());
                    bookmark->set_tags(tags);
                    update(bookmark);
                }
            }
            
            return true;
        }
    }

    bool rename_folder(int user_id, const std::string& old_name, const std::string& new_name) override {
        auto conn = db_pool_.get_connection();
        
        const char* sql = "UPDATE bookmarks SET folder = ?, updated_at = ? WHERE user_id = ? AND folder = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        std::time_t now = std::time(nullptr);
        
        sqlite3_bind_text(stmt, 1, new_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 2, now);
        sqlite3_bind_int(stmt, 3, user_id);
        sqlite3_bind_text(stmt, 4, old_name.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        int changes = sqlite3_changes(conn.get());
        sqlite3_finalize(stmt);
        
        return changes >= 0;
    }

    bool delete_folder(int user_id, const std::string& folder_name, bool remove_bookmarks) override {
        auto conn = db_pool_.get_connection();
        
        if(remove_bookmarks) {
            // Delete all bookmarks in this folder
            const char* sql = "DELETE FROM bookmarks WHERE user_id = ? AND folder = ?;";
            sqlite3_stmt* stmt;
            
            int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
            if(rc != SQLITE_OK) {
                throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
            }
            
            sqlite3_bind_int(stmt, 1, user_id);
            sqlite3_bind_text(stmt, 2, folder_name.c_str(), -1, SQLITE_STATIC);
            
            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
            }
            
            int changes = sqlite3_changes(conn.get());
            sqlite3_finalize(stmt);
            
            return changes >= 0;
        } else {
            // Move bookmarks to default folder instead of deleting
            const char* sql = "UPDATE bookmarks SET folder = '', updated_at = ? WHERE user_id = ? AND folder = ?;";
            sqlite3_stmt* stmt;
            
            int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
            if(rc != SQLITE_OK) {
                throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
            }
            
            std::time_t now = std::time(nullptr);
            
            sqlite3_bind_int64(stmt, 1, now);
            sqlite3_bind_int(stmt, 2, user_id);
            sqlite3_bind_text(stmt, 3, folder_name.c_str(), -1, SQLITE_STATIC);
            
            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
            }
            
            int changes = sqlite3_changes(conn.get());
            sqlite3_finalize(stmt);
            
            return changes >= 0;
        }
    }
};

std::unique_ptr<BookmarkRepository> create_bookmark_repository(DatabasePool& db_pool) {
    return std::make_unique<SQLiteBookmarkRepository>(db_pool);
}

} // namespace repository
