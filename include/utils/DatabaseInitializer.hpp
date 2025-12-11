#pragma once

#include "utils/Database.hpp"

namespace utils {

class DatabaseInitializer {
public:
    explicit DatabaseInitializer(Database& db);
    bool initialize();

private:
    Database& db_;
    bool createUsersTable();
    bool createAlbumsTable();
    bool createImageItemsTable();
    bool createTagsTable();
    bool createImageTagsTable();
    bool createImageLikesTable();
};

} // namespace utils
