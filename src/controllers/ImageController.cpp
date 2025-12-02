#include "controllers/ImageController.hpp"
#include <stdexcept>
#include <ctime>
#include <algorithm>
#include <cctype>

namespace controllers {

bool ImageController::validateImageCreationRequest(const nlohmann::json& request, std::string& error_message) {
    // Check if all required fields are present
    if (!request.contains("image_url") || !request.contains("title")) {
        error_message = "Missing required fields: image_url, title";
        return false;
    }

    // Check if fields are not empty
    std::string image_url = request["image_url"];
    std::string title = request["title"];

    if (image_url.empty() || title.empty()) {
        error_message = "Image URL and title cannot be empty";
        return false;
    }

    // Validate image URL format (basic check for http/https prefix)
    if (image_url.substr(0, 7) != "http://" && image_url.substr(0, 8) != "https://") {
        error_message = "Invalid image URL format: must start with http:// or https://";
        return false;
    }

    // Validate title length
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

    // Validate source page URL format (if present)
    if (request.contains("source_page_url") && request["source_page_url"].is_string()) {
        std::string source_page_url = request["source_page_url"];
        if (!source_page_url.empty() && source_page_url.substr(0, 7) != "http://" && source_page_url.substr(0, 8) != "https://") {
            error_message = "Invalid source page URL format: must start with http:// or https://";
            return false;
        }
    }

    // Validate tags (if present)
    if (request.contains("tags") && request["tags"].is_array()) {
        const auto& tags = request["tags"];
        for (const auto& tag : tags) {
            if (!tag.is_string()) {
                error_message = "Invalid tag format: all tags must be strings";
                return false;
            }
            std::string tag_str = tag;
            if (tag_str.empty()) {
                error_message = "Invalid tag: tags cannot be empty";
                return false;
            }
            if (tag_str.length() > 50) {
                error_message = "Invalid tag: tags cannot be longer than 50 characters";
                return false;
            }
            // Check if tag contains only alphanumeric characters and underscores
            for (char c : tag_str) {
                if (!std::isalnum(c) && c != '_') {
                    error_message = "Invalid tag: tags can only contain alphanumeric characters and underscores";
                    return false;
                }
            }
        }
    }

    return true;
}

bool ImageController::validateImageUpdateRequest(const nlohmann::json& request, std::string& error_message) {
    // Check if at least one field is present to update
    if (!request.contains("image_url") && !request.contains("title") && !request.contains("description") && 
        !request.contains("source_page_url") && !request.contains("tags")) {
        error_message = "No fields to update: image_url, title, description, source_page_url, tags";
        return false;
    }

    // Validate image URL if present
    if (request.contains("image_url") && request["image_url"].is_string()) {
        std::string image_url = request["image_url"];
        if (image_url.empty()) {
            error_message = "Image URL cannot be empty";
            return false;
        }
        if (image_url.substr(0, 7) != "http://" && image_url.substr(0, 8) != "https://") {
            error_message = "Invalid image URL format: must start with http:// or https://";
            return false;
        }
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

    // Validate source page URL if present
    if (request.contains("source_page_url") && request["source_page_url"].is_string()) {
        std::string source_page_url = request["source_page_url"];
        if (!source_page_url.empty() && source_page_url.substr(0, 7) != "http://" && source_page_url.substr(0, 8) != "https://") {
            error_message = "Invalid source page URL format: must start with http:// or https://";
            return false;
        }
    }

    // Validate tags if present
    if (request.contains("tags") && request["tags"].is_array()) {
        const auto& tags = request["tags"];
        for (const auto& tag : tags) {
            if (!tag.is_string()) {
                error_message = "Invalid tag format: all tags must be strings";
                return false;
            }
            std::string tag_str = tag;
            if (tag_str.empty()) {
                error_message = "Invalid tag: tags cannot be empty";
                return false;
            }
            if (tag_str.length() > 50) {
                error_message = "Invalid tag: tags cannot be longer than 50 characters";
                return false;
            }
            // Check if tag contains only alphanumeric characters and underscores
            for (char c : tag_str) {
                if (!std::isalnum(c) && c != '_') {
                    error_message = "Invalid tag: tags can only contain alphanumeric characters and underscores";
                    return false;
                }
            }
        }
    }

    return true;
}

bool ImageController::isImageOwner(int image_id, int user_id, std::string& error_message) {
    // Get image by ID
    models::ImageItem image = image_item_dao_.getImageItemById(image_id);

    // Check if image exists
    if (image.getId() == 0) {
        error_message = "Image not found";
        return false;
    }

    // Check if image belongs to the current user
    if (image.getOwnerId() != user_id) {
        error_message = "You are not the owner of this image";
        return false;
    }

    return true;
}

bool ImageController::isImageAccessible(int image_id, int user_id, std::string& error_message) {
    // Get image by ID
    models::ImageItem image = image_item_dao_.getImageItemById(image_id);

    // Check if image exists
    if (image.getId() == 0) {
        error_message = "Image not found";
        return false;
    }

    // Get album by ID to check visibility
    models::Album album = album_dao_.getAlbumById(image.getAlbumId());

    // Check if album exists
    if (album.getId() == 0) {
        error_message = "Album not found";
        return false;
    }

    // Check if album is public or belongs to the current user
    if (album.getVisibility() == "private" && album.getOwnerId() != user_id) {
        error_message = "You do not have access to this image";
        return false;
    }

    return true;
}

bool ImageController::processImageTags(int image_id, const std::vector<std::string>& tags, std::string& error_message) {
    try {
        // Delete existing tags for this image
        bool existing_tags_deleted = image_tag_dao_.deleteImageTagsByImageId(image_id);
        if (!existing_tags_deleted) {
            error_message = "Failed to delete existing tags for image";
            return false;
        }

        // Process each tag
        for (const std::string& tag_name : tags) {
            // Check if tag already exists
            models::Tag existing_tag = tag_dao_.getTagByName(tag_name);

            int tag_id;
            if (existing_tag.getId() == 0) {
                // Create new tag
                models::Tag new_tag;
                new_tag.setName(tag_name);

                // Set created_at timestamp
                std::time_t now = std::time(nullptr);
                char buf[20];
                std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
                new_tag.setCreatedAt(buf);

                // Save tag to database
                bool tag_created = tag_dao_.createTag(new_tag);
                if (!tag_created) {
                    error_message = "Failed to create tag: " + tag_name;
                    return false;
                }

                // Get the created tag ID
                existing_tag = tag_dao_.getTagByName(tag_name);
                if (existing_tag.getId() == 0) {
                    error_message = "Failed to get created tag ID: " + tag_name;
                    return false;
                }

                tag_id = existing_tag.getId();
            } else {
                // Use existing tag ID
                tag_id = existing_tag.getId();
            }

            // Create image-tag relationship
            models::ImageTag image_tag;
            image_tag.setImageId(image_id);
            image_tag.setTagId(tag_id);

            bool image_tag_created = image_tag_dao_.createImageTag(image_tag);
            if (!image_tag_created) {
                error_message = "Failed to create image-tag relationship for tag: " + tag_name;
                return false;
            }
        }

        return true;

    } catch (const std::exception& e) {
        error_message = "Failed to process image tags: " + std::string(e.what());
        return false;
    }
}

void ImageController::addImageCountToImageJson(nlohmann::json& image_json, int image_id) {
    // Get like count for this image
    int like_count = image_like_dao_.getImageLikeCount(image_id);
    image_json["like_count"] = like_count;
}

crow::response ImageController::addImageToAlbum(const crow::request& req, const std::string& token, int album_id) {
    try {
        // Verify token and get user ID
        int user_id = utils::AuthUtils::verifyTokenAndGetUserId(token);
        if (user_id == 0) {
            return crow::response(401, utils::JsonUtils::createErrorResponse("UNAUTHORIZED", "Invalid or expired token"));
        }

        // Check if album exists and belongs to the current user
        std::string error_message;
        if (!album_dao_.isAlbumAccessible(album_id, user_id, error_message)) {
            if (error_message == "Album not found") {
                return crow::response(404, utils::JsonUtils::createErrorResponse("RESOURCE_NOT_FOUND", error_message));
            } else {
                return crow::response(403, utils::JsonUtils::createErrorResponse("FORBIDDEN", error_message));
            }
        }

        // Parse request body to JSON
        nlohmann::json request_body = utils::JsonUtils::parse(req.body);

        // Validate image creation request
        if (!validateImageCreationRequest(request_body, error_message)) {
            return crow::response(400, utils::JsonUtils::createErrorResponse("BAD_REQUEST", error_message));
        }

        // Create new image item object
        models::ImageItem new_image;
        new_image.setAlbumId(album_id);
        new_image.setOwnerId(user_id);
        new_image.setImageUrl(request_body["image_url"]);
        new_image.setTitle(request_body["title"]);

        // Set description if present
        if (request_body.contains("description") && request_body["description"].is_string()) {
            new_image.setDescription(request_body["description"]);
        }

        // Set source page URL if present
        if (request_body.contains("source_page_url") && request_body["source_page_url"].is_string()) {
            new_image.setSourcePageUrl(request_body["source_page_url"]);
        }

        // Set created_at timestamp
        std::time_t now = std::time(nullptr);
        char buf[20];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        new_image.setCreatedAt(buf);

        // Start transaction to ensure atomicity
        image_item_dao_.beginTransaction();

        try {
            // Save image to database
            bool created = image_item_dao_.createImageItem(new_image);
            if (!created) {
                image_item_dao_.rollbackTransaction();
                return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", "Failed to create image"));
            }

            // Process tags if present
            if (request_body.contains("tags") && request_body["tags"].is_array()) {
                std::vector<std::string> tags;
                for (const auto& tag : request_body["tags"]) {
                    tags.push_back(tag.get<std::string>());
                }

                bool tags_processed = processImageTags(new_image.getId(), tags, error_message);
                if (!tags_processed) {
                    image_item_dao_.rollbackTransaction();
                    return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", error_message));
                }
            }

            // Commit transaction
            image_item_dao_.commitTransaction();

            // Get the created image (with ID)
            models::ImageItem created_image = image_item_dao_.getImageItemById(new_image.getId());

            // Get tags for this image
            std::vector<models::Tag> tags = image_tag_dao_.getTagsByImageId(created_image.getId());

            // Prepare response
            nlohmann::json response_body = created_image.toJson();

            // Add tags to response
            nlohmann::json tags_json = nlohmann::json::array();
            for (const auto& tag : tags) {
                tags_json.push_back(tag.toJson());
            }
            response_body["tags"] = tags_json;

            // Add like count to response
            addImageCountToImageJson(response_body, created_image.getId());

            return crow::response(201, response_body.dump());

        } catch (const std::exception& e) {
            // Rollback transaction on error
            image_item_dao_.rollbackTransaction();
            throw;
        }

    } catch (const nlohmann::json::parse_error& e) {
        return crow::response(400, utils::JsonUtils::createErrorResponse("BAD_REQUEST", "Invalid JSON format"));
    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

crow::response ImageController::getImagesInAlbum(const crow::request& req, const std::string& token, int album_id) {
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
        if (!album_dao_.isAlbumAccessible(album_id, user_id, error_message)) {
            if (error_message == "Album not found") {
                return crow::response(404, utils::JsonUtils::createErrorResponse("RESOURCE_NOT_FOUND", error_message));
            } else {
                return crow::response(403, utils::JsonUtils::createErrorResponse("FORBIDDEN", error_message));
            }
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

        // Get tag filter from query string (if present)
        std::string tag_filter;
        if (req.url_params.get("tag")) {
            tag_filter = req.url_params.get("tag");
        }

        // Calculate offset
        int offset = (page - 1) * page_size;

        // Get images by album ID with pagination and tag filter
        std::vector<models::ImageItem> images = image_item_dao_.getImageItemsByAlbumId(album_id, offset, page_size, tag_filter);

        // Get total number of images for this album (with tag filter if present)
        int total_images = image_item_dao_.getImageItemCountByAlbumId(album_id, tag_filter);

        // Prepare response
        nlohmann::json response_body;
        response_body["images"] = nlohmann::json::array();

        for (const auto& image : images) {
            nlohmann::json image_json = image.toJson();

            // Get tags for this image
            std::vector<models::Tag> tags = image_tag_dao_.getTagsByImageId(image.getId());
            nlohmann::json tags_json = nlohmann::json::array();
            for (const auto& tag : tags) {
                tags_json.push_back(tag.toJson());
            }
            image_json["tags"] = tags_json;

            // Add like count to image JSON
            addImageCountToImageJson(image_json, image.getId());

            response_body["images"].push_back(image_json);
        }

        // Add pagination information
        response_body["pagination"] = {
            {"page", page},
            {"page_size", page_size},
            {"total_images", total_images},
            {"total_pages", (total_images + page_size - 1) / page_size}
        };

        return crow::response(200, response_body.dump());

    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

crow::response ImageController::updateImage(const crow::request& req, const std::string& token, int image_id) {
    try {
        // Verify token and get user ID
        int user_id = utils::AuthUtils::verifyTokenAndGetUserId(token);
        if (user_id == 0) {
            return crow::response(401, utils::JsonUtils::createErrorResponse("UNAUTHORIZED", "Invalid or expired token"));
        }

        // Check if image exists and belongs to the current user
        std::string error_message;
        if (!isImageOwner(image_id, user_id, error_message)) {
            if (error_message == "Image not found") {
                return crow::response(404, utils::JsonUtils::createErrorResponse("RESOURCE_NOT_FOUND", error_message));
            } else {
                return crow::response(403, utils::JsonUtils::createErrorResponse("FORBIDDEN", error_message));
            }
        }

        // Parse request body to JSON
        nlohmann::json request_body = utils::JsonUtils::parse(req.body);

        // Validate image update request
        if (!validateImageUpdateRequest(request_body, error_message)) {
            return crow::response(400, utils::JsonUtils::createErrorResponse("BAD_REQUEST", error_message));
        }

        // Get image by ID
        models::ImageItem image = image_item_dao_.getImageItemById(image_id);

        // Update image fields
        if (request_body.contains("image_url")) {
            image.setImageUrl(request_body["image_url"]);
        }

        if (request_body.contains("title")) {
            image.setTitle(request_body["title"]);
        }

        if (request_body.contains("description")) {
            image.setDescription(request_body["description"]);
        }

        if (request_body.contains("source_page_url")) {
            image.setSourcePageUrl(request_body["source_page_url"]);
        }

        // Start transaction to ensure atomicity
        image_item_dao_.beginTransaction();

        try {
            // Update image in database
            bool updated = image_item_dao_.updateImageItem(image);
            if (!updated) {
                image_item_dao_.rollbackTransaction();
                return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", "Failed to update image"));
            }

            // Process tags if present
            if (request_body.contains("tags") && request_body["tags"].is_array()) {
                std::vector<std::string> tags;
                for (const auto& tag : request_body["tags"]) {
                    tags.push_back(tag.get<std::string>());
                }

                bool tags_processed = processImageTags(image_id, tags, error_message);
                if (!tags_processed) {
                    image_item_dao_.rollbackTransaction();
                    return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", error_message));
                }
            }

            // Commit transaction
            image_item_dao_.commitTransaction();

            // Get the updated image
            models::ImageItem updated_image = image_item_dao_.getImageItemById(image_id);

            // Get tags for this image
            std::vector<models::Tag> tags = image_tag_dao_.getTagsByImageId(updated_image.getId());

            // Prepare response
            nlohmann::json response_body = updated_image.toJson();

            // Add tags to response
            nlohmann::json tags_json = nlohmann::json::array();
            for (const auto& tag : tags) {
                tags_json.push_back(tag.toJson());
            }
            response_body["tags"] = tags_json;

            // Add like count to response
            addImageCountToImageJson(response_body, updated_image.getId());

            return crow::response(200, response_body.dump());

        } catch (const std::exception& e) {
            // Rollback transaction on error
            image_item_dao_.rollbackTransaction();
            throw;
        }

    } catch (const nlohmann::json::parse_error& e) {
        return crow::response(400, utils::JsonUtils::createErrorResponse("BAD_REQUEST", "Invalid JSON format"));
    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

crow::response ImageController::deleteImage(const crow::request& req [[maybe_unused]], const std::string& token, int image_id) {
    try {
        // Verify token and get user ID
        int user_id = utils::AuthUtils::verifyTokenAndGetUserId(token);
        if (user_id == 0) {
            return crow::response(401, utils::JsonUtils::createErrorResponse("UNAUTHORIZED", "Invalid or expired token"));
        }

        // Check if image exists and belongs to the current user
        std::string error_message;
        if (!isImageOwner(image_id, user_id, error_message)) {
            if (error_message == "Image not found") {
                return crow::response(404, utils::JsonUtils::createErrorResponse("RESOURCE_NOT_FOUND", error_message));
            } else {
                return crow::response(403, utils::JsonUtils::createErrorResponse("FORBIDDEN", error_message));
            }
        }

        // Start transaction to ensure atomicity
        image_item_dao_.beginTransaction();

        try {
            // Delete all tags for this image
            bool tags_deleted = image_tag_dao_.deleteImageTagsByImageId(image_id);
            if (!tags_deleted) {
                image_item_dao_.rollbackTransaction();
                return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", "Failed to delete image tags"));
            }

            // Delete all likes for this image
            bool likes_deleted = image_like_dao_.deleteImageLikesByImageId(image_id);
            if (!likes_deleted) {
                image_item_dao_.rollbackTransaction();
                return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", "Failed to delete image likes"));
            }

            // Delete the image itself
            bool image_deleted = image_item_dao_.deleteImageItem(image_id);
            if (!image_deleted) {
                image_item_dao_.rollbackTransaction();
                return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", "Failed to delete image"));
            }

            // Commit transaction
            image_item_dao_.commitTransaction();

            return crow::response(204);

        } catch (const std::exception& e) {
            // Rollback transaction on error
            image_item_dao_.rollbackTransaction();
            throw;
        }

    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

crow::response ImageController::searchPublicImages(const crow::request& req) {
    try {
        // Get search parameters from query string
        std::string keyword;
        if (req.url_params.get("keyword")) {
            keyword = req.url_params.get("keyword");
        }

        std::string tag;
        if (req.url_params.get("tag")) {
            tag = req.url_params.get("tag");
        }

        std::string owner;
        if (req.url_params.get("owner")) {
            owner = req.url_params.get("owner");
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

        // Search public images with the given parameters
        std::vector<models::ImageItem> images = image_item_dao_.searchPublicImageItems(
            keyword, tag, owner, offset, page_size
        );

        // Get total number of images matching the search criteria
        int total_images = image_item_dao_.getPublicImageItemCount(keyword, tag, owner);

        // Prepare response
        nlohmann::json response_body;
        response_body["images"] = nlohmann::json::array();

        for (const auto& image : images) {
            nlohmann::json image_json = image.toJson();

            // Get tags for this image
            std::vector<models::Tag> tags = image_tag_dao_.getTagsByImageId(image.getId());
            nlohmann::json tags_json = nlohmann::json::array();
            for (const auto& tag : tags) {
                tags_json.push_back(tag.toJson());
            }
            image_json["tags"] = tags_json;

            // Add like count to image JSON
            addImageCountToImageJson(image_json, image.getId());

            response_body["images"].push_back(image_json);
        }

        // Add pagination information
        response_body["pagination"] = {
            {"page", page},
            {"page_size", page_size},
            {"total_images", total_images},
            {"total_pages", (total_images + page_size - 1) / page_size}
        };

        return crow::response(200, response_body.dump());

    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

crow::response ImageController::getPopularPublicImages(const crow::request& req) {
    try {
        // Get limit parameter from query string (default to 20)
        int limit = 20;
        const int max_limit = 100;

        if (req.url_params.get("limit")) {
            limit = std::stoi(req.url_params.get("limit"));
            if (limit < 1) {
                limit = 1;
            } else if (limit > max_limit) {
                limit = max_limit;
            }
        }

        // Get popular public images
        std::vector<models::ImageItem> images = image_item_dao_.getPopularPublicImageItems(limit);

        // Prepare response
        nlohmann::json response_body;
        response_body["images"] = nlohmann::json::array();

        for (const auto& image : images) {
            nlohmann::json image_json = image.toJson();

            // Get tags for this image
            std::vector<models::Tag> tags = image_tag_dao_.getTagsByImageId(image.getId());
            nlohmann::json tags_json = nlohmann::json::array();
            for (const auto& tag : tags) {
                tags_json.push_back(tag.toJson());
            }
            image_json["tags"] = tags_json;

            // Add like count to image JSON
            addImageCountToImageJson(image_json, image.getId());

            response_body["images"].push_back(image_json);
        }

        return crow::response(200, response_body.dump());

    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

crow::response ImageController::likeImage(const crow::request& req [[maybe_unused]], const std::string& token, int image_id) {
    try {
        // Verify token and get user ID
        int user_id = utils::AuthUtils::verifyTokenAndGetUserId(token);
        if (user_id == 0) {
            return crow::response(401, utils::JsonUtils::createErrorResponse("UNAUTHORIZED", "Invalid or expired token"));
        }

        // Check if image exists and is accessible to the current user
        std::string error_message;
        if (!isImageAccessible(image_id, user_id, error_message)) {
            if (error_message == "Image not found") {
                return crow::response(404, utils::JsonUtils::createErrorResponse("RESOURCE_NOT_FOUND", error_message));
            } else {
                return crow::response(403, utils::JsonUtils::createErrorResponse("FORBIDDEN", error_message));
            }
        }

        // Check if user has already liked this image
        models::ImageLike existing_like = image_like_dao_.getImageLikeByImageIdAndUserId(image_id, user_id);
        if (existing_like.getId() != 0) {
            return crow::response(400, utils::JsonUtils::createErrorResponse("BAD_REQUEST", "You have already liked this image"));
        }

        // Create new image like object
        models::ImageLike new_like;
        new_like.setImageId(image_id);
        new_like.setUserId(user_id);

        // Set created_at timestamp
        std::time_t now = std::time(nullptr);
        char buf[20];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        new_like.setCreatedAt(buf);

        // Save image like to database
        bool created = image_like_dao_.createImageLike(new_like);
        if (!created) {
            return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", "Failed to like image"));
        }

        // Get the created image like (with ID)
        models::ImageLike created_like = image_like_dao_.getImageLikeByImageIdAndUserId(image_id, user_id);

        // Prepare response
        nlohmann::json response_body = created_like.toJson();

        return crow::response(201, response_body.dump());

    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

crow::response ImageController::unlikeImage(const crow::request& req [[maybe_unused]], const std::string& token, int image_id) {
    try {
        // Verify token and get user ID
        int user_id = utils::AuthUtils::verifyTokenAndGetUserId(token);
        if (user_id == 0) {
            return crow::response(401, utils::JsonUtils::createErrorResponse("UNAUTHORIZED", "Invalid or expired token"));
        }

        // Check if user has liked this image
        models::ImageLike existing_like = image_like_dao_.getImageLikeByImageIdAndUserId(image_id, user_id);
        if (existing_like.getId() == 0) {
            return crow::response(404, utils::JsonUtils::createErrorResponse("RESOURCE_NOT_FOUND", "You have not liked this image"));
        }

        // Delete the image like
        bool deleted = image_like_dao_.deleteImageLike(existing_like.getId());
        if (!deleted) {
            return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", "Failed to unlike image"));
        }

        return crow::response(204);

    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

crow::response ImageController::getImageLikes(const crow::request& req [[maybe_unused]], const std::string& token, int image_id) {
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

        // Check if image exists and is accessible to the current user
        std::string error_message;
        if (!isImageAccessible(image_id, user_id, error_message)) {
            if (error_message == "Image not found") {
                return crow::response(404, utils::JsonUtils::createErrorResponse("RESOURCE_NOT_FOUND", error_message));
            } else {
                return crow::response(403, utils::JsonUtils::createErrorResponse("FORBIDDEN", error_message));
            }
        }

        // Get like count for this image
        int like_count = image_like_dao_.getImageLikeCount(image_id);

        // Get recent likes for this image (limit to 10 users)
        std::vector<models::ImageLike> recent_likes = image_like_dao_.getRecentImageLikes(image_id, 10);

        // Prepare response
        nlohmann::json response_body;
        response_body["like_count"] = like_count;
        response_body["recent_likes"] = nlohmann::json::array();

        for (const auto& like : recent_likes) {
            // Get user information for this like
            models::User user = user_dao_.getUserById(like.getUserId());
            if (user.getId() != 0) {
                nlohmann::json like_json;
                like_json["user_id"] = user.getId();
                like_json["username"] = user.getUsername();
                like_json["liked_at"] = like.getCreatedAt();
                response_body["recent_likes"].push_back(like_json);
            }
        }

        return crow::response(200, response_body.dump());

    } catch (const std::exception& e) {
        return crow::response(500, utils::JsonUtils::createErrorResponse("INTERNAL_SERVER_ERROR", e.what()));
    }
}

} // namespace controllers
