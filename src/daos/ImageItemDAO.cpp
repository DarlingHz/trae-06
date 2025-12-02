#include "daos/ImageItemDAO.hpp"
#include <stdexcept>
#include <sstream>

namespace daos {

bool ImageItemDAO::createImageItem(const models::ImageItem& image_item) {
    std::string sql = "INSERT INTO image_items (album_id, owner_id, image_url, title, description, source_page_url, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)";
    std::vector<std::string> params = {
        std::to_string(image_item.getAlbumId()),
        std::to_string(image_item.getOwnerId()),
        image_item.getImageUrl(),
        image_item.getTitle(),
        image_item.getDescription(),
        image_item.getSourcePageUrl(),
        image_item.getCreatedAt()
    };

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create image item: " + std::string(e.what()));
    }
}

models::ImageItem ImageItemDAO::getImageItemById(int id) {
    std::string sql = "SELECT * FROM image_items WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};

    try {
        auto result = db_.fetchWithParams(sql, params);
        if (result.empty()) {
            return models::ImageItem();
        }
        return rowToImageItem(result[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get image item by ID: " + std::string(e.what()));
    }
}

std::vector<models::ImageItem> ImageItemDAO::getImageItemsByAlbumId(int album_id, int page, int page_size, const std::string& tag) {
    std::stringstream sql;
    std::vector<std::string> params;

    if (tag.empty()) {
        sql << "SELECT * FROM image_items WHERE album_id = ? ORDER BY created_at DESC LIMIT ? OFFSET ?";
        params = {
            std::to_string(album_id),
            std::to_string(page_size),
            std::to_string((page - 1) * page_size)
        };
    } else {
        sql << "SELECT DISTINCT ii.* FROM image_items ii JOIN image_tags it ON ii.id = it.image_id JOIN tags t ON it.tag_id = t.id WHERE ii.album_id = ? AND t.name = ? ORDER BY ii.created_at DESC LIMIT ? OFFSET ?";
        params = {
            std::to_string(album_id),
            tag,
            std::to_string(page_size),
            std::to_string((page - 1) * page_size)
        };
    }

    try {
        auto result = db_.fetchWithParams(sql.str(), params);
        std::vector<models::ImageItem> image_items;
        for (const auto& row : result) {
            image_items.push_back(rowToImageItem(row));
        }
        return image_items;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get image items by album ID: " + std::string(e.what()));
    }
}

std::vector<models::ImageItem> ImageItemDAO::getImageItemsByOwnerId(int owner_id, int page, int page_size) {
    std::string sql = "SELECT * FROM image_items WHERE owner_id = ? ORDER BY created_at DESC LIMIT ? OFFSET ?";
    std::vector<std::string> params = {
        std::to_string(owner_id),
        std::to_string(page_size),
        std::to_string((page - 1) * page_size)
    };

    try {
        auto result = db_.fetchWithParams(sql, params);
        std::vector<models::ImageItem> image_items;
        for (const auto& row : result) {
            image_items.push_back(rowToImageItem(row));
        }
        return image_items;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get image items by owner ID: " + std::string(e.what()));
    }
}

bool ImageItemDAO::updateImageItem(const models::ImageItem& image_item) {
    std::string sql = "UPDATE image_items SET album_id = ?, owner_id = ?, image_url = ?, title = ?, description = ?, source_page_url = ? WHERE id = ?";
    std::vector<std::string> params = {
        std::to_string(image_item.getAlbumId()),
        std::to_string(image_item.getOwnerId()),
        image_item.getImageUrl(),
        image_item.getTitle(),
        image_item.getDescription(),
        image_item.getSourcePageUrl(),
        std::to_string(image_item.getId())
    };

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to update image item: " + std::string(e.what()));
    }
}

bool ImageItemDAO::deleteImageItem(int id) {
    std::string sql = "DELETE FROM image_items WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to delete image item: " + std::string(e.what()));
    }
}

std::vector<models::ImageItem> ImageItemDAO::searchPublicImageItems(const std::string& keyword, const std::string& tag, const std::string& owner, int page, int page_size) {
    std::stringstream sql;
    std::vector<std::string> params;

    sql << "SELECT DISTINCT ii.* FROM image_items ii JOIN albums a ON ii.album_id = a.id JOIN users u ON ii.owner_id = u.id LEFT JOIN image_tags it ON ii.id = it.image_id LEFT JOIN tags t ON it.tag_id = t.id WHERE a.visibility = 'public'";

    if (!keyword.empty()) {
        sql << " AND (ii.title LIKE ? OR ii.description LIKE ?)";
        params.push_back("%" + keyword + "%");
        params.push_back("%" + keyword + "%");
    }

    if (!tag.empty()) {
        sql << " AND t.name = ?";
        params.push_back(tag);
    }

    if (!owner.empty()) {
        sql << " AND u.username = ?";
        params.push_back(owner);
    }

    sql << " ORDER BY ii.created_at DESC LIMIT ? OFFSET ?";
    params.push_back(std::to_string(page_size));
    params.push_back(std::to_string((page - 1) * page_size));

    try {
        auto result = db_.fetchWithParams(sql.str(), params);
        std::vector<models::ImageItem> image_items;
        for (const auto& row : result) {
            image_items.push_back(rowToImageItem(row));
        }
        return image_items;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to search public image items: " + std::string(e.what()));
    }
}

std::vector<models::ImageItem> ImageItemDAO::getPopularPublicImageItems(int limit) {
    std::string sql = "SELECT ii.*, COUNT(il.id) AS like_count FROM image_items ii JOIN albums a ON ii.album_id = a.id LEFT JOIN image_likes il ON ii.id = il.image_id WHERE a.visibility = 'public' GROUP BY ii.id ORDER BY like_count DESC LIMIT ?";
    std::vector<std::string> params = {std::to_string(limit)};

    try {
        auto result = db_.fetchWithParams(sql, params);
        std::vector<models::ImageItem> image_items;
        for (const auto& row : result) {
            image_items.push_back(rowToImageItem(row));
        }
        return image_items;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get popular public image items: " + std::string(e.what()));
    }
}

int ImageItemDAO::getPublicImageItemCount(const std::string& keyword, const std::string& tag, const std::string& owner) {
    std::stringstream sql;
    std::vector<std::string> params;

    sql << "SELECT COUNT(DISTINCT ii.id) FROM image_items ii JOIN albums a ON ii.album_id = a.id JOIN users u ON ii.owner_id = u.id LEFT JOIN image_tags it ON ii.id = it.image_id LEFT JOIN tags t ON it.tag_id = t.id WHERE a.visibility = 'public'";

    if (!keyword.empty()) {
        sql << " AND (ii.title LIKE ? OR ii.description LIKE ?)";
        params.push_back("%" + keyword + "%");
        params.push_back("%" + keyword + "%");
    }

    if (!tag.empty()) {
        sql << " AND t.name = ?";
        params.push_back(tag);
    }

    if (!owner.empty()) {
        sql << " AND u.username = ?";
        params.push_back(owner);
    }

    try {
        auto result = db_.fetchWithParams(sql.str(), params);
        if (result.empty() || result[0].empty()) {
            return 0;
        }
        return std::stoi(result[0]["COUNT(DISTINCT ii.id)"]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get public image item count: " + std::string(e.what()));
    }
}

models::ImageItem ImageItemDAO::rowToImageItem(const std::map<std::string, std::string>& row) {
    models::ImageItem image_item;

    auto it = row.find("id");
    if (it != row.end()) {
        image_item.setId(std::stoi(it->second));
    }

    it = row.find("album_id");
    if (it != row.end()) {
        image_item.setAlbumId(std::stoi(it->second));
    }

    it = row.find("owner_id");
    if (it != row.end()) {
        image_item.setOwnerId(std::stoi(it->second));
    }

    it = row.find("image_url");
    if (it != row.end()) {
        image_item.setImageUrl(it->second);
    }

    it = row.find("title");
    if (it != row.end()) {
        image_item.setTitle(it->second);
    }

    it = row.find("description");
    if (it != row.end()) {
        image_item.setDescription(it->second);
    }

    it = row.find("source_page_url");
    if (it != row.end()) {
        image_item.setSourcePageUrl(it->second);
    }

    it = row.find("created_at");
    if (it != row.end()) {
        image_item.setCreatedAt(it->second);
    }

    return image_item;
}

int ImageItemDAO::getImageItemCountByAlbumId(int album_id, const std::string& tag) {
    std::stringstream sql;
    std::vector<std::string> params;

    if (tag.empty()) {
        sql << "SELECT COUNT(*) FROM image_items WHERE album_id = ?";
        params = {std::to_string(album_id)};
    } else {
        sql << "SELECT COUNT(DISTINCT ii.id) FROM image_items ii JOIN image_tags it ON ii.id = it.image_id JOIN tags t ON it.tag_id = t.id WHERE ii.album_id = ? AND t.name = ?";
        params = {std::to_string(album_id), tag};
    }

    try {
        auto result = db_.fetchWithParams(sql.str(), params);
        if (result.empty() || result[0].empty()) {
            return 0;
        }
        if (tag.empty()) {
            return std::stoi(result[0]["COUNT(*)"]);
        } else {
            return std::stoi(result[0]["COUNT(DISTINCT ii.id)"]);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get image item count by album ID: " + std::string(e.what()));
    }
}

bool ImageItemDAO::deleteImageItemsByAlbumId(int album_id) {
    std::string sql = "DELETE FROM image_items WHERE album_id = ?";
    std::vector<std::string> params = {std::to_string(album_id)};

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to delete image items by album ID: " + std::string(e.what()));
    }
}

bool ImageItemDAO::beginTransaction() {
    return db_.beginTransaction();
}

bool ImageItemDAO::commitTransaction() {
    return db_.commitTransaction();
}

bool ImageItemDAO::rollbackTransaction() {
    return db_.rollbackTransaction();
}

} // namespace daos
