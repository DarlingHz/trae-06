#include "controllers/AlbumController.hpp"
#include <stdexcept>
#include <ctime>

namespace controllers {

bool AlbumController::validateAlbumCreationRequest(const nlohmann::json& request, std::string& error_message) {
    // Check if all required fields are present
    if (!request.contains("title") || !request.contains("visibility")) {
        error_message = "Missing required fields: title, visibility";
        return false;
    }

    // Check if fields are not empty
    std::string title = request["title"];
    std::string visibility = request["visibility"];

    if (title.empty() || visibility.empty()) {
        error_message = "Title and visibility cannot be empty";
        return false;
    }

    // Validate visibility value
    if (visibility != "private" && visibility != "public") {
        error_message = "Invalid visibility value: must be 'private' or 'public'";
        return false;
    }

    // Validate title length (optional, but recommended)
    if (title.length() > 100) {
        error_message = "Title cannot be longer than 100 characters";
        return false;
    }

    // Validate description length (if present)
    if (request.contains("description") && request["description"].is_string()) {
        std::string description = request["description"];
        if (description.length() > 500) {
            error_message = "Description cannot be longer than 500 characters";
            return false;
        }
    }

    return true;
}

bool AlbumController::validateAlbumUpdateRequest(const nlohmann::json& request, std::string& error_message) {
    // Check if at least one field is present to update
    if (!request.contains("title") && !request.contains("description") && !request.contains("visibility")) {
        error_message = "No fields to update: title, description, visibility";
        return false;
    }

    // Validate title if present
    if (request.contains("title") && request["title"].is_string()) {
        std::string title = request["title"];
        if (title.empty()) {
            error_message = "Title cannot be empty";
            return false;
        }
        if (title.length() > 100) {
            error_message = "Title cannot be longer than 100 characters";
            return false;
        }
    }

    // Validate description if present
    if (request.contains("description") && request["description"].is_string()) {
        std::string description = request["description"];
        if (description.length() > 500) {
            error_message = "Description cannot be longer than 500 characters";
            return false;
        }
    }

    // Validate visibility if present
    if (request.contains("visibility") && request["visibility"].is_string()) {
        std::string visibility = request["visibility"];
        if (visibility.empty()) {
            error_message = "Visibility cannot be empty";
            return false;
        }
        if (visibility != "private" && visibility != "public") {
            error_message = "Invalid visibility value: must be 'private' or 'public'";
            return false;
        }
    }

    return true;
}

bool AlbumController::isAlbumOwner(int album_id, int user_id, std::string& error_message) {
    // Get album by ID
    models::Album album = album_dao_.getAlbumById(album_id);

    // Check if album exists
    if (album.getId() == 0) {
        error_message = "Album not found";
        return false;
    }

    // Check if album belongs to the current user
    if (album.getOwnerId() != user_id) {
        error_message = "You are not the owner of this album";
        return false;
    }

    return true;
}

bool AlbumController::isAlbumAccessible(int album_id, int user_id, std::string& error_message) {
    // Get album by ID
    models::Album album = album_dao_.getAlbumById(album_id);

    // Check if album exists
    if (album.getId() == 0) {
        error_message = "Album not found";
        return false;
    }

    // Check if album is public or belongs to the current user
    if (album.getVisibility() == "private" && album.getOwnerId() != user_id) {
        error_message = "You do not have access to this album";
        return false;
    }

    return true;
}

crow::response AlbumController::createAlbum(const crow::request& req, const std::string& token) {
    try {
        // Verify token and get user ID
        int user_id = utils::AuthUtils::verifyTokenAndGetUserId(token);
        if (user_id == 0) {
            return crow::response(401, utils::JsonUtils::createErrorResponse("UNAUTHORIZED", "Invalid or expired token"));
        }

        // Parse request body to JSON
        nlohmann::json request_body = utils::JsonUtils::parse(req.body);

        // Validate album creation request
        std::string error_message;
        if (!validateAlbumCreationRequest(request_body, error_message)) {
            return crow::response(400, utils::JsonUtils::createErrorResponse("BAD_REQUEST", error_message));
        }

        // Create new album object
        models::Album new_album;
        new_album.setOwnerId(user_id);
        new_album.setTitle(request_body["title"]);

        // Set description if present
        if (request_body.contains("description") && request_body["description"].is_string()) {
            new_album.setDescription(request_body["description"]);
        }

        new_album.setVisibility(request_body["visibility"]);

        // Set created_at and updated_at timestamps
        std::time_t now = std::time(nullptr);
        char buf[20];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        std::string timestamp = buf;

        new_album.setCreatedAt(timestamp);
        new_album.setUpdatedAt(timestamp);

        // Save album to database
        bool created = album_dao_.createAlbum(new_album);
        if (!created) {
            return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", "Failed to create album"));
        }

        // Get the created album (with ID)
        models::Album created_album = album_dao_.getAlbumById(new_album.getId());

        // Prepare response
        nlohmann::json response_body = created_album.toJson();

        return crow::response(201, response_body.dump());

    } catch (const nlohmann::json::parse_error& e) {
        return crow::response(400, utils::JsonUtils::createErrorResponse("BAD_REQUEST", "Invalid JSON format"));
    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

crow::response AlbumController::getMyAlbums(const crow::request& req, const std::string& token) {
    try {
        // Verify token and get user ID
        int user_id = utils::AuthUtils::verifyTokenAndGetUserId(token);
        if (user_id == 0) {
            return crow::response(401, utils::JsonUtils::createErrorResponse("UNAUTHORIZED", "Invalid or expired token"));
        }

        // Get pagination parameters from query string
        int page = 1;
        int page_size = 20;
        const int max_page_size = 100;

        if (req.url_params.get("page")) {
            page = std::stoi(req.url_params.get("page"));
            if (page < 1) {
                page = 1;
            }
        }

        if (req.url_params.get("page_size")) {
            page_size = std::stoi(req.url_params.get("page_size"));
            if (page_size < 1) {
                page_size = 1;
            } else if (page_size > max_page_size) {
                page_size = max_page_size;
            }
        }

        // Calculate offset
        int offset = (page - 1) * page_size;

        // Get albums by owner ID with pagination
        std::vector<models::Album> albums = album_dao_.getAlbumsByOwnerId(user_id, offset, page_size);

        // Get total number of albums for this user
        int total_albums = album_dao_.getAlbumCountByOwnerId(user_id);

        // Prepare response
        nlohmann::json response_body;
        response_body["albums"] = nlohmann::json::array();

        for (const auto& album : albums) {
            nlohmann::json album_json = album.toJson();
            // Add image count to album JSON
            int image_count = image_item_dao_.getImageItemCountByAlbumId(album.getId());
            album_json["image_count"] = image_count;
            response_body["albums"].push_back(album_json);
        }

        // Add pagination information
        response_body["pagination"] = {
            {"page", page},
            {"page_size", page_size},
            {"total_albums", total_albums},
            {"total_pages", (total_albums + page_size - 1) / page_size}
        };

        return crow::response(200, response_body.dump());

    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

crow::response AlbumController::getAlbumById(const crow::request& req [[maybe_unused]], const std::string& token, int album_id) {
    try {
        // Get current user ID from token (if present)
        int user_id = 0;
        if (!token.empty()) {
            user_id = utils::AuthUtils::verifyTokenAndGetUserId(token);
            // If token is invalid, treat as anonymous user
            if (user_id == -1) {
                user_id = 0;
            }
        }

        // Check if album is accessible to the current user
        std::string error_message;
        if (!isAlbumAccessible(album_id, user_id, error_message)) {
            if (error_message == "Album not found") {
                return crow::response(404, utils::JsonUtils::createErrorResponse("RESOURCE_NOT_FOUND", error_message));
            } else {
                return crow::response(403, utils::JsonUtils::createErrorResponse("FORBIDDEN", error_message));
            }
        }

        // Get album by ID
        models::Album album = album_dao_.getAlbumById(album_id);

        // Get image count for this album
        int image_count = image_item_dao_.getImageItemCountByAlbumId(album.getId());

        // Prepare response
        nlohmann::json response_body = album.toJson();
        response_body["image_count"] = image_count;

        return crow::response(200, response_body.dump());

    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

crow::response AlbumController::updateAlbum(const crow::request& req, const std::string& token, int album_id) {
    try {
        // Verify token and get user ID
        int user_id = utils::AuthUtils::verifyTokenAndGetUserId(token);
        if (user_id == 0) {
            return crow::response(401, utils::JsonUtils::createErrorResponse("UNAUTHORIZED", "Invalid or expired token"));
        }

        // Check if album belongs to the current user
        std::string error_message;
        if (!isAlbumOwner(album_id, user_id, error_message)) {
            if (error_message == "Album not found") {
                return crow::response(404, utils::JsonUtils::createErrorResponse("RESOURCE_NOT_FOUND", error_message));
            } else {
                return crow::response(403, utils::JsonUtils::createErrorResponse("FORBIDDEN", error_message));
            }
        }

        // Parse request body to JSON
        nlohmann::json request_body = utils::JsonUtils::parse(req.body);

        // Validate album update request
        if (!validateAlbumUpdateRequest(request_body, error_message)) {
            return crow::response(400, utils::JsonUtils::createErrorResponse("BAD_REQUEST", error_message));
        }

        // Get album by ID
        models::Album album = album_dao_.getAlbumById(album_id);

        // Update album fields
        if (request_body.contains("title")) {
            album.setTitle(request_body["title"]);
        }

        if (request_body.contains("description")) {
            album.setDescription(request_body["description"]);
        }

        if (request_body.contains("visibility")) {
            album.setVisibility(request_body["visibility"]);
        }

        // Set updated_at timestamp
        std::time_t now = std::time(nullptr);
        char buf[20];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        album.setUpdatedAt(buf);

        // Update album in database
        bool updated = album_dao_.updateAlbum(album);
        if (!updated) {
            return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", "Failed to update album"));
        }

        // Get the updated album
        models::Album updated_album = album_dao_.getAlbumById(album_id);

        // Get image count for this album
        int image_count = image_item_dao_.getImageItemCountByAlbumId(album.getId());

        // Prepare response
        nlohmann::json response_body = updated_album.toJson();
        response_body["image_count"] = image_count;

        return crow::response(200, response_body.dump());

    } catch (const nlohmann::json::parse_error& e) {
        return crow::response(400, utils::JsonUtils::createErrorResponse("BAD_REQUEST", "Invalid JSON format"));
    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

crow::response AlbumController::deleteAlbum(const crow::request& req [[maybe_unused]], const std::string& token, int album_id) {
    try {
        // Verify token and get user ID
        int user_id = utils::AuthUtils::verifyTokenAndGetUserId(token);
        if (user_id == 0) {
            return crow::response(401, utils::JsonUtils::createErrorResponse("UNAUTHORIZED", "Invalid or expired token"));
        }

        // Check if album belongs to the current user
        std::string error_message;
        if (!isAlbumOwner(album_id, user_id, error_message)) {
            if (error_message == "Album not found") {
                return crow::response(404, utils::JsonUtils::createErrorResponse("RESOURCE_NOT_FOUND", error_message));
            } else {
                return crow::response(403, utils::JsonUtils::createErrorResponse("FORBIDDEN", error_message));
            }
        }

        // Start transaction to ensure atomicity
        album_dao_.beginTransaction();

        try {
            // Delete all images in this album
            bool images_deleted = image_item_dao_.deleteImageItemsByAlbumId(album_id);
            if (!images_deleted) {
                album_dao_.rollbackTransaction();
                return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", "Failed to delete images in album"));
            }

            // Delete the album itself
            bool album_deleted = album_dao_.deleteAlbum(album_id);
            if (!album_deleted) {
                album_dao_.rollbackTransaction();
                return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", "Failed to delete album"));
            }

            // Commit transaction
            album_dao_.commitTransaction();

            return crow::response(204);

        } catch (const std::exception& e) {
            // Rollback transaction on error
            album_dao_.rollbackTransaction();
            throw;
        }

    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

} // namespace controllers
