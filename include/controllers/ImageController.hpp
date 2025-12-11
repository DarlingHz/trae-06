#pragma once

#include "daos/ImageItemDAO.hpp"
#include "daos/AlbumDAO.hpp"
#include "daos/TagDAO.hpp"
#include "daos/ImageTagDAO.hpp"
#include "daos/ImageLikeDAO.hpp"
#include "daos/UserDAO.hpp"
#include "utils/AuthUtils.hpp"
#include "utils/JsonUtils.hpp"
#include <crow.h>

namespace controllers {

class ImageController {
public:
    // Constructor
    explicit ImageController(
        daos::ImageItemDAO& image_item_dao,
        daos::AlbumDAO& album_dao,
        daos::TagDAO& tag_dao,
        daos::ImageTagDAO& image_tag_dao,
        daos::ImageLikeDAO& image_like_dao,
        daos::UserDAO& user_dao
    ) : 
        image_item_dao_(image_item_dao),
        album_dao_(album_dao),
        tag_dao_(tag_dao),
        image_tag_dao_(image_tag_dao),
        image_like_dao_(image_like_dao),
        user_dao_(user_dao) {}

    // Destructor
    ~ImageController() = default;

    // Handle adding an image to an album
    crow::response addImageToAlbum(const crow::request& req, const std::string& token, int album_id);

    // Handle getting images in an album
    crow::response getImagesInAlbum(const crow::request& req, const std::string& token, int album_id);

    // Handle updating an image
    crow::response updateImage(const crow::request& req, const std::string& token, int image_id);

    // Handle deleting an image
    crow::response deleteImage(const crow::request& req, const std::string& token, int image_id);

    // Handle searching public images
    crow::response searchPublicImages(const crow::request& req);

    // Handle getting popular public images
    crow::response getPopularPublicImages(const crow::request& req);

    // Handle liking an image
    crow::response likeImage(const crow::request& req, const std::string& token, int image_id);

    // Handle unliking an image
    crow::response unlikeImage(const crow::request& req, const std::string& token, int image_id);

    // Handle getting image like information
    crow::response getImageLikes(const crow::request& req, const std::string& token, int image_id);

private:
    daos::ImageItemDAO& image_item_dao_;
    daos::AlbumDAO& album_dao_;
    daos::TagDAO& tag_dao_;
    daos::ImageTagDAO& image_tag_dao_;
    daos::ImageLikeDAO& image_like_dao_;
    daos::UserDAO& user_dao_;

    // Validate image creation request
    bool validateImageCreationRequest(const nlohmann::json& request, std::string& error_message);

    // Validate image update request
    bool validateImageUpdateRequest(const nlohmann::json& request, std::string& error_message);

    // Check if image exists and belongs to the current user
    bool isImageOwner(int image_id, int user_id, std::string& error_message);

    // Check if image exists and is accessible to the current user
    bool isImageAccessible(int image_id, int user_id, std::string& error_message);

    // Process tags for an image
    bool processImageTags(int image_id, const std::vector<std::string>& tags, std::string& error_message);

    // Add image count to image JSON
    void addImageCountToImageJson(nlohmann::json& image_json, int image_id);
};

} // namespace controllers
