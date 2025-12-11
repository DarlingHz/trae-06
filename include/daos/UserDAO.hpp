#pragma once

#include "utils/Database.hpp"
#include "models/User.hpp"
#include <vector>

namespace daos {

class UserDAO {
public:
    // Constructor
    explicit UserDAO(utils::Database& db) : db_(db) {}

    // Destructor
    ~UserDAO() = default;

    // Create a new user
    bool createUser(const models::User& user);

    // Get a user by ID
    models::User getUserById(int id);

    // Get a user by username
    models::User getUserByUsername(const std::string& username);

    // Get a user by email
    models::User getUserByEmail(const std::string& email);

    // Get a user by username or email
    models::User getUserByUsernameOrEmail(const std::string& username_or_email);

    // Update a user's information
    bool updateUser(const models::User& user);

    // Delete a user by ID
    bool deleteUser(int id);

private:
    utils::Database& db_;

    // Convert a database row to a User object
    models::User rowToUser(const std::map<std::string, std::string>& row);
};

} // namespace daos
