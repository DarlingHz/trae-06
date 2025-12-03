#pragma once

#include <cpprest/http_listener.h>
#include <memory>

#include "../services/announcement_service.h"
#include "../services/read_receipt_service.h"
#include "../auth/auth.h"
#include "../http/response_util.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class AnnouncementController {
private:
    std::shared_ptr<AnnouncementService> announcement_service_;
    std::shared_ptr<ReadReceiptService> read_receipt_service_;
    std::shared_ptr<AuthService> auth_service_;
    
    void handle_get_announcements(http_request message);
    void handle_get_announcement_by_id(http_request message);
    void handle_create_announcement(http_request message);
    void handle_update_announcement(http_request message);
    void handle_delete_announcement(http_request message);
    void handle_publish_announcement(http_request message);
    void handle_unpublish_announcement(http_request message);
    void handle_get_unread_announcements(http_request message);
    void handle_get_read_announcements(http_request message);
    void handle_mark_as_read(http_request message);
    void handle_batch_mark_as_read(http_request message);
    void handle_get_announcement_statistics(http_request message);
    
    std::string get_current_user_id(http_request& message);
    
public:
    AnnouncementController(
        std::shared_ptr<AnnouncementService> announcement_service,
        std::shared_ptr<ReadReceiptService> read_receipt_service,
        std::shared_ptr<AuthService> auth_service
    );
    ~AnnouncementController() = default;
    
    void register_routes(uri_builder& uri_builder, std::function<void(http_listener&)> add_route);
};
