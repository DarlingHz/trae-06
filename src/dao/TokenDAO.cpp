#include "dao/TokenDAO.h"
#include <sstream>

namespace pet_hospital {

bool TokenDAO::create_token(const Token& token) {
    try {
        std::stringstream sql;
        sql << "INSERT INTO tokens (user_id, token, expires_at) VALUES (" 
            << token.get_user_id() << ", "
            << "'" << token.get_token() << "', "
            << "'" << token.get_expires_at() << "');";

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create token: " + std::string(e.what()));
        return false;
    }
}

std::optional<Token> TokenDAO::get_token_by_value(const std::string& token_value) {
    try {
        std::stringstream sql;
        sql << "SELECT id, user_id, token, expires_at, created_at FROM tokens WHERE token = '" << token_value << "'";

        std::vector<std::vector<std::string>> result;
        if (!database_->execute_query(sql.str(), result)) {
            return std::nullopt;
        }

        if (result.empty()) {
            return std::nullopt;
        }

        const auto& row = result[0];
        int id = std::stoi(row[0]);
        int user_id = std::stoi(row[1]);
        std::string token = row[2];
        std::string expires_at = row[3];
        std::string created_at = row[4];

        return Token(id, user_id, token, expires_at, created_at);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get token by value: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::optional<Token> TokenDAO::get_token_by_user_id(int user_id) {
    try {
        std::stringstream sql;
        sql << "SELECT id, user_id, token, expires_at, created_at FROM tokens WHERE user_id = " << user_id;

        std::vector<std::vector<std::string>> result;
        if (!database_->execute_query(sql.str(), result)) {
            return std::nullopt;
        }

        if (result.empty()) {
            return std::nullopt;
        }

        const auto& row = result[0];
        int id = std::stoi(row[0]);
        int user_id_val = std::stoi(row[1]);
        std::string token = row[2];
        std::string expires_at = row[3];
        std::string created_at = row[4];

        return Token(id, user_id_val, token, expires_at, created_at);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get token by user ID: " + std::string(e.what()));
        return std::nullopt;
    }
}

bool TokenDAO::update_token(const Token& token) {
    try {
        std::stringstream sql;
        sql << "UPDATE tokens SET " 
            << "user_id = " << token.get_user_id() << ", "
            << "token = '" << token.get_token() << "', "
            << "expires_at = '" << token.get_expires_at() << "' "
            << "WHERE id = " << token.get_id() << ";";

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update token: " + std::string(e.what()));
        return false;
    }
}

bool TokenDAO::delete_token(int token_id) {
    try {
        std::stringstream sql;
        sql << "DELETE FROM tokens WHERE id = " << token_id << ";";

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to delete token: " + std::string(e.what()));
        return false;
    }
}

bool TokenDAO::delete_token_by_value(const std::string& token_value) {
    try {
        std::stringstream sql;
        sql << "DELETE FROM tokens WHERE token = '" << token_value << "';";

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to delete token by value: " + std::string(e.what()));
        return false;
    }
}

} // namespace pet_hospital