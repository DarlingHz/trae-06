#pragma once

#include "daos/AlbumDAO.hpp"
#include "daos/ImageItemDAO.hpp"
#include "utils/AuthUtils.hpp"
#include "utils/JsonUtils.hpp"
#include <crow.h>

namespace controllers {

class AlbumController {
public:
    // Constructor
    explicit AlbumController(daos::AlbumDAO& album_dao, daos::ImageItemDAO& image_item_dao) 
        : album_dao_(album_dao), image_item_dao_(image_item_dao) {}

    // Destructor
    ~AlbumController() = default;

    // Handle creating a new album
    crow::response createAlbum(const crow::request& req, const std::string& token);

    // Handle getting all albums of the current user
    crow::response getMyAlbums(const crow::request& req, const std::string& token);

    // Handle getting a single album by ID
    crow::response getAlbumById(const crow::request& req, const std::string& token, int album_id);

    // Handle updating an album
    crow::response updateAlbum(const crow::request& req, const std::string& token, int album_id);

    // Handle deleting an album
    crow::response deleteAlbum(const crow::request& req, const std::string& token, int album_id);

private:
    daos::AlbumDAO& album_dao_;
    daos::ImageItemDAO& image_item_dao_;

    // Validate album creation request
    bool validateAlbumCreationRequest(const nlohmann::json& request, std::string& error_message);

    // Validate album update request
    bool validateAlbumUpdateRequest(const nlohmann::json& request, std::string& error_message);

    // Check if album exists and belongs to the current user
    bool isAlbumOwner(int album_id, int user_id, std::string& error_message);

    // Check if album exists and is accessible to the current user
    bool isAlbumAccessible(int album_id, int user_id, std::string& error_message);
};

} // namespace controllers
