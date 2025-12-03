#pragma once

#include <cpprest/http_listener.h>
#include <cpprest/uri_builder.h>
#include <memory>
#include <vector>
#include <functional>

#include "../controller/user_controller.h"
#include "../controller/announcement_controller.h"
#include "../auth/middleware.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class Router {
private:
    std::vector<std::shared_ptr<http_listener>> listeners_;
    std::shared_ptr<AuthMiddleware> auth_middleware_;
    std::shared_ptr<UserController> user_controller_;
    std::shared_ptr<AnnouncementController> announcement_controller_;
    
    std::string base_url_;
    uint16_t port_;
    
    void setup_cors(http_request& message);
    void handle_options(http_request message);
    
    template <typename T>
    void apply_middleware(std::shared_ptr<T> listener,
                         const std::vector<std::function<void(http_request, std::function<void()>)>>& middlewares,
                         std::function<void(http_request)> handler);
    
public:
    Router(const std::string& base_url = "http://localhost", uint16_t port = 3000);
    ~Router();
    
    void set_auth_middleware(std::shared_ptr<AuthMiddleware> auth_middleware);
    void set_user_controller(std::shared_ptr<UserController> user_controller);
    void set_announcement_controller(std::shared_ptr<AnnouncementController> announcement_controller);
    
    void register_all_routes();
    pplx::task<void> start();
    pplx::task<void> stop();
    
    std::string get_base_url() const;
    uint16_t get_port() const;
};
