#include "daos/AlbumDAO.hpp"
#include <stdexcept>

namespace daos {

bool AlbumDAO::createAlbum(const models::Album& album) {
    std::string sql = "INSERT INTO albums (owner_id, title, description, visibility, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?)";
    std::vector<std::string> params = {
        std::to_string(album.getOwnerId()),
        album.getTitle(),
        album.getDescription(),
        album.getVisibility(),
        album.getCreatedAt(),
        album.getUpdatedAt()
    };

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create album: " + std::string(e.what()));
    }
}

models::Album AlbumDAO::getAlbumById(int id) {
    std::string sql = "SELECT * FROM albums WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};

    try {
        auto result = db_.fetchWithParams(sql, params);
        if (result.empty()) {
            return models::Album();
        }
        return rowToAlbum(result[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get album by ID: " + std::string(e.what()));
    }
}

std::vector<models::Album> AlbumDAO::getAlbumsByOwnerId(int owner_id, int page, int page_size) {
    std::string sql = "SELECT * FROM albums WHERE owner_id = ? ORDER BY created_at DESC LIMIT ? OFFSET ?";
    int offset = (page - 1) * page_size;
    std::vector<std::string> params = {
        std::to_string(owner_id),
        std::to_string(page_size),
        std::to_string(offset)
    };

    try {
        auto result = db_.fetchWithParams(sql, params);
        std::vector<models::Album> albums;
        for (const auto& row : result) {
            albums.push_back(rowToAlbum(row));
        }
        return albums;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get albums by owner ID: " + std::string(e.what()));
    }
}

bool AlbumDAO::updateAlbum(const models::Album& album) {
    std::string sql = "UPDATE albums SET title = ?, description = ?, visibility = ?, updated_at = ? WHERE id = ?";
    std::vector<std::string> params = {
        album.getTitle(),
        album.getDescription(),
        album.getVisibility(),
        album.getUpdatedAt(),
        std::to_string(album.getId())
    };

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to update album: " + std::string(e.what()));
    }
}

bool AlbumDAO::deleteAlbum(int id) {
    std::string sql = "DELETE FROM albums WHERE id = ?";
    std::vector<std::string> params = {std::to_string(id)};

    try {
        return db_.executeWithParams(sql, params);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to delete album: " + std::string(e.what()));
    }
}

bool AlbumDAO::isAlbumAccessible(int album_id, int user_id, std::string& error_message) {
    try {
        // Get album by ID
        models::Album album = getAlbumById(album_id);
        if (album.getId() == 0) {
            error_message = "Album not found";
            return false;
        }

        // Check if album is public
        if (album.getVisibility() == "public") {
            return true;
        }

        // Check if user is album owner
        if (album.getOwnerId() == user_id) {
            return true;
        }

        // User does not have access to album
        error_message = "Forbidden";
        return false;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to check album accessibility: " + std::string(e.what()));
    }
}

bool AlbumDAO::beginTransaction() {
    try {
        return db_.beginTransaction();
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to begin transaction: " + std::string(e.what()));
    }
}

bool AlbumDAO::commitTransaction() {
    try {
        return db_.commitTransaction();
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to commit transaction: " + std::string(e.what()));
    }
}

bool AlbumDAO::rollbackTransaction() {
    try {
        return db_.rollbackTransaction();
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to rollback transaction: " + std::string(e.what()));
    }
}

int AlbumDAO::getAlbumCountByOwnerId(int owner_id) {
    std::string sql = "SELECT COUNT(*) FROM albums WHERE owner_id = ?";
    std::vector<std::string> params = {std::to_string(owner_id)};

    try {
        auto result = db_.fetchWithParams(sql, params);
        if (result.empty() || result[0].empty()) {
            return 0;
        }
        return std::stoi(result[0]["COUNT(*)"]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to get album count by owner ID: " + std::string(e.what()));
    }
}

models::Album AlbumDAO::rowToAlbum(const std::map<std::string, std::string>& row) {
    models::Album album;

    auto it = row.find("id");
    if (it != row.end()) {
        album.setId(std::stoi(it->second));
    }

    it = row.find("owner_id");
    if (it != row.end()) {
        album.setOwnerId(std::stoi(it->second));
    }

    it = row.find("title");
    if (it != row.end()) {
        album.setTitle(it->second);
    }

    it = row.find("description");
    if (it != row.end()) {
        album.setDescription(it->second);
    }

    it = row.find("visibility");
    if (it != row.end()) {
        album.setVisibility(it->second);
    }

    it = row.find("created_at");
    if (it != row.end()) {
        album.setCreatedAt(it->second);
    }

    it = row.find("updated_at");
    if (it != row.end()) {
        album.setUpdatedAt(it->second);
    }

    return album;
}

} // namespace daos
