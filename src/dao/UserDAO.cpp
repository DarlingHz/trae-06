#include "dao/UserDAO.h"
#include <sstream>

namespace pet_hospital {

bool UserDAO::create_user(const User& user) {
    try {
        std::stringstream sql;
        sql << "INSERT INTO users (email, password_hash, name, phone) VALUES (" 
            << "'" << user.get_email() << "', "
            << "'" << user.get_password_hash() << "', "
            << "'" << user.get_name() << "', ";
        
        if (user.get_phone()) {
            sql << "'" << user.get_phone().value() << "');";
        } else {
            sql << "NULL);";
        }

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create user: " + std::string(e.what()));
        return false;
    }
}

std::optional<User> UserDAO::get_user_by_id(int user_id) {
    try {
        std::stringstream sql;
        sql << "SELECT id, email, password_hash, name, phone, created_at, updated_at FROM users WHERE id = " << user_id;

        std::vector<std::vector<std::string>> result;
        if (!database_->execute_query(sql.str(), result)) {
            return std::nullopt;
        }

        if (result.empty()) {
            return std::nullopt;
        }

        const auto& row = result[0];
        int id = std::stoi(row[0]);
        std::string email = row[1];
        std::string password_hash = row[2];
        std::string name = row[3];
        std::optional<std::string> phone = row[4].empty() ? std::nullopt : std::optional<std::string>(row[4]);
        std::string created_at = row[5];
        std::string updated_at = row[6];

        return User(id, email, password_hash, name, phone, created_at, updated_at);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get user by ID: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::optional<User> UserDAO::get_user_by_email(const std::string& email) {
    try {
        std::stringstream sql;
        sql << "SELECT id, email, password_hash, name, phone, created_at, updated_at FROM users WHERE email = '" << email << "'";

        std::vector<std::vector<std::string>> result;
        if (!database_->execute_query(sql.str(), result)) {
            return std::nullopt;
        }

        if (result.empty()) {
            return std::nullopt;
        }

        const auto& row = result[0];
        int id = std::stoi(row[0]);
        std::string user_email = row[1];
        std::string password_hash = row[2];
        std::string name = row[3];
        std::optional<std::string> phone = row[4].empty() ? std::nullopt : std::optional<std::string>(row[4]);
        std::string created_at = row[5];
        std::string updated_at = row[6];

        return User(id, user_email, password_hash, name, phone, created_at, updated_at);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get user by email: " + std::string(e.what()));
        return std::nullopt;
    }
}

bool UserDAO::update_user(const User& user) {
    try {
        std::stringstream sql;
        sql << "UPDATE users SET " 
            << "email = '" << user.get_email() << "', "
            << "password_hash = '" << user.get_password_hash() << "', "
            << "name = '" << user.get_name() << "', ";
        
        if (user.get_phone()) {
            sql << "phone = '" << user.get_phone().value() << "', ";
        } else {
            sql << "phone = NULL, ";
        }
        
        sql << "updated_at = CURRENT_TIMESTAMP WHERE id = " << user.get_id();

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update user: " + std::string(e.what()));
        return false;
    }
}

bool UserDAO::delete_user(int user_id) {
    try {
        std::stringstream sql;
        sql << "DELETE FROM users WHERE id = " << user_id;

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to delete user: " + std::string(e.what()));
        return false;
    }
}

} // namespace pet_hospital
