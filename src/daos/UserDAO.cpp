#include "daos/UserDAO.hpp"
#include <stdexcept>

namespace daos {

bool UserDAO::createUser(const models::User& user) {
    std::string sql = "INSERT INTO users (username, email, password_hash, created_at) VALUES (?, ?, ?, ?)";
    std::vector<std::string> params = {
        user.getUsername(),
        user.getEmail(),
        user.getPasswordHash(),
        user.getCreatedAt()
    };

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create user: " + std::string(e.what()));
    }
}

models::User UserDAO::getUserById(int id) {
    std::string sql = "SELECT * FROM users WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};

    try {
        auto result = db_.fetchWithParams(sql, params);
        if (result.empty()) {
            return models::User();
        }
        return rowToUser(result[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get user by ID: " + std::string(e.what()));
    }
}

models::User UserDAO::getUserByUsername(const std::string& username) {
    std::string sql = "SELECT * FROM users WHERE username = ?";
    std::vector<std::string> params = {username};

    try {
        auto result = db_.fetchWithParams(sql, params);
        if (result.empty()) {
            return models::User();
        }
        return rowToUser(result[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get user by username: " + std::string(e.what()));
    }
}

models::User UserDAO::getUserByEmail(const std::string& email) {
    std::string sql = "SELECT * FROM users WHERE email = ?";
    std::vector<std::string> params = {email};

    try {
        auto result = db_.fetchWithParams(sql, params);
        if (result.empty()) {
            return models::User();
        }
        return rowToUser(result[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get user by email: " + std::string(e.what()));
    }
}

models::User UserDAO::getUserByUsernameOrEmail(const std::string& username_or_email) {
    std::string sql = "SELECT * FROM users WHERE username = ? OR email = ?";
    std::vector<std::string> params = {username_or_email, username_or_email};

    try {
        auto result = db_.fetchWithParams(sql, params);
        if (result.empty()) {
            return models::User();
        }
        return rowToUser(result[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get user by username or email: " + std::string(e.what()));
    }
}

bool UserDAO::updateUser(const models::User& user) {
    std::string sql = "UPDATE users SET username = ?, email = ?, password_hash = ? WHERE id = ?";
    std::vector<std::string> params = {
        user.getUsername(),
        user.getEmail(),
        user.getPasswordHash(),
        std::to_string(user.getId())
    };

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to update user: " + std::string(e.what()));
    }
}

bool UserDAO::deleteUser(int id) {
    std::string sql = "DELETE FROM users WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to delete user: " + std::string(e.what()));
    }
}

models::User UserDAO::rowToUser(const std::map<std::string, std::string>& row) {
    models::User user;

    auto it = row.find("id");
    if (it != row.end()) {
        user.setId(std::stoi(it->second));
    }

    it = row.find("username");
    if (it != row.end()) {
        user.setUsername(it->second);
    }

    it = row.find("email");
    if (it != row.end()) {
        user.setEmail(it->second);
    }

    it = row.find("password_hash");
    if (it != row.end()) {
        user.setPasswordHash(it->second);
    }

    it = row.find("created_at");
    if (it != row.end()) {
        user.setCreatedAt(it->second);
    }

    return user;
}

} // namespace daos
