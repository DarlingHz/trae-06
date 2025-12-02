#pragma once

#include "utils/Database.hpp"
#include "models/ImageTag.hpp"
#include "models/Tag.hpp"
#include "models/ImageItem.hpp"
#include <vector>

namespace daos {

class ImageTagDAO {
public:
    // Constructor
    explicit ImageTagDAO(utils::Database& db) : db_(db) {}

    // Destructor
    ~ImageTagDAO() = default;

    // Create a new image-tag relationship
    bool createImageTag(const models::ImageTag& image_tag);

    // Get an image-tag relationship by ID
    models::ImageTag getImageTagById(int id);

    // Get all tags associated with an image
    std::vector<models::Tag> getTagsByImageId(int image_id);

    // Get all images associated with a tag
    std::vector<models::ImageItem> getImagesByTagId(int tag_id);

    // Delete an image-tag relationship by ID
    bool deleteImageTag(int id);

    // Delete all image-tag relationships for a specific image
    bool deleteImageTagsByImageId(int image_id);

    // Delete all image-tag relationships for a specific tag
    bool deleteImageTagsByTagId(int tag_id);

private:
    utils::Database& db_;

    // Convert a database row to an ImageTag object
    models::ImageTag rowToImageTag(const std::map<std::string, std::string>& row);
};

} // namespace daos
