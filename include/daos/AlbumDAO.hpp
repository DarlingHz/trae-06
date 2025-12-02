#pragma once

#include "utils/Database.hpp"
#include "models/Album.hpp"
#include <vector>

namespace daos {

class AlbumDAO {
public:
    // Constructor
    explicit AlbumDAO(utils::Database& db) : db_(db) {}

    // Destructor
    ~AlbumDAO() = default;

    // Create a new album
    bool createAlbum(const models::Album& album);

    // Get an album by ID
    models::Album getAlbumById(int id);

    // Get all albums by owner ID
    std::vector<models::Album> getAlbumsByOwnerId(int owner_id, int page = 1, int page_size = 20);

    // Get the number of albums by owner ID
    int getAlbumCountByOwnerId(int owner_id);

    // Update an album's information
    bool updateAlbum(const models::Album& album);

    // Delete an album by ID
    bool deleteAlbum(int id);

    // Check if album is accessible by user
    bool isAlbumAccessible(int album_id, int user_id, std::string& error_message);

    // Transaction management methods
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

private:
    utils::Database& db_;

    // Convert a database row to an Album object
    models::Album rowToAlbum(const std::map<std::string, std::string>& row);
};

} // namespace daos
