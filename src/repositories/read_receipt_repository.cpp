#include "repositories/read_receipt_repository.h"
#include "repositories/db_connection_pool.h"
#include "models/read_receipt.h"
#include "models/user.h"
#include "models/announcement.h"
#include <sqlite3.h>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <algorithm>

namespace repositories {

class SQLiteReadReceiptRepository : public ReadReceiptRepository {
public:
    std::optional<models::ReadReceipt> find_by_id(int id) const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "SELECT id, announcement_id, user_id, read_at, client_ip, user_agent, extra_metadata FROM read_receipts WHERE id = ?";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_int(stmt, 1, id);
        
        std::optional<models::ReadReceipt> receipt;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            receipt = parse_read_receipt(stmt);
        }
        
        sqlite3_finalize(stmt);
        return receipt;
    }
    
    std::optional<models::ReadReceipt> find_by_announcement_and_user(int announcement_id, int user_id) const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "SELECT id, announcement_id, user_id, read_at, client_ip, user_agent, extra_metadata FROM read_receipts WHERE announcement_id = ? AND user_id = ?";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_int(stmt, 1, announcement_id);
        sqlite3_bind_int(stmt, 2, user_id);
        
        std::optional<models::ReadReceipt> receipt;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            receipt = parse_read_receipt(stmt);
        }
        
        sqlite3_finalize(stmt);
        return receipt;
    }
    
    std::vector<models::ReadReceipt> find_with_filter(const ReadReceiptFilter& filter) const override {
        std::ostringstream sql;
        sql << "SELECT id, announcement_id, user_id, read_at, client_ip, user_agent, extra_metadata FROM read_receipts WHERE 1=1";
        
        if (filter.announcement_id) {
            sql << " AND announcement_id = ?";
        }
        if (filter.user_id) {
            sql << " AND user_id = ?";
        }
        if (filter.start_time) {
            sql << " AND read_at >= ?";
        }
        if (filter.end_time) {
            sql << " AND read_at <= ?";
        }
        
        sql << " ORDER BY read_at DESC";
        
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(conn, sql.str().c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        int param_index = 1;
        if (filter.announcement_id) {
            sqlite3_bind_int(stmt, param_index++, *filter.announcement_id);
        }
        if (filter.user_id) {
            sqlite3_bind_int(stmt, param_index++, *filter.user_id);
        }
        if (filter.start_time) {
            sqlite3_bind_int64(stmt, param_index++, *filter.start_time);
        }
        if (filter.end_time) {
            sqlite3_bind_int64(stmt, param_index++, *filter.end_time);
        }
        
        std::vector<models::ReadReceipt> receipts;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            receipts.push_back(parse_read_receipt(stmt));
        }
        
        sqlite3_finalize(stmt);
        return receipts;
    }
    
    std::vector<models::ReadReceipt> find_read_users_by_announcement(int announcement_id) const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = R"(
            SELECT r.id, r.announcement_id, r.user_id, r.read_at, r.client_ip, r.user_agent, r.extra_metadata 
            FROM read_receipts r 
            WHERE r.announcement_id = ? 
            ORDER BY r.read_at DESC
        )";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_int(stmt, 1, announcement_id);
        
        std::vector<models::ReadReceipt> receipts;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            receipts.push_back(parse_read_receipt(stmt));
        }
        
        sqlite3_finalize(stmt);
        return receipts;
    }
    
    int create(const models::ReadReceipt& receipt) override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO read_receipts (announcement_id, user_id, read_at, client_ip, user_agent, extra_metadata) VALUES (?, ?, ?, ?, ?, ?)";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_int(stmt, 1, receipt.get_announcement_id());
        sqlite3_bind_int(stmt, 2, receipt.get_user_id());
        sqlite3_bind_int64(stmt, 3, receipt.get_read_at());
        
        if (receipt.get_client_ip()) {
            sqlite3_bind_text(stmt, 4, receipt.get_client_ip()->c_str(), -1, SQLITE_STATIC);
        } else {
            sqlite3_bind_null(stmt, 4);
        }
        
        if (receipt.get_user_agent()) {
            sqlite3_bind_text(stmt, 5, receipt.get_user_agent()->c_str(), -1, SQLITE_STATIC);
        } else {
            sqlite3_bind_null(stmt, 5);
        }
        
        if (receipt.get_extra_metadata()) {
            sqlite3_bind_text(stmt, 6, receipt.get_extra_metadata()->c_str(), -1, SQLITE_STATIC);
        } else {
            sqlite3_bind_null(stmt, 6);
        }
        
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error(std::string("Failed to execute statement: ") + sqlite3_errmsg(conn));
        }
        
        int id = sqlite3_last_insert_rowid(conn);
        sqlite3_finalize(stmt);
        return id;
    }
    
    bool update(const models::ReadReceipt& receipt) override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "UPDATE read_receipts SET read_at = ?, client_ip = ?, user_agent = ?, extra_metadata = ? WHERE id = ?";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_int64(stmt, 1, receipt.get_read_at());
        
        if (receipt.get_client_ip()) {
            sqlite3_bind_text(stmt, 2, receipt.get_client_ip()->c_str(), -1, SQLITE_STATIC);
        } else {
            sqlite3_bind_null(stmt, 2);
        }
        
        if (receipt.get_user_agent()) {
            sqlite3_bind_text(stmt, 3, receipt.get_user_agent()->c_str(), -1, SQLITE_STATIC);
        } else {
            sqlite3_bind_null(stmt, 3);
        }
        
        if (receipt.get_extra_metadata()) {
            sqlite3_bind_text(stmt, 4, receipt.get_extra_metadata()->c_str(), -1, SQLITE_STATIC);
        } else {
            sqlite3_bind_null(stmt, 4);
        }
        
        sqlite3_bind_int(stmt, 5, receipt.get_id());
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        return rc == SQLITE_DONE && sqlite3_changes(conn) > 0;
    }
    
    bool delete_by_id(int id) override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "DELETE FROM read_receipts WHERE id = ?";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_int(stmt, 1, id);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        return rc == SQLITE_DONE && sqlite3_changes(conn) > 0;
    }
    
    AnnouncementStats get_announcement_stats(int announcement_id) const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        
        // 计算总用户数（活跃用户）
        sqlite3_stmt* stmt_users;
        const char* sql_users = "SELECT COUNT(*) FROM users WHERE status = 'active'";
        int rc = sqlite3_prepare_v2(conn, sql_users, -1, &stmt_users, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        int total_users = 0;
        if (sqlite3_step(stmt_users) == SQLITE_ROW) {
            total_users = sqlite3_column_int(stmt_users, 0);
        }
        sqlite3_finalize(stmt_users);
        
        // 计算已读人数
        sqlite3_stmt* stmt_read;
        const char* sql_read = "SELECT COUNT(DISTINCT user_id) FROM read_receipts WHERE announcement_id = ?";
        rc = sqlite3_prepare_v2(conn, sql_read, -1, &stmt_read, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_int(stmt_read, 1, announcement_id);
        int read_count = 0;
        if (sqlite3_step(stmt_read) == SQLITE_ROW) {
            read_count = sqlite3_column_int(stmt_read, 0);
        }
        sqlite3_finalize(stmt_read);
        
        AnnouncementStats stats;
        stats.total_users = total_users;
        stats.read_count = read_count;
        stats.unread_count = total_users - read_count;
        
        return stats;
    }
    
    UserReadingStats get_user_reading_stats(int user_id, const std::optional<std::time_t>& start_time,
                                            const std::optional<std::time_t>& end_time) const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        
        std::ostringstream sql_total;
        sql_total << "SELECT COUNT(*) FROM announcements WHERE status = 'normal'";
        if (start_time) sql_total << " AND publish_time >= ?";
        if (end_time) sql_total << " AND publish_time <= ?";
        
        sqlite3_stmt* stmt_total;
        int rc = sqlite3_prepare_v2(conn, sql_total.str().c_str(), -1, &stmt_total, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        int param_index = 1;
        if (start_time) sqlite3_bind_int64(stmt_total, param_index++, *start_time);
        if (end_time) sqlite3_bind_int64(stmt_total, param_index++, *end_time);
        
        int total_announcements = 0;
        if (sqlite3_step(stmt_total) == SQLITE_ROW) {
            total_announcements = sqlite3_column_int(stmt_total, 0);
        }
        sqlite3_finalize(stmt_total);
        
        std::ostringstream sql_read;
        sql_read << "SELECT COUNT(*) FROM read_receipts r JOIN announcements a ON r.announcement_id = a.id WHERE r.user_id = ? AND a.status = 'normal'";
        if (start_time) sql_read << " AND a.publish_time >= ?";
        if (end_time) sql_read << " AND a.publish_time <= ?";
        
        sqlite3_stmt* stmt_read;
        rc = sqlite3_prepare_v2(conn, sql_read.str().c_str(), -1, &stmt_read, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        param_index = 1;
        sqlite3_bind_int(stmt_read, param_index++, user_id);
        if (start_time) sqlite3_bind_int64(stmt_read, param_index++, *start_time);
        if (end_time) sqlite3_bind_int64(stmt_read, param_index++, *end_time);
        
        int read_count = 0;
        if (sqlite3_step(stmt_read) == SQLITE_ROW) {
            read_count = sqlite3_column_int(stmt_read, 0);
        }
        sqlite3_finalize(stmt_read);
        
        std::ostringstream sql_mandatory_total;
        sql_mandatory_total << "SELECT COUNT(*) FROM announcements WHERE status = 'normal' AND mandatory = 1";
        if (start_time) sql_mandatory_total << " AND publish_time >= ?";
        if (end_time) sql_mandatory_total << " AND publish_time <= ?";
        
        sqlite3_stmt* stmt_mandatory_total;
        rc = sqlite3_prepare_v2(conn, sql_mandatory_total.str().c_str(), -1, &stmt_mandatory_total, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        param_index = 1;
        if (start_time) sqlite3_bind_int64(stmt_mandatory_total, param_index++, *start_time);
        if (end_time) sqlite3_bind_int64(stmt_mandatory_total, param_index++, *end_time);
        
        int mandatory_total = 0;
        if (sqlite3_step(stmt_mandatory_total) == SQLITE_ROW) {
            mandatory_total = sqlite3_column_int(stmt_mandatory_total, 0);
        }
        sqlite3_finalize(stmt_mandatory_total);
        
        std::ostringstream sql_mandatory_read;
        sql_mandatory_read << "SELECT COUNT(*) FROM read_receipts r JOIN announcements a ON r.announcement_id = a.id WHERE r.user_id = ? AND a.status = 'normal' AND a.mandatory = 1";
        if (start_time) sql_mandatory_read << " AND a.publish_time >= ?";
        if (end_time) sql_mandatory_read << " AND a.publish_time <= ?";
        
        sqlite3_stmt* stmt_mandatory_read;
        rc = sqlite3_prepare_v2(conn, sql_mandatory_read.str().c_str(), -1, &stmt_mandatory_read, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        param_index = 1;
        sqlite3_bind_int(stmt_mandatory_read, param_index++, user_id);
        if (start_time) sqlite3_bind_int64(stmt_mandatory_read, param_index++, *start_time);
        if (end_time) sqlite3_bind_int64(stmt_mandatory_read, param_index++, *end_time);
        
        int mandatory_read = 0;
        if (sqlite3_step(stmt_mandatory_read) == SQLITE_ROW) {
            mandatory_read = sqlite3_column_int(stmt_mandatory_read, 0);
        }
        sqlite3_finalize(stmt_mandatory_read);
        
        UserReadingStats stats;
        stats.total_announcements = total_announcements;
        stats.read_count = read_count;
        stats.unread_count = total_announcements - read_count;
        stats.mandatory_total = mandatory_total;
        stats.mandatory_read = mandatory_read;
        stats.mandatory_completion_rate = mandatory_total > 0 ? static_cast<double>(mandatory_read) / mandatory_total : 1.0;
        
        return stats;
    }
    
    bool has_read(int user_id, int announcement_id) const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "SELECT 1 FROM read_receipts WHERE announcement_id = ? AND user_id = ? LIMIT 1";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_int(stmt, 1, announcement_id);
        sqlite3_bind_int(stmt, 2, user_id);
        
        bool has_read = sqlite3_step(stmt) == SQLITE_ROW;
        sqlite3_finalize(stmt);
        return has_read;
    }
    
    std::vector<int> find_read_announcement_ids(int user_id) const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "SELECT announcement_id FROM read_receipts WHERE user_id = ?";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_int(stmt, 1, user_id);
        
        std::vector<int> announcement_ids;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            announcement_ids.push_back(sqlite3_column_int(stmt, 0));
        }
        
        sqlite3_finalize(stmt);
        return announcement_ids;
    }
    
private:
    models::ReadReceipt parse_read_receipt(sqlite3_stmt* stmt) const {
        int id = sqlite3_column_int(stmt, 0);
        int announcement_id = sqlite3_column_int(stmt, 1);
        int user_id = sqlite3_column_int(stmt, 2);
        std::time_t read_at = sqlite3_column_int64(stmt, 3);
        
        std::optional<std::string> client_ip;
        if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
            client_ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        }
        
        std::optional<std::string> user_agent;
        if (sqlite3_column_type(stmt, 5) != SQLITE_NULL) {
            user_agent = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        }
        
        std::optional<std::string> extra_metadata;
        if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
            extra_metadata = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        }
        
        return models::ReadReceipt(id, announcement_id, user_id, read_at, client_ip, user_agent, extra_metadata);
    }
};

std::unique_ptr<ReadReceiptRepository> create_read_receipt_repository() {
    return std::make_unique<SQLiteReadReceiptRepository>();
}

} // namespace repositories
