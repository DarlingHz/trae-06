#include "utils/DatabaseInitializer.hpp"
#include <iostream>

namespace utils {

DatabaseInitializer::DatabaseInitializer(Database& db) : db_(db) {
}

bool DatabaseInitializer::initialize() {
    std::cout << "Initializing database..." << std::endl;

    if (!createUsersTable()) {
        std::cerr << "Failed to create users table" << std::endl;
        return false;
    }

    if (!createAlbumsTable()) {
        std::cerr << "Failed to create albums table" << std::endl;
        return false;
    }

    if (!createImageItemsTable()) {
        std::cerr << "Failed to create image_items table" << std::endl;
        return false;
    }

    if (!createTagsTable()) {
        std::cerr << "Failed to create tags table" << std::endl;
        return false;
    }

    if (!createImageTagsTable()) {
        std::cerr << "Failed to create image_tags table" << std::endl;
        return false;
    }

    if (!createImageLikesTable()) {
        std::cerr << "Failed to create image_likes table" << std::endl;
        return false;
    }

    std::cout << "Database initialized successfully" << std::endl;
    return true;
}

bool DatabaseInitializer::createUsersTable() {
    const std::string query = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            email TEXT NOT NULL,
            password_hash TEXT NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    return db_.execute(query);
}

bool DatabaseInitializer::createAlbumsTable() {
    const std::string query = R"(
        CREATE TABLE IF NOT EXISTS albums (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            owner_id INTEGER NOT NULL,
            title TEXT NOT NULL,
            description TEXT,
            visibility TEXT NOT NULL DEFAULT 'private',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (owner_id) REFERENCES users(id) ON DELETE CASCADE
        );
    )";

    return db_.execute(query);
}

bool DatabaseInitializer::createImageItemsTable() {
    const std::string query = R"(
        CREATE TABLE IF NOT EXISTS image_items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            album_id INTEGER NOT NULL,
            owner_id INTEGER NOT NULL,
            image_url TEXT NOT NULL,
            title TEXT NOT NULL,
            description TEXT,
            source_page_url TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (album_id) REFERENCES albums(id) ON DELETE CASCADE,
            FOREIGN KEY (owner_id) REFERENCES users(id) ON DELETE CASCADE
        );
    )";

    return db_.execute(query);
}

bool DatabaseInitializer::createTagsTable() {
    const std::string query = R"(
        CREATE TABLE IF NOT EXISTS tags (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    return db_.execute(query);
}

bool DatabaseInitializer::createImageTagsTable() {
    const std::string query = R"(
        CREATE TABLE IF NOT EXISTS image_tags (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            image_id INTEGER NOT NULL,
            tag_id INTEGER NOT NULL,
            FOREIGN KEY (image_id) REFERENCES image_items(id) ON DELETE CASCADE,
            FOREIGN KEY (tag_id) REFERENCES tags(id) ON DELETE CASCADE,
            UNIQUE(image_id, tag_id)
        );
    )";

    return db_.execute(query);
}

bool DatabaseInitializer::createImageLikesTable() {
    const std::string query = R"(
        CREATE TABLE IF NOT EXISTS image_likes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            image_id INTEGER NOT NULL,
            user_id INTEGER NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (image_id) REFERENCES image_items(id) ON DELETE CASCADE,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            UNIQUE(image_id, user_id)
        );
    )";

    return db_.execute(query);
}

} // namespace utils
