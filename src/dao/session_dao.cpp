#include "parking/dao.h"
#include "parking/database.h"
#include "parking/models.h"
#include <stdexcept>
#include <string>

// SQLiteSessionDAO实现
void SQLiteSessionDAO::create(const Session& session) {
    try {
        db::get().execute(
            "INSERT INTO sessions (token, user_id, expires_at, created_at) "
            "VALUES (" 
            "'" + session.token + "', "
            + std::to_string(session.user_id) + ", "
            + std::to_string(session.expires_at) + ", "
            + std::to_string(session.created_at) + ")"
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create session: " + std::string(e.what()));
    }
}

std::optional<Session> SQLiteSessionDAO::find_by_token(const std::string& token) const {
    Session session;
    bool found = false;

    try {
        db::get().query(
            "SELECT token, user_id, expires_at, created_at "
            "FROM sessions WHERE token = '" + token + "'",
            [&](int argc, char** argv, char** azColName) -> int {
                if (argc >= 4) {
                    session.token = argv[0];
                    session.user_id = std::stoi(argv[1]);
                    session.expires_at = std::stoll(argv[2]);
                    session.created_at = std::stoll(argv[3]);
                    found = true;
                }
                return 0;
            }
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find session by token: " + std::string(e.what()));
    }

    return found ? std::optional<Session>(session) : std::nullopt;
}

void SQLiteSessionDAO::cleanup_expired() {
    try {
        db::get().execute(
            "DELETE FROM sessions WHERE expires_at < " + std::to_string(std::time(nullptr))
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to cleanup expired sessions: " + std::string(e.what()));
    }
}

void SQLiteSessionDAO::delete_by_token(const std::string& token) {
    try {
        db::get().execute(
            "DELETE FROM sessions WHERE token = '" + token + "'"
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to delete session: " + std::string(e.what()));
    }
}
