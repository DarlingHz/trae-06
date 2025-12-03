#include "repository/UserRepository.h"
#include "repository/DatabasePool.h"
#include <memory>
#include <ctime>
#include <stdexcept>

namespace repository {

class SQLiteUserRepository : public UserRepository {
private:
    DatabasePool& db_pool_;

    std::shared_ptr<models::User> user_from_row(sqlite3_stmt* stmt) {
        int id = sqlite3_column_int(stmt, 0);
        const char* email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* nickname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        std::time_t created_at = sqlite3_column_int64(stmt, 4);
        std::time_t updated_at = sqlite3_column_int64(stmt, 5);
        
        return std::make_shared<models::User>(id, email ? email : "", 
                                             password_hash ? password_hash : "", 
                                             nickname ? nickname : "", 
                                             created_at, updated_at);
    }

public:
    SQLiteUserRepository(DatabasePool& db_pool) : db_pool_(db_pool) {}

    std::shared_ptr<models::User> create(const std::string& email, 
                                         const std::string& password_hash, 
                                         const std::string& nickname) override {
        auto conn = db_pool_.get_connection();
        
        const char* sql = "INSERT INTO users (email, password_hash, nickname, created_at, updated_at) VALUES (?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        std::time_t now = std::time(nullptr);
        
        sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password_hash.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, nickname.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 4, now);
        sqlite3_bind_int64(stmt, 5, now);
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        int user_id = sqlite3_last_insert_rowid(conn.get());
        sqlite3_finalize(stmt);
        
        return std::make_shared<models::User>(user_id, email, password_hash, nickname, now, now);
    }

    std::shared_ptr<models::User> find_by_id(int id) override {
        auto conn = db_pool_.get_connection();
        
        const char* sql = "SELECT id, email, password_hash, nickname, created_at, updated_at FROM users WHERE id = ?;";
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
        
        auto user = user_from_row(stmt);
        sqlite3_finalize(stmt);
        
        return user;
    }

    std::shared_ptr<models::User> find_by_email(const std::string& email) override {
        auto conn = db_pool_.get_connection();
        
        const char* sql = "SELECT id, email, password_hash, nickname, created_at, updated_at FROM users WHERE email = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_ROW) {
            sqlite3_finalize(stmt);
            return nullptr;
        }
        
        auto user = user_from_row(stmt);
        sqlite3_finalize(stmt);
        
        return user;
    }

    bool update(std::shared_ptr<models::User> user) override {
        if(!user) return false;
        
        auto conn = db_pool_.get_connection();
        
        const char* sql = "UPDATE users SET email = ?, password_hash = ?, nickname = ?, updated_at = ? WHERE id = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(conn.get(), sql, -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        std::time_t now = std::time(nullptr);
        
        sqlite3_bind_text(stmt, 1, user->email().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, user->password_hash().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, user->nickname().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 4, now);
        sqlite3_bind_int(stmt, 5, user->id());
        
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(conn.get())));
        }
        
        int changes = sqlite3_changes(conn.get());
        sqlite3_finalize(stmt);
        
        if(changes > 0) {
            user->set_updated_at(now);
        }
        
        return changes > 0;
    }
};

std::unique_ptr<UserRepository> create_user_repository(DatabasePool& db_pool) {
    return std::make_unique<SQLiteUserRepository>(db_pool);
}

} // namespace repository
