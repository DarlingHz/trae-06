#include "daos/ImageLikeDAO.hpp"
#include <stdexcept>

namespace daos {

bool ImageLikeDAO::createImageLike(const models::ImageLike& image_like) {
    std::string sql = "INSERT INTO image_likes (image_id, user_id, created_at) VALUES (?, ?, ?)";
    std::vector<std::string> params = {
        std::to_string(image_like.getImageId()),
        std::to_string(image_like.getUserId()),
        image_like.getCreatedAt()
    };

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create image like: " + std::string(e.what()));
    }
}

models::ImageLike ImageLikeDAO::getImageLikeById(int id) {
    std::string sql = "SELECT * FROM image_likes WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};

    try {
        auto result = db_.fetchWithParams(sql, params);
        if (result.empty()) {
            return models::ImageLike();
        }
        return rowToImageLike(result[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get image like by ID: " + std::string(e.what()));
    }
}

models::ImageLike ImageLikeDAO::getImageLikeByImageIdAndUserId(int image_id, int user_id) {
    std::string sql = "SELECT * FROM image_likes WHERE image_id = ? AND user_id = ?";
    std::vector<std::string> params = {
        std::to_string(image_id),
        std::to_string(user_id)
    };

    try {
        auto result = db_.fetchWithParams(sql, params);
        if (result.empty()) {
            return models::ImageLike();
        }
        return rowToImageLike(result[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get image like by image ID and user ID: " + std::string(e.what()));
    }
}

std::vector<models::ImageLike> ImageLikeDAO::getImageLikesByImageId(int image_id) {
    std::string sql = "SELECT * FROM image_likes WHERE image_id = ?";
    std::vector<std::string> params = {std::to_string(image_id)};

    try {
        auto result = db_.fetchWithParams(sql, params);
        std::vector<models::ImageLike> image_likes;
        for (const auto& row : result) {
            image_likes.push_back(rowToImageLike(row));
        }
        return image_likes;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get image likes by image ID: " + std::string(e.what()));
    }
}

std::vector<models::ImageLike> ImageLikeDAO::getImageLikesByUserId(int user_id) {
    std::string sql = "SELECT * FROM image_likes WHERE user_id = ?";
    std::vector<std::string> params = {std::to_string(user_id)};

    try {
        auto result = db_.fetchWithParams(sql, params);
        std::vector<models::ImageLike> image_likes;
        for (const auto& row : result) {
            image_likes.push_back(rowToImageLike(row));
        }
        return image_likes;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get image likes by user ID: " + std::string(e.what()));
    }
}

bool ImageLikeDAO::deleteImageLike(int id) {
    std::string sql = "DELETE FROM image_likes WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to delete image like: " + std::string(e.what()));
    }
}

bool ImageLikeDAO::deleteImageLikeByImageIdAndUserId(int image_id, int user_id) {
    std::string sql = "DELETE FROM image_likes WHERE image_id = ? AND user_id = ?";
    std::vector<std::string> params = {
        std::to_string(image_id),
        std::to_string(user_id)
    };

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to delete image like by image ID and user ID: " + std::string(e.what()));
    }
}

bool ImageLikeDAO::deleteImageLikesByImageId(int image_id) {
    std::string sql = "DELETE FROM image_likes WHERE image_id = ?";
    std::vector<std::string> params = {std::to_string(image_id)};

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to delete image likes by image ID: " + std::string(e.what()));
    }
}

bool ImageLikeDAO::deleteImageLikesByUserId(int user_id) {
    std::string sql = "DELETE FROM image_likes WHERE user_id = ?";
    std::vector<std::string> params = {std::to_string(user_id)};

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to delete image likes by user ID: " + std::string(e.what()));
    }
}

int ImageLikeDAO::getImageLikeCount(int image_id) {
    std::string sql = "SELECT COUNT(*) AS like_count FROM image_likes WHERE image_id = ?";
    std::vector<std::string> params = {std::to_string(image_id)};

    try {
        auto result = db_.fetchWithParams(sql, params);
        if (result.empty() || result[0].find("like_count") == result[0].end()) {
            return 0;
        }
        return std::stoi(result[0]["like_count"]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get image like count: " + std::string(e.what()));
    }
}

std::vector<models::ImageLike> ImageLikeDAO::getRecentImageLikes(int image_id, int limit) {
    std::string sql = "SELECT * FROM image_likes WHERE image_id = ? ORDER BY created_at DESC LIMIT ?";
    std::vector<std::string> params = {std::to_string(image_id), std::to_string(limit)};

    try {
        auto result = db_.fetchWithParams(sql, params);
        std::vector<models::ImageLike> image_likes;
        for (const auto& row : result) {
            image_likes.push_back(rowToImageLike(row));
        }
        return image_likes;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get recent image likes: " + std::string(e.what()));
    }
}

models::ImageLike ImageLikeDAO::rowToImageLike(const std::map<std::string, std::string>& row) {
    models::ImageLike image_like;

    auto it = row.find("id");
    if (it != row.end()) {
        image_like.setId(std::stoi(it->second));
    }

    it = row.find("image_id");
    if (it != row.end()) {
        image_like.setImageId(std::stoi(it->second));
    }

    it = row.find("user_id");
    if (it != row.end()) {
        image_like.setUserId(std::stoi(it->second));
    }

    it = row.find("created_at");
    if (it != row.end()) {
        image_like.setCreatedAt(it->second);
    }

    return image_like;
}

} // namespace daos
