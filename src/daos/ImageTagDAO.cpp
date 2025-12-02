#include "daos/ImageTagDAO.hpp"
#include "daos/TagDAO.hpp"
#include "daos/ImageItemDAO.hpp"
#include <stdexcept>

namespace daos {

bool ImageTagDAO::createImageTag(const models::ImageTag& image_tag) {
    std::string sql = "INSERT INTO image_tags (image_id, tag_id) VALUES (?, ?)";
    std::vector<std::string> params = {
        std::to_string(image_tag.getImageId()),
        std::to_string(image_tag.getTagId())
    };

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create image-tag relationship: " + std::string(e.what()));
    }
}

models::ImageTag ImageTagDAO::getImageTagById(int id) {
    std::string sql = "SELECT * FROM image_tags WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};

    try {
        auto result = db_.fetchWithParams(sql, params);
        if (result.empty()) {
            return models::ImageTag();
        }
        return rowToImageTag(result[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get image-tag relationship by ID: " + std::string(e.what()));
    }
}

std::vector<models::Tag> ImageTagDAO::getTagsByImageId(int image_id) {
    std::string sql = "SELECT t.* FROM tags t JOIN image_tags it ON t.id = it.tag_id WHERE it.image_id = ?";
    std::vector<std::string> params = {std::to_string(image_id)};

    try {
        auto result = db_.fetchWithParams(sql, params);
        std::vector<models::Tag> tags;
        TagDAO tag_dao(db_);
        for (const auto& row : result) {
            tags.push_back(tag_dao.rowToTag(row));
        }
        return tags;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get tags by image ID: " + std::string(e.what()));
    }
}

std::vector<models::ImageItem> ImageTagDAO::getImagesByTagId(int tag_id) {
    std::string sql = "SELECT ii.* FROM image_items ii JOIN image_tags it ON ii.id = it.image_id WHERE it.tag_id = ?";
    std::vector<std::string> params = {std::to_string(tag_id)};

    try {
        auto result = db_.fetchWithParams(sql, params);
        std::vector<models::ImageItem> image_items;
        ImageItemDAO image_item_dao(db_);
        for (const auto& row : result) {
            image_items.push_back(image_item_dao.rowToImageItem(row));
        }
        return image_items;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get images by tag ID: " + std::string(e.what()));
    }
}

bool ImageTagDAO::deleteImageTag(int id) {
    std::string sql = "DELETE FROM image_tags WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to delete image-tag relationship: " + std::string(e.what()));
    }
}

bool ImageTagDAO::deleteImageTagsByImageId(int image_id) {
    std::string sql = "DELETE FROM image_tags WHERE image_id = ?";
    std::vector<std::string> params = {std::to_string(image_id)};

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to delete image-tag relationships by image ID: " + std::string(e.what()));
    }
}

bool ImageTagDAO::deleteImageTagsByTagId(int tag_id) {
    std::string sql = "DELETE FROM image_tags WHERE tag_id = ?";
    std::vector<std::string> params = {std::to_string(tag_id)};

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to delete image-tag relationships by tag ID: " + std::string(e.what()));
    }
}

models::ImageTag ImageTagDAO::rowToImageTag(const std::map<std::string, std::string>& row) {
    models::ImageTag image_tag;

    auto it = row.find("id");
    if (it != row.end()) {
        image_tag.setId(std::stoi(it->second));
    }

    it = row.find("image_id");
    if (it != row.end()) {
        image_tag.setImageId(std::stoi(it->second));
    }

    it = row.find("tag_id");
    if (it != row.end()) {
        image_tag.setTagId(std::stoi(it->second));
    }

    return image_tag;
}

} // namespace daos
