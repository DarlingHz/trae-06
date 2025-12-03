#include "Server.h"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include "models/Bookmark.h"

using json = nlohmann::json;

namespace http {

Server::Server(std::shared_ptr<service::UserService> user_service,
               std::shared_ptr<service::BookmarkService> bookmark_service,
               std::shared_ptr<auth::JWT> jwt,
               int port)
    : user_service_(std::move(user_service)),
      bookmark_service_(std::move(bookmark_service)),
      jwt_(std::move(jwt)),
      port_(port) {
    setup_routes();
}

void Server::send_success_response(httplib::Response& res, const json& data) const {
    res.set_content(data.dump(), "application/json");
    res.status = 200;
}

void Server::send_error_response(httplib::Response& res, int status, const std::string& message) const {
    json error = {
        {"code", status},
        {"message", message}
    };
    res.set_content(error.dump(), "application/json");
    res.status = status;
}

std::optional<int> Server::get_current_user_id(const httplib::Request& req) const {
    auto it = req.headers.find("Authorization");
    if(it == req.headers.end()) {
        return std::nullopt;
    }

    std::string auth_header = it->second;
    if(auth_header.substr(0, 7) != "Bearer ") {
        return std::nullopt;
    }

    std::string token = auth_header.substr(7);
    return jwt_->validate_token(token);
}

void Server::require_auth(const httplib::Request& req, httplib::Response& res, std::function<void(int)> handler) {
    auto user_id = get_current_user_id(req);
    if(!user_id) {
        send_error_response(res, 401, "Authentication required");
        return;
    }

    try {
        handler(*user_id);
    } catch(const std::exception& e) {
        send_error_response(res, 500, e.what());
    }
}

void Server::setup_user_routes() {
    // User registration
    server_.Post("/api/users/register", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            service::UserRegisterRequest request;
            if(!parse_request_body(req, request)) {
                send_error_response(res, 400, "Invalid request body");
                return;
            }

            auto user = user_service_->register_user(request);
            if(user) {
                json data = models::to_json(*user);
                data.erase("password_hash");
                nlohmann::json response_data; response_data["user"] = data; send_success_response(res, response_data);
            } else {
                send_error_response(res, 500, "Registration failed");
            }
        } catch(const std::invalid_argument& e) {
            send_error_response(res, 400, e.what());
        } catch(const std::runtime_error& e) {
            send_error_response(res, 409, e.what());
        } catch(const std::exception& e) {
            send_error_response(res, 500, e.what());
        }
    });

    // User login
    server_.Post("/api/users/login", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            service::UserLoginRequest request;
            if(!parse_request_body(req, request)) {
                send_error_response(res, 400, "Invalid request body");
                return;
            }

            auto response = user_service_->login_user(request);
            if(response) {
                json user_json = models::to_json(*response->user);
                user_json.erase("password_hash");
                nlohmann::json response_data; response_data["user"] = user_json; response_data["token"] = response->token; send_success_response(res, response_data);
            } else {
                send_error_response(res, 401, "Invalid email or password");
            }
        } catch(const std::exception& e) {
            send_error_response(res, 500, e.what());
        }
    });

    // Get current user
    server_.Get("/api/users/me", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &res](int user_id) {
            auto user = user_service_->get_user_by_id(user_id);
            if(user) {
                json data = models::to_json(*user);
                data.erase("password_hash");
                nlohmann::json response_data; response_data["user"] = data; send_success_response(res, response_data);
            } else {
                send_error_response(res, 404, "User not found");
            }
        });
    });
}

void Server::setup_bookmark_routes() {
    // Create bookmark
    server_.Post("/api/bookmarks", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            service::BookmarkCreateRequest request;
            if(!parse_request_body(req, request)) {
                send_error_response(res, 400, "Invalid request body");
                return;
            }

            auto bookmark = bookmark_service_->create_bookmark(user_id, request);
            if(bookmark) {
                nlohmann::json response_data; response_data["bookmark"] = models::to_json(*bookmark); send_success_response(res, response_data);
            } else {
                send_error_response(res, 500, "Create bookmark failed");
            }
        });
    });

    // Get single bookmark
    server_.Get("/api/bookmarks/(\\d+)", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            int id = std::stoi(req.matches[1]);
            auto bookmark = bookmark_service_->get_bookmark(id, user_id);

            if(bookmark) {
                nlohmann::json response_data; response_data["bookmark"] = models::to_json(*bookmark); send_success_response(res, response_data);
            } else {
                send_error_response(res, 404, "Bookmark not found");
            }
        });
    });

    // Update bookmark
    server_.Put("/api/bookmarks/(\\d+)", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            int id = std::stoi(req.matches[1]);
            service::BookmarkUpdateRequest request;

            if(!parse_request_body(req, request)) {
                send_error_response(res, 400, "Invalid request body");
                return;
            }

            bool success = bookmark_service_->update_bookmark(id, user_id, request);
            if(success) {
                nlohmann::json response_data; response_data["success"] = true; send_success_response(res, response_data);
            } else {
                send_error_response(res, 404, "Bookmark not found");
            }
        });
    });

    // Update read status
    server_.Put("/api/bookmarks/(\\d+)/read", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            int id = std::stoi(req.matches[1]);
            bool success = bookmark_service_->mark_as_read(id, user_id);

            if(success) {
                nlohmann::json response_data; response_data["success"] = true; send_success_response(res, response_data);
            } else {
                send_error_response(res, 404, "Bookmark not found");
            }
        });
    });

    // Delete bookmark
    server_.Delete("/api/bookmarks/(\\d+)", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            int id = std::stoi(req.matches[1]);
            bool success = bookmark_service_->delete_bookmark(id, user_id);

            if(success) {
                nlohmann::json response_data; response_data["success"] = true; send_success_response(res, response_data);
            } else {
                send_error_response(res, 404, "Bookmark not found");
            }
        });
    });

    // Get bookmarks list with pagination and filtering
    server_.Get("/api/bookmarks", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            service::BookmarkQueryRequest request;

            // Parse query parameters
            if(req.has_param("page")) {
                request.page = std::stoi(req.get_param_value("page"));
            }
            if(req.has_param("page_size")) {
                request.page_size = std::stoi(req.get_param_value("page_size"));
            }
            if(req.has_param("search")) {
                request.search_keyword = req.get_param_value("search");
            }
            if(req.has_param("tag")) {
                request.tags.push_back(req.get_param_value("tag"));
            }
            if(req.has_param("folder")) {
                request.folder = req.get_param_value("folder");
            }
            if(req.has_param("read_status")) {
                try {
                    request.read_status = models::Bookmark::read_status_from_string(req.get_param_value("read_status"));
                } catch(...) {
                    send_error_response(res, 400, "Invalid read_status");
                    return;
                }
            }
            if(req.has_param("is_favorite")) {
                request.is_favorite = (req.get_param_value("is_favorite") == "true");
            }
            if(req.has_param("sort_by")) {
                request.sort_by = req.get_param_value("sort_by");
            }
            if(req.has_param("sort_desc")) {
                request.sort_desc = (req.get_param_value("sort_desc") == "true");
            }

            auto result = bookmark_service_->query_bookmarks(user_id, request);

            json data = json({
                {"bookmarks", models::to_json(result.bookmarks)},
                {"total", result.total},
                {"page", result.page},
                {"page_size", result.page_size},
                {"total_pages", result.total_pages}
            });

            send_success_response(res, data);
        });
    });

    // Batch operations
    server_.Post("/api/bookmarks/batch/read", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            service::BatchUpdateRequest request;
            if(!parse_request_body(req, request)) {
                send_error_response(res, 400, "Invalid request body");
                return;
            }

            bool success = bookmark_service_->batch_update_read_status(user_id, request, models::ReadStatus::READ);
            nlohmann::json response_data; response_data["success"] = success; send_success_response(res, response_data);
        });
    });

    server_.Post("/api/bookmarks/batch/unread", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            service::BatchUpdateRequest request;
            if(!parse_request_body(req, request)) {
                send_error_response(res, 400, "Invalid request body");
                return;
            }

            bool success = bookmark_service_->batch_update_read_status(user_id, request, models::ReadStatus::UNREAD);
            nlohmann::json response_data; response_data["success"] = success; send_success_response(res, response_data);
        });
    });

    server_.Post("/api/bookmarks/batch/move", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            service::BatchMoveRequest request;
            if(!parse_request_body(req, request)) {
                send_error_response(res, 400, "Invalid request body");
                return;
            }

            bool success = bookmark_service_->batch_move_to_folder(user_id, request);
            nlohmann::json response_data; response_data["success"] = success; send_success_response(res, response_data);
        });
    });

    server_.Delete("/api/bookmarks/batch", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            service::BatchUpdateRequest request;
            if(!parse_request_body(req, request)) {
                send_error_response(res, 400, "Invalid request body");
                return;
            }

            bool success = bookmark_service_->batch_delete(user_id, request);
            nlohmann::json response_data; response_data["success"] = success; send_success_response(res, response_data);
        });
    });

    // Record click
    server_.Post("/api/bookmarks/(\\d+)/click", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            int id = std::stoi(req.matches[1]);
            bool success = bookmark_service_->record_click(id, user_id);

            if(success) {
                nlohmann::json response_data; response_data["success"] = true; send_success_response(res, response_data);
            } else {
                send_error_response(res, 404, "Bookmark not found");
            }
        });
    });
}

void Server::setup_stats_routes() {
    // User stats
    server_.Get("/api/stats", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &res](int user_id) {
            auto stats = bookmark_service_->get_user_stats(user_id);
            nlohmann::json response_data; response_data["stats"] = models::to_json(stats); send_success_response(res, response_data);
        });
    });

    // Daily stats
    server_.Get("/api/stats/daily", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            int days = 14;
            if(req.has_param("days")) {
                days = std::stoi(req.get_param_value("days"));
            }

            auto stats = bookmark_service_->get_daily_stats(user_id, days);
            json data = json({
                {"daily_stats", models::to_json(stats)},
                {"days", days}
            });

            send_success_response(res, data);
        });
    });

    // Top domains
    server_.Get("/api/stats/domains", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            int limit = 10;
            if(req.has_param("limit")) {
                limit = std::stoi(req.get_param_value("limit"));
            }

            auto stats = bookmark_service_->get_top_domains(user_id, limit);
            nlohmann::json response_data; response_data["top_domains"] = models::to_json(stats); send_success_response(res, response_data);
        });
    });
}

void Server::setup_tag_routes() {
    // Get user tags
    server_.Get("/api/tags", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &res](int user_id) {
            auto tags = bookmark_service_->get_user_tags(user_id);
            nlohmann::json response_data; response_data["tags"] = models::to_json(tags); send_success_response(res, response_data);
        });
    });

    // Rename tag
    server_.Put("/api/tags", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            std::string old_tag, new_tag;
            try {
                json data = json::parse(req.body);
                old_tag = data["old_tag"];
                new_tag = data["new_tag"];
            } catch(...) {
                send_error_response(res, 400, "Invalid request body");
                return;
            }

            bool success = bookmark_service_->rename_tag(user_id, old_tag, new_tag);
            nlohmann::json response_data; response_data["success"] = success; send_success_response(res, response_data);
        });
    });

    // Delete tag
    server_.Delete("/api/tags", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            std::string tag;
            bool remove_from_bookmarks = false;
            try {
                json data = json::parse(req.body);
                tag = data["tag"];
                if(data.contains("remove_from_bookmarks")) {
                    remove_from_bookmarks = data["remove_from_bookmarks"];
                }
            } catch(...) {
                send_error_response(res, 400, "Invalid request body");
                return;
            }

            bool success = bookmark_service_->delete_tag(user_id, tag, remove_from_bookmarks);
            nlohmann::json response_data; response_data["success"] = success; send_success_response(res, response_data);
        });
    });
}

void Server::setup_folder_routes() {
    // Get user folders
    server_.Get("/api/folders", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &res](int user_id) {
            auto folders = bookmark_service_->get_user_folders(user_id);
            nlohmann::json response_data; response_data["folders"] = models::to_json(folders); send_success_response(res, response_data);
        });
    });

    // Rename folder
    server_.Put("/api/folders", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            std::string old_name, new_name;
            try {
                json data = json::parse(req.body);
                old_name = data["old_name"];
                new_name = data["new_name"];
            } catch(...) {
                send_error_response(res, 400, "Invalid request body");
                return;
            }

            bool success = bookmark_service_->rename_folder(user_id, old_name, new_name);
            nlohmann::json response_data; response_data["success"] = success; send_success_response(res, response_data);
        });
    });

    // Delete folder
    server_.Delete("/api/folders", [this](const httplib::Request& req, httplib::Response& res) {
        require_auth(req, res, [this, &req, &res](int user_id) {
            std::string folder_name;
            bool remove_bookmarks = false;
            try {
                json data = json::parse(req.body);
                folder_name = data["folder_name"];
                if(data.contains("remove_bookmarks")) {
                    remove_bookmarks = data["remove_bookmarks"];
                }
            } catch(...) {
                send_error_response(res, 400, "Invalid request body");
                return;
            }

            bool success = bookmark_service_->delete_folder(user_id, folder_name, remove_bookmarks);
            nlohmann::json response_data; response_data["success"] = success; send_success_response(res, response_data);
        });
    });
}

void Server::setup_routes() {
    setup_user_routes();
    setup_bookmark_routes();
    setup_stats_routes();
    setup_tag_routes();
    setup_folder_routes();

    // Health check
    server_.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok"})", "application/json");
        res.status = 200;
    });
}

void Server::start() {
    spdlog::info("Server starting on port {}", port_);
    server_.set_mount_point("/", "./public");
    server_.listen("0.0.0.0", port_);
}

} // namespace http