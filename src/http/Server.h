#pragma once

#include <httplib.h>
#include <memory>
#include <string>
#include "service/UserService.h"
#include "service/BookmarkService.h"
#include "auth/JWT.h"

namespace http {

class Server {
private:
    httplib::Server server_;
    std::shared_ptr<service::UserService> user_service_;
    std::shared_ptr<service::BookmarkService> bookmark_service_;
    std::shared_ptr<auth::JWT> jwt_;
    int port_;

    void setup_routes();
    void setup_user_routes();
    void setup_bookmark_routes();
    void setup_stats_routes();
    void setup_tag_routes();
    void setup_folder_routes();

    template<typename T>
    bool parse_request_body(const httplib::Request& req, T& data) const {
        try {
            if(req.body.empty()) {
                return false;
            }
            nlohmann::json::parse(req.body).get_to(data);
            return true;
        } catch(const std::exception&) {
            return false;
        }
    }

    void send_success_response(httplib::Response& res, const nlohmann::json& data) const;
    void send_error_response(httplib::Response& res, int status, const std::string& message) const;

    std::optional<int> get_current_user_id(const httplib::Request& req) const;
    void require_auth(const httplib::Request& req, httplib::Response& res, std::function<void(int)> handler);

public:
    Server(std::shared_ptr<service::UserService> user_service,
           std::shared_ptr<service::BookmarkService> bookmark_service,
           std::shared_ptr<auth::JWT> jwt,
           int port = 8080);

    void start();
};

} // namespace http
