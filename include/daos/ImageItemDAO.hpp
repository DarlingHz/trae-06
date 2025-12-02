#pragma once

#include "utils/Database.hpp"
#include "models/ImageItem.hpp"
#include <vector>

namespace daos {

class ImageItemDAO {
public:
    // Constructor
    explicit ImageItemDAO(utils::Database& db) : db_(db) {}

    // Destructor
    ~ImageItemDAO() = default;

    // Create a new image item
    bool createImageItem(const models::ImageItem& image_item);

    // Get an image item by ID
    models::ImageItem getImageItemById(int id);

    // Get all image items by album ID
    std::vector<models::ImageItem> getImageItemsByAlbumId(int album_id, int page = 1, int page_size = 20, const std::string& tag = "");

    // Get the number of image items by album ID
    int getImageItemCountByAlbumId(int album_id, const std::string& tag = "");

    // Get all image items by owner ID
    std::vector<models::ImageItem> getImageItemsByOwnerId(int owner_id, int page = 1, int page_size = 20);

    // Update an image item's information
    bool updateImageItem(const models::ImageItem& image_item);

    // Delete an image item by ID
    bool deleteImageItem(int id);

    // Delete all image items by album ID
    bool deleteImageItemsByAlbumId(int album_id);

    // Search public image items by keyword, tag, or owner
    std::vector<models::ImageItem> searchPublicImageItems(const std::string& keyword = "", const std::string& tag = "", const std::string& owner = "", int page = 1, int page_size = 20);

    // Get popular public image items by like count
    std::vector<models::ImageItem> getPopularPublicImageItems(int limit = 20);

    // Get the number of public image items matching search criteria
    int getPublicImageItemCount(const std::string& keyword = "", const std::string& tag = "", const std::string& owner = "");

    // Transaction management methods
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

private:
    utils::Database& db_;

public:
    // Convert a database row to an ImageItem object
    models::ImageItem rowToImageItem(const std::map<std::string, std::string>& row);
};

} // namespace daos
