#include "user_service.h"
#include "database.h"
#include "auth.h"
#include "logger.h"
#include <stdexcept>

std::optional<UserDTO> UserService::register_user(const RegisterRequest& request) {
    if (is_email_exists(request.email)) {
        return std::nullopt;
    }
    
    std::string hashed_pwd = AuthManager::instance().hash_password(request.password);
    
    std::string sql = "INSERT INTO users (name, email, password_hash, phone) VALUES ('";
    sql += request.name + "','" + request.email + "','" + hashed_pwd + "','";
    if (request.phone.has_value()) {
        sql += request.phone.value();
    }
    sql += "');";
    
    int user_id = 0;
    if (!Database::instance().execute_update(sql, &user_id)) {
        return std::nullopt;
    }
    
    return get_user_by_id(user_id);
}

std::optional<UserDTO> UserService::login(const LoginRequest& request) {
    auto user = get_user_by_email(request.email);
    if (!user.has_value()) {
        return std::nullopt;
    }
    
    if (!AuthManager::instance().verify_password(request.password, user->password_hash)) {
        return std::nullopt;
    }
    
    return user;
}

std::optional<UserDTO> UserService::get_user_by_id(int user_id) {
    std::string sql = "SELECT id, name, email, phone, role, created_at FROM users WHERE id = " + std::to_string(user_id) + ";";
    
    UserDTO user;
    bool found = false;
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        user.id = sqlite3_column_int(stmt, 0);
        user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        
        const char* phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        if (phone != nullptr) {
            user.phone = std::string(phone);
        }
        
        user.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        user.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        
        found = true;
        return 1; // 只取第一条
    });
    
    return found ? std::optional<UserDTO>(user) : std::nullopt;
}

std::optional<UserDTO> UserService::get_user_by_email(const std::string& email) {
    std::string sql = "SELECT id, name, email, phone, role, created_at FROM users WHERE email = '" + email + "';";
    
    UserDTO user;
    bool found = false;
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        user.id = sqlite3_column_int(stmt, 0);
        user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        
        const char* phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        if (phone != nullptr) {
            user.phone = std::string(phone);
        }
        
        user.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        user.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        
        found = true;
        return 1; // 只取第一条
    });
    
    return found ? std::optional<UserDTO>(user) : std::nullopt;
}

bool UserService::is_email_exists(const std::string& email) {
    std::string sql = "SELECT COUNT(*) FROM users WHERE email = '" + email + "';";
    int count = 0;
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        count = sqlite3_column_int(stmt, 0);
        return 1;
    });
    
    return count > 0;
}