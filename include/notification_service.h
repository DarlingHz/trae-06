#ifndef NOTIFICATION_SERVICE_H
#define NOTIFICATION_SERVICE_H

#include <vector>
#include <string>
#include "dto.h"

class NotificationService {
private:
    NotificationService() = default;
    
public:
    static NotificationService& instance() {
        static NotificationService instance;
        return instance;
    }
    
    bool create_notification(int user_id, const std::string& message, const std::string& type);
    std::vector<NotificationDTO> get_notifications(int user_id, bool mark_as_read = true);
    bool mark_notifications_as_read(int user_id);
};

#endif // NOTIFICATION_SERVICE_H