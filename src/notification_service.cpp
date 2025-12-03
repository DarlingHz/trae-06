#include "notification_service.h"
#include "database.h"
#include <stdexcept>

bool NotificationService::create_notification(int user_id, const std::string& message, const std::string& type) {
    std::string sql = "INSERT INTO notifications (user_id, message, type) VALUES (";
    sql += std::to_string(user_id) + ",'" + message + "','" + type + "');";
    
    return Database::instance().execute_update(sql);
}

std::vector<NotificationDTO> NotificationService::get_notifications(int user_id, bool mark_as_read) {
    std::vector<NotificationDTO> notifications;
    
    std::string sql = "SELECT id, user_id, message, type, is_read, created_at FROM notifications WHERE user_id = " + std::to_string(user_id) + " ORDER BY created_at DESC;";
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        NotificationDTO notification;
        notification.id = sqlite3_column_int(stmt, 0);
        notification.user_id = sqlite3_column_int(stmt, 1);
        notification.message = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        notification.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        notification.is_read = sqlite3_column_int(stmt, 4) == 1;
        notification.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        
        notifications.push_back(notification);
        return 0;
    });
    
    // 标记为已读
    if (mark_as_read && !notifications.empty()) {
        mark_notifications_as_read(user_id);
    }
    
    return notifications;
}

bool NotificationService::mark_notifications_as_read(int user_id) {
    std::string sql = "UPDATE notifications SET is_read = 1 WHERE user_id = " + std::to_string(user_id) + " AND is_read = 0;";
    
    return Database::instance().execute_update(sql);
}