#pragma once

#include <cpprest/http_listener.h>
#include <memory>

#include "../services/user_service.h"
#include "../auth/auth.h"
#include "../http/response_util.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class UserController {
private:
    std::shared_ptr<UserService> user_service_;
    std::shared_ptr<AuthService> auth_service_;
    
    void handle_get_users(http_request message);
    void handle_get_user_by_id(http_request message);
    void handle_create_user(http_request message);
    void handle_update_user(http_request message);
    void handle_delete_user(http_request message);
    void handle_login(http_request message);
    void handle_refresh_token(http_request message);
    void handle_change_password(http_request message);
    void handle_reset_password(http_request message);
    void handle_get_current_user(http_request message);
    
public:
    UserController(std::shared_ptr<UserService> user_service, std::shared_ptr<AuthService> auth_service);
    ~UserController() = default;
    
    void register_routes(uri_builder& uri_builder, std::function<void(http_listener&)> add_route);
};
