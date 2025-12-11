#pragma once

#include "utils/Database.hpp"
#include "models/ImageLike.hpp"
#include <vector>

namespace daos {

class ImageLikeDAO {
public:
    // Constructor
    explicit ImageLikeDAO(utils::Database& db) : db_(db) {}

    // Destructor
    ~ImageLikeDAO() = default;

    // Create a new image like
    bool createImageLike(const models::ImageLike& image_like);

    // Get an image like by ID
    models::ImageLike getImageLikeById(int id);

    // Get an image like by image ID and user ID
    models::ImageLike getImageLikeByImageIdAndUserId(int image_id, int user_id);

    // Get all image likes for a specific image
    std::vector<models::ImageLike> getImageLikesByImageId(int image_id);

    // Get all image likes for a specific user
    std::vector<models::ImageLike> getImageLikesByUserId(int user_id);

    // Delete an image like by ID
    bool deleteImageLike(int id);

    // Delete an image like by image ID and user ID
    bool deleteImageLikeByImageIdAndUserId(int image_id, int user_id);

    // Delete all image likes for a specific image
    bool deleteImageLikesByImageId(int image_id);

    // Delete all image likes for a specific user
    bool deleteImageLikesByUserId(int user_id);

    // Get the number of likes for a specific image
    int getImageLikeCount(int image_id);

    // Get recent image likes for a specific image
    std::vector<models::ImageLike> getRecentImageLikes(int image_id, int limit = 10);

private:
    utils::Database& db_;

    // Convert a database row to an ImageLike object
    models::ImageLike rowToImageLike(const std::map<std::string, std::string>& row);
};

} // namespace daos
