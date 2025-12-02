#pragma once

#include "utils/Database.hpp"
#include "models/Tag.hpp"
#include <vector>

namespace daos {

class TagDAO {
public:
    // Constructor
    explicit TagDAO(utils::Database& db) : db_(db) {}

    // Destructor
    ~TagDAO() = default;

    // Create a new tag
    bool createTag(const models::Tag& tag);

    // Get a tag by ID
    models::Tag getTagById(int id);

    // Get a tag by name
    models::Tag getTagByName(const std::string& name);

    // Get all tags
    std::vector<models::Tag> getAllTags(int page = 1, int page_size = 20);

    // Update a tag's information
    bool updateTag(const models::Tag& tag);

    // Delete a tag by ID
    bool deleteTag(int id);

private:
    utils::Database& db_;

public:
    // Convert a database row to a Tag object
    models::Tag rowToTag(const std::map<std::string, std::string>& row);
};

} // namespace daos
