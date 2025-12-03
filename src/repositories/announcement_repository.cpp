#include "repositories/announcement_repository.h"
#include "repositories/db_connection_pool.h"
#include "models/announcement.h"
#include <sqlite3.h>
#include <stdexcept>
#include <vector>
#include <sstream>

namespace repositories {

class SQLiteAnnouncementRepository : public AnnouncementRepository {
public:
    std::optional<models::Announcement> find_by_id(int id) const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "SELECT id, title, content, category, mandatory, publisher_id, publish_time, expire_time, created_at, updated_at, status FROM announcements WHERE id = ?";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_int(stmt, 1, id);
        
        std::optional<models::Announcement> announcement;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            announcement = parse_announcement(stmt);
        }
        
        sqlite3_finalize(stmt);
        return announcement;
    }
    
    std::vector<models::Announcement> find_by_publisher_id(int publisher_id) const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "SELECT id, title, content, category, mandatory, publisher_id, publish_time, expire_time, created_at, updated_at, status FROM announcements WHERE publisher_id = ? ORDER BY publish_time DESC";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_int(stmt, 1, publisher_id);
        
        std::vector<models::Announcement> announcements;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            announcements.push_back(parse_announcement(stmt));
        }
        
        sqlite3_finalize(stmt);
        return announcements;
    }
    
    std::vector<models::Announcement> find_with_filter(const AnnouncementFilter& filter, 
                                                       int page, int page_size,
                                                       bool order_by_publish_time_desc) const override {
        std::ostringstream sql;
        sql << "SELECT id, title, content, category, mandatory, publisher_id, publish_time, expire_time, created_at, updated_at, status FROM announcements WHERE 1=1";
        
        if (filter.category) {
            sql << " AND category = ?";
        }
        if (filter.mandatory) {
            sql << " AND mandatory = ?";
        }
        if (filter.status) {
            sql << " AND status = ?";
        }
        if (filter.start_time) {
            sql << " AND publish_time >= ?";
        }
        if (filter.end_time) {
            sql << " AND publish_time <= ?";
        }
        
        if (order_by_publish_time_desc) {
            sql << " ORDER BY publish_time DESC";
        } else {
            sql << " ORDER BY publish_time ASC";
        }
        
        sql << " LIMIT ? OFFSET ?";
        
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(conn, sql.str().c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        int param_index = 1;
        if (filter.category) {
            sqlite3_bind_text(stmt, param_index++, filter.category->c_str(), -1, SQLITE_STATIC);
        }
        if (filter.mandatory) {
            sqlite3_bind_int(stmt, param_index++, *filter.mandatory ? 1 : 0);
        }
        if (filter.status) {
            sqlite3_bind_text(stmt, param_index++, models::Announcement::status_to_string(*filter.status).c_str(), -1, SQLITE_STATIC);
        }
        if (filter.start_time) {
            sqlite3_bind_int64(stmt, param_index++, *filter.start_time);
        }
        if (filter.end_time) {
            sqlite3_bind_int64(stmt, param_index++, *filter.end_time);
        }
        
        int offset = (page - 1) * page_size;
        sqlite3_bind_int(stmt, param_index++, page_size);
        sqlite3_bind_int(stmt, param_index++, offset);
        
        std::vector<models::Announcement> announcements;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            announcements.push_back(parse_announcement(stmt));
        }
        
        sqlite3_finalize(stmt);
        return announcements;
    }
    
    int count_with_filter(const AnnouncementFilter& filter) const override {
        std::ostringstream sql;
        sql << "SELECT COUNT(*) FROM announcements WHERE 1=1";
        
        if (filter.category) {
            sql << " AND category = ?";
        }
        if (filter.mandatory) {
            sql << " AND mandatory = ?";
        }
        if (filter.status) {
            sql << " AND status = ?";
        }
        if (filter.start_time) {
            sql << " AND publish_time >= ?";
        }
        if (filter.end_time) {
            sql << " AND publish_time <= ?";
        }
        
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(conn, sql.str().c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        int param_index = 1;
        if (filter.category) {
            sqlite3_bind_text(stmt, param_index++, filter.category->c_str(), -1, SQLITE_STATIC);
        }
        if (filter.mandatory) {
            sqlite3_bind_int(stmt, param_index++, *filter.mandatory ? 1 : 0);
        }
        if (filter.status) {
            sqlite3_bind_text(stmt, param_index++, models::Announcement::status_to_string(*filter.status).c_str(), -1, SQLITE_STATIC);
        }
        if (filter.start_time) {
            sqlite3_bind_int64(stmt, param_index++, *filter.start_time);
        }
        if (filter.end_time) {
            sqlite3_bind_int64(stmt, param_index++, *filter.end_time);
        }
        
        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        
        sqlite3_finalize(stmt);
        return count;
    }
    
    int create(const models::Announcement& announcement) override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO announcements (title, content, category, mandatory, publisher_id, publish_time, expire_time, created_at, updated_at, status) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_text(stmt, 1, announcement.get_title().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, announcement.get_content().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, announcement.get_category().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, announcement.is_mandatory() ? 1 : 0);
        sqlite3_bind_int(stmt, 5, announcement.get_publisher_id());
        sqlite3_bind_int64(stmt, 6, announcement.get_publish_time());
        
        if (announcement.get_expire_time()) {
            sqlite3_bind_int64(stmt, 7, *announcement.get_expire_time());
        } else {
            sqlite3_bind_null(stmt, 7);
        }
        
        sqlite3_bind_int64(stmt, 8, announcement.get_created_at());
        sqlite3_bind_int64(stmt, 9, announcement.get_updated_at());
        sqlite3_bind_text(stmt, 10, models::Announcement::status_to_string(announcement.get_status()).c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error(std::string("Failed to execute statement: ") + sqlite3_errmsg(conn));
        }
        
        int id = sqlite3_last_insert_rowid(conn);
        sqlite3_finalize(stmt);
        return id;
    }
    
    bool update(const models::Announcement& announcement) override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "UPDATE announcements SET title = ?, content = ?, category = ?, mandatory = ?, publisher_id = ?, publish_time = ?, expire_time = ?, updated_at = ?, status = ? WHERE id = ?";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_text(stmt, 1, announcement.get_title().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, announcement.get_content().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, announcement.get_category().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, announcement.is_mandatory() ? 1 : 0);
        sqlite3_bind_int(stmt, 5, announcement.get_publisher_id());
        sqlite3_bind_int64(stmt, 6, announcement.get_publish_time());
        
        if (announcement.get_expire_time()) {
            sqlite3_bind_int64(stmt, 7, *announcement.get_expire_time());
        } else {
            sqlite3_bind_null(stmt, 7);
        }
        
        sqlite3_bind_int64(stmt, 8, announcement.get_updated_at());
        sqlite3_bind_text(stmt, 9, models::Announcement::status_to_string(announcement.get_status()).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 10, announcement.get_id());
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        return rc == SQLITE_DONE && sqlite3_changes(conn) > 0;
    }
    
    bool delete_by_id(int id) override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "UPDATE announcements SET status = 'deleted' WHERE id = ?";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_int(stmt, 1, id);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        return rc == SQLITE_DONE && sqlite3_changes(conn) > 0;
    }
    
    std::vector<models::Announcement> find_unread_by_user_id(int user_id, int page, int page_size) const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = R"(
            SELECT a.id, a.title, a.content, a.category, a.mandatory, a.publisher_id, 
                   a.publish_time, a.expire_time, a.created_at, a.updated_at, a.status 
            FROM announcements a 
            WHERE a.status = 'normal' 
              AND NOT EXISTS (
                  SELECT 1 FROM read_receipts r 
                  WHERE r.announcement_id = a.id AND r.user_id = ?
              ) 
            ORDER BY a.publish_time DESC 
            LIMIT ? OFFSET ?
        )";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        int offset = (page - 1) * page_size;
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_int(stmt, 2, page_size);
        sqlite3_bind_int(stmt, 3, offset);
        
        std::vector<models::Announcement> announcements;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            announcements.push_back(parse_announcement(stmt));
        }
        
        sqlite3_finalize(stmt);
        return announcements;
    }
    
    std::vector<models::Announcement> find_mandatory_by_user_id(int user_id, int page, int page_size) const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = R"(
            SELECT a.id, a.title, a.content, a.category, a.mandatory, a.publisher_id, 
                   a.publish_time, a.expire_time, a.created_at, a.updated_at, a.status 
            FROM announcements a 
            WHERE a.status = 'normal' AND a.mandatory = 1 
            ORDER BY a.publish_time DESC 
            LIMIT ? OFFSET ?
        )";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        int offset = (page - 1) * page_size;
        sqlite3_bind_int(stmt, 1, page_size);
        sqlite3_bind_int(stmt, 2, offset);
        
        std::vector<models::Announcement> announcements;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            announcements.push_back(parse_announcement(stmt));
        }
        
        sqlite3_finalize(stmt);
        return announcements;
    }
    
private:
    models::Announcement parse_announcement(sqlite3_stmt* stmt) const {
        int id = sqlite3_column_int(stmt, 0);
        const char* title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        bool mandatory = sqlite3_column_int(stmt, 4) != 0;
        int publisher_id = sqlite3_column_int(stmt, 5);
        std::time_t publish_time = sqlite3_column_int64(stmt, 6);
        
        std::optional<std::time_t> expire_time;
        if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) {
            expire_time = sqlite3_column_int64(stmt, 7);
        }
        
        std::time_t created_at = sqlite3_column_int64(stmt, 8);
        std::time_t updated_at = sqlite3_column_int64(stmt, 9);
        const char* status_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        
        models::Announcement::Status status = models::Announcement::status_from_string(status_str ? status_str : "normal");
        
        return models::Announcement(id, title ? title : "", content ? content : "", category ? category : "", 
                                   mandatory, publisher_id, publish_time, expire_time, created_at, updated_at, status);
    }
};

std::unique_ptr<AnnouncementRepository> create_announcement_repository() {
    return std::make_unique<SQLiteAnnouncementRepository>();
}

} // namespace repositories
