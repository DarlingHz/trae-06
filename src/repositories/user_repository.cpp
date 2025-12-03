#include "repositories/user_repository.h"
#include "repositories/db_connection_pool.h"
#include "models/user.h"
#include <sqlite3.h>
#include <stdexcept>
#include <vector>

namespace repositories {

class SQLiteUserRepository : public UserRepository {
public:
    std::optional<models::User> find_by_id(int id) const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "SELECT id, name, email, department, role, password_hash, created_at, updated_at, status FROM users WHERE id = ?";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_int(stmt, 1, id);
        
        std::optional<models::User> user;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            user = parse_user(stmt);
        }
        
        sqlite3_finalize(stmt);
        return user;
    }
    
    std::optional<models::User> find_by_email(const std::string& email) const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "SELECT id, name, email, department, role, password_hash, created_at, updated_at, status FROM users WHERE email = ?";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
        
        std::optional<models::User> user;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            user = parse_user(stmt);
        }
        
        sqlite3_finalize(stmt);
        return user;
    }
    
    std::vector<models::User> find_by_department(const std::string& department) const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "SELECT id, name, email, department, role, password_hash, created_at, updated_at, status FROM users WHERE department = ?";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_text(stmt, 1, department.c_str(), -1, SQLITE_STATIC);
        
        std::vector<models::User> users;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            users.push_back(parse_user(stmt));
        }
        
        sqlite3_finalize(stmt);
        return users;
    }
    
    int create(const models::User& user) override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO users (name, email, department, role, password_hash, created_at, updated_at, status) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_text(stmt, 1, user.get_name().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, user.get_email().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, user.get_department().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, models::User::role_to_string(user.get_role()).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, user.get_password_hash().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 6, user.get_created_at());
        sqlite3_bind_int64(stmt, 7, user.get_updated_at());
        sqlite3_bind_text(stmt, 8, models::User::status_to_string(user.get_status()).c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error(std::string("Failed to execute statement: ") + sqlite3_errmsg(conn));
        }
        
        int id = sqlite3_last_insert_rowid(conn);
        sqlite3_finalize(stmt);
        return id;
    }
    
    bool update(const models::User& user) override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "UPDATE users SET name = ?, email = ?, department = ?, role = ?, password_hash = ?, updated_at = ?, status = ? WHERE id = ?";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_text(stmt, 1, user.get_name().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, user.get_email().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, user.get_department().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, models::User::role_to_string(user.get_role()).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, user.get_password_hash().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 6, user.get_updated_at());
        sqlite3_bind_text(stmt, 7, models::User::status_to_string(user.get_status()).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 8, user.get_id());
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        return rc == SQLITE_DONE && sqlite3_changes(conn) > 0;
    }
    
    bool delete_by_id(int id) override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "DELETE FROM users WHERE id = ?";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        sqlite3_bind_int(stmt, 1, id);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        return rc == SQLITE_DONE && sqlite3_changes(conn) > 0;
    }
    
    std::vector<models::User> find_all(int page, int page_size) const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "SELECT id, name, email, department, role, password_hash, created_at, updated_at, status FROM users ORDER BY id LIMIT ? OFFSET ?";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        int offset = (page - 1) * page_size;
        sqlite3_bind_int(stmt, 1, page_size);
        sqlite3_bind_int(stmt, 2, offset);
        
        std::vector<models::User> users;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            users.push_back(parse_user(stmt));
        }
        
        sqlite3_finalize(stmt);
        return users;
    }
    
    int count_all() const override {
        auto conn = DBConnectionPool::instance().acquire_connection();
        sqlite3_stmt* stmt;
        const char* sql = "SELECT COUNT(*) FROM users";
        
        int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + sqlite3_errmsg(conn));
        }
        
        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        
        sqlite3_finalize(stmt);
        return count;
    }
    
private:
    models::User parse_user(sqlite3_stmt* stmt) const {
        int id = sqlite3_column_int(stmt, 0);
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* department = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* role_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        const char* password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        std::time_t created_at = sqlite3_column_int64(stmt, 6);
        std::time_t updated_at = sqlite3_column_int64(stmt, 7);
        const char* status_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        
        models::User::Role role = models::User::role_from_string(role_str ? role_str : "employee");
        models::User::Status status = models::User::status_from_string(status_str ? status_str : "active");
        
        return models::User(id, name ? name : "", email ? email : "", department ? department : "", 
                           role, password_hash ? password_hash : "", created_at, updated_at, status);
    }
};

std::unique_ptr<UserRepository> create_user_repository() {
    return std::make_unique<SQLiteUserRepository>();
}

} // namespace repositories
