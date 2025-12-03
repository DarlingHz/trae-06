#include "lost_item_service.h"
#include "database.h"
#include "logger.h"
#include <stdexcept>

std::optional<LostItemDTO> LostItemService::create_lost_item(const CreateLostItemRequest& request, int user_id) {
    std::string sql = "INSERT INTO lost_items (owner_user_id, title, description, category, lost_time, lost_location) VALUES (";
    sql += std::to_string(user_id) + ",'" + request.title + "','" + request.description + "','";
    sql += request.category + "','" + request.lost_time + "','" + request.lost_location + "');";
    
    int item_id = 0;
    if (!Database::instance().execute_update(sql, &item_id)) {
        return std::nullopt;
    }
    
    return get_lost_item_by_id(item_id);
}

std::vector<LostItemDTO> LostItemService::get_lost_items(int page, int limit,
                                                         const std::optional<std::string>& category,
                                                         const std::optional<std::string>& keyword,
                                                         const std::optional<std::string>& status) {
    std::vector<LostItemDTO> items;
    
    std::string sql = "SELECT id, owner_user_id, title, description, category, lost_time, lost_location, status, created_at, updated_at FROM lost_items WHERE 1=1";
    
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
        LostItemDTO item;
        item.id = sqlite3_column_int(stmt, 0);
        item.owner_user_id = sqlite3_column_int(stmt, 1);
        item.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        item.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        item.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        item.lost_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        item.lost_location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        item.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        item.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        item.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        
        items.push_back(item);
        return 0;
    });
    
    return items;
}

std::optional<LostItemDTO> LostItemService::get_lost_item_by_id(int id) {
    std::string sql = "SELECT id, owner_user_id, title, description, category, lost_time, lost_location, status, created_at, updated_at FROM lost_items WHERE id = " + std::to_string(id) + ";";
    
    LostItemDTO item;
    bool found = false;
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        item.id = sqlite3_column_int(stmt, 0);
        item.owner_user_id = sqlite3_column_int(stmt, 1);
        item.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        item.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        item.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        item.lost_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        item.lost_location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        item.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        item.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        item.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        
        found = true;
        return 1;
    });
    
    return found ? std::optional<LostItemDTO>(item) : std::nullopt;
}

bool LostItemService::update_lost_item(int id, const UpdateLostItemRequest& request, const UserDTO& user) {
    auto item = get_lost_item_by_id(id);
    if (!item.has_value()) {
        return false;
    }
    
    if (item->status == "closed" && user.role != "admin") {
        return false;
    }
    
    if (item->owner_user_id != user.id && user.role != "admin") {
        return false;
    }
    
    std::string sql = "UPDATE lost_items SET updated_at = CURRENT_TIMESTAMP";
    
    if (request.title.has_value()) {
        sql += ", title = '" + request.title.value() + "'";
    }
    if (request.description.has_value()) {
        sql += ", description = '" + request.description.value() + "'";
    }
    if (request.category.has_value()) {
        sql += ", category = '" + request.category.value() + "'";
    }
    if (request.lost_time.has_value()) {
        sql += ", lost_time = '" + request.lost_time.value() + "'";
    }
    if (request.lost_location.has_value()) {
        sql += ", lost_location = '" + request.lost_location.value() + "'";
    }
    if (request.status.has_value()) {
        sql += ", status = '" + request.status.value() + "'";
    }
    
    sql += " WHERE id = " + std::to_string(id) + ";";
    
    return Database::instance().execute_update(sql);
}

bool LostItemService::delete_lost_item(int id, const UserDTO& user) {
    auto item = get_lost_item_by_id(id);
    if (!item.has_value()) {
        return false;
    }
    
    if (item->owner_user_id != user.id && user.role != "admin") {
        return false;
    }
    
    std::string sql = "DELETE FROM lost_items WHERE id = " + std::to_string(id) + ";";
    return Database::instance().execute_update(sql);
}

int LostItemService::get_total_lost_items() {
    std::string sql = "SELECT COUNT(*) FROM lost_items;";
    int count = 0;
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        count = sqlite3_column_int(stmt, 0);
        return 1;
    });
    
    return count;
}

int LostItemService::get_open_lost_items_count() {
    std::string sql = "SELECT COUNT(*) FROM lost_items WHERE status = 'open';";
    int count = 0;
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        count = sqlite3_column_int(stmt, 0);
        return 1;
    });
    
    return count;
}

int LostItemService::get_lost_items_7d_count() {
    std::string sql = "SELECT COUNT(*) FROM lost_items WHERE created_at >= datetime('now', '-7 days');";
    int count = 0;
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        count = sqlite3_column_int(stmt, 0);
        return 1;
    });
    
    return count;
}