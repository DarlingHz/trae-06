#include "parking/dao.h"
#include "parking/database.h"
#include "parking/models.h"
#include <stdexcept>
#include <string>

// SQLiteUserDAO实现
int SQLiteUserDAO::create(const User& user) {
    try {
        db_.execute(
            "INSERT INTO users (name, email, password_hash, status, created_at, updated_at) "
            "VALUES (" 
            "'" + user.name + "', "
            "'" + user.email + "', "
            "'" + user.password_hash + "', "
            "'" + to_string(user.status) + "', "
            + std::to_string(user.created_at) + ", "
            + std::to_string(user.updated_at) + ")"
        );
        return static_cast<int>(db_.last_insert_rowid());
    } catch (const std::exception& e) {
        // 检查是否是唯一约束冲突（邮箱重复）
        std::string err_msg = e.what();
        if (err_msg.find("UNIQUE constraint failed: users.email") != std::string::npos) {
            throw std::runtime_error("Email already exists");
        }
        throw;
    }
}

std::optional<User> SQLiteUserDAO::find_by_id(int id) const {
    User user;
    bool found = false;

    try {
        db_.query(
            "SELECT id, name, email, password_hash, status, created_at, updated_at "
            "FROM users WHERE id = " + std::to_string(id),
            [&](int argc, char** argv, char** azColName) -> int {
                if (argc >= 7) {
                    user.id = std::stoi(argv[0]);
                    user.name = argv[1];
                    user.email = argv[2];
                    user.password_hash = argv[3];
                    user.status = user_status_from_string(argv[4]);
                    user.created_at = std::stoll(argv[5]);
                    user.updated_at = std::stoll(argv[6]);
                    found = true;
                }
                return 0;
            }
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find user by id: " + std::string(e.what()));
    }

    return found ? std::optional<User>(user) : std::nullopt;
}

std::optional<User> SQLiteUserDAO::find_by_email(const std::string& email) const {
    User user;
    bool found = false;

    try {
        db::get().query(
            "SELECT id, name, email, password_hash, status, created_at, updated_at "
            "FROM users WHERE email = '" + email + "'",
            [&](int argc, char** argv, char** azColName) -> int {
                if (argc >= 7) {
                    user.id = std::stoi(argv[0]);
                    user.name = argv[1];
                    user.email = argv[2];
                    user.password_hash = argv[3];
                    user.status = user_status_from_string(argv[4]);
                    user.created_at = std::stoll(argv[5]);
                    user.updated_at = std::stoll(argv[6]);
                    found = true;
                }
                return 0;
            }
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find user by email: " + std::string(e.what()));
    }

    return found ? std::optional<User>(user) : std::nullopt;
}

bool SQLiteUserDAO::update(const User& user) {
    try {
        db::get().execute(
            "UPDATE users SET "
            "name = '" + user.name + "', "
            "email = '" + user.email + "', "
            "password_hash = '" + user.password_hash + "', "
            "status = '" + to_string(user.status) + "', "
            "updated_at = " + std::to_string(user.updated_at) + " "
            "WHERE id = " + std::to_string(user.id)
        );
        return sqlite3_changes(db_.get_native_handle()) > 0;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to update user: " + std::string(e.what()));
    }
}
