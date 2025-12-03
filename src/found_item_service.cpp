#include "found_item_service.h"
#include "database.h"
#include "logger.h"
#include <stdexcept>

std::optional<FoundItemDTO> FoundItemService::create_found_item(const CreateFoundItemRequest& request, int user_id) {
    std::string sql = "INSERT INTO found_items (finder_user_id, title, description, category, found_time, found_location, keep_place) VALUES (";
    sql += std::to_string(user_id) + ",'" + request.title + "','" + request.description + "','";
    sql += request.category + "','" + request.found_time + "','" + request.found_location + "','";
    sql += request.keep_place + "');";
    
    int item_id = 0;
    if (!Database::instance().execute_update(sql, &item_id)) {
        return std::nullopt;
    }
    
    return get_found_item_by_id(item_id);
}

std::vector<FoundItemDTO> FoundItemService::get_found_items(int page, int limit,
                                                           const std::optional<std::string>& category,
                                                           const std::optional<std::string>& keyword,
                                                           const std::optional<std::string>& status) {
    std::vector<FoundItemDTO> items;
    
    std::string sql = "SELECT id, finder_user_id, title, description, category, found_time, found_location, keep_place, status, created_at, updated_at FROM found_items WHERE 1=1";
    
    if (category.has_value()) {
        sql += " AND category = '" + category.value() + "'";
    }
    
    if (status.has_value()) {
        sql += " AND status = '" + status.value() + "'";
    }
    
    if (keyword.has_value()) {
        sql += " AND (title LIKE '%" + keyword.value() + "%' OR description LIKE '%" + keyword.value() + "%')";
    }
    
    sql += " ORDER BY created_at DESC LIMIT " + std::to_string(limit) + " OFFSET " + std::to_string((page - 1) * limit) + ";";
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        FoundItemDTO item;
        item.id = sqlite3_column_int(stmt, 0);
        item.finder_user_id = sqlite3_column_int(stmt, 1);
        item.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        item.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        item.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        item.found_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        item.found_location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        item.keep_place = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        item.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        item.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        item.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        
        items.push_back(item);
        return 0;
    });
    
    return items;
}

std::optional<FoundItemDTO> FoundItemService::get_found_item_by_id(int id) {
    std::string sql = "SELECT id, finder_user_id, title, description, category, found_time, found_location, keep_place, status, created_at, updated_at FROM found_items WHERE id = " + std::to_string(id) + ";";
    
    FoundItemDTO item;
    bool found = false;
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        item.id = sqlite3_column_int(stmt, 0);
        item.finder_user_id = sqlite3_column_int(stmt, 1);
        item.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        item.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        item.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        item.found_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        item.found_location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        item.keep_place = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        item.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        item.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        item.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        
        found = true;
        return 1;
    });
    
    return found ? std::optional<FoundItemDTO>(item) : std::nullopt;
}

bool FoundItemService::update_found_item(int id, const UpdateFoundItemRequest& request, const UserDTO& user) {
    auto item = get_found_item_by_id(id);
    if (!item.has_value()) {
        return false;
    }
    
    if (item->status == "closed" && user.role != "admin") {
        return false;
    }
    
    if (item->finder_user_id != user.id && user.role != "admin") {
        return false;
    }
    
    std::string sql = "UPDATE found_items SET updated_at = CURRENT_TIMESTAMP";
    
    if (request.title.has_value()) {
        sql += ", title = '" + request.title.value() + "'";
    }
    if (request.description.has_value()) {
        sql += ", description = '" + request.description.value() + "'";
    }
    if (request.category.has_value()) {
        sql += ", category = '" + request.category.value() + "'";
    }
    if (request.found_time.has_value()) {
        sql += ", found_time = '" + request.found_time.value() + "'";
    }
    if (request.found_location.has_value()) {
        sql += ", found_location = '" + request.found_location.value() + "'";
    }
    if (request.keep_place.has_value()) {
        sql += ", keep_place = '" + request.keep_place.value() + "'";
    }
    if (request.status.has_value()) {
        sql += ", status = '" + request.status.value() + "'";
    }
    
    sql += " WHERE id = " + std::to_string(id) + ";";
    
    return Database::instance().execute_update(sql);
}

bool FoundItemService::delete_found_item(int id, const UserDTO& user) {
    auto item = get_found_item_by_id(id);
    if (!item.has_value()) {
        return false;
    }
    
    if (item->finder_user_id != user.id && user.role != "admin") {
        return false;
    }
    
    std::string sql = "DELETE FROM found_items WHERE id = " + std::to_string(id) + ";";
    return Database::instance().execute_update(sql);
}

int FoundItemService::get_total_found_items() {
    std::string sql = "SELECT COUNT(*) FROM found_items;";
    int count = 0;
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        count = sqlite3_column_int(stmt, 0);
        return 1;
    });
    
    return count;
}

int FoundItemService::get_open_found_items_count() {
    std::string sql = "SELECT COUNT(*) FROM found_items WHERE status = 'open';";
    int count = 0;
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        count = sqlite3_column_int(stmt, 0);
        return 1;
    });
    
    return count;
}

int FoundItemService::get_found_items_7d_count() {
    std::string sql = "SELECT COUNT(*) FROM found_items WHERE created_at >= datetime('now', '-7 days');";
    int count = 0;
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        count = sqlite3_column_int(stmt, 0);
        return 1;
    });
    
    return count;
}