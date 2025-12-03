#include "announcement_controller.h"
#include <cpprest/json.h>

AnnouncementController::AnnouncementController(
    std::shared_ptr<AnnouncementService> announcement_service,
    std::shared_ptr<ReadReceiptService> read_receipt_service,
    std::shared_ptr<AuthService> auth_service
) : announcement_service_(announcement_service), read_receipt_service_(read_receipt_service), auth_service_(auth_service) {}

std::string AnnouncementController::get_current_user_id(http_request& message) {
    auto token = message.headers().find(U("Authorization"));
    if (token == message.headers().end()) {
        throw InvalidTokenException("缺少授权令牌");
    }
    
    auto token_str = utility::conversions::to_utf8string(token->second);
    if (token_str.substr(0, 7) == "Bearer ") {
        token_str = token_str.substr(7);
    }
    
    auto token_info = auth_service_->verify_token(token_str);
    return token_info.user_id;
}

void AnnouncementController::handle_get_announcements(http_request message) {
    try {
        QueryParams params;
        
        if (!message.request_uri().query().empty()) {
            uri_builder ub(U("?") + message.request_uri().query());
            for (auto& query : ub.query()) {
                params[utility::conversions::to_utf8string(query.first)] = 
                    utility::conversions::to_utf8string(query.second);
            }
        }
        
        auto result = announcement_service_->get_announcements(params);
        message.reply(status_codes::OK, HttpResponseUtil::create_pagination_response("announcements", result));
    } catch (const AnnouncementNotFoundException& e) {
        message.reply(status_codes::NotFound, HttpResponseUtil::create_error_response(ErrorCode::ANNOUNCEMENT_NOT_FOUND, e.what()));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void AnnouncementController::handle_get_announcement_by_id(http_request message) {
    try {
        auto path = message.request_uri().path();
        auto id_str = path.substr(path.find_last_of('/') + 1);
        int64_t id = std::stoll(utility::conversions::to_utf8string(id_str));
        
        auto announcement = announcement_service_->get_announcement_by_id(id);
        message.reply(status_codes::OK, HttpResponseUtil::create_success_response("announcement", announcement.to_json()));
    } catch (const AnnouncementNotFoundException& e) {
        message.reply(status_codes::NotFound, HttpResponseUtil::create_error_response(ErrorCode::ANNOUNCEMENT_NOT_FOUND, e.what()));
    } catch (const std::invalid_argument& e) {
        message.reply(status_codes::BadRequest, HttpResponseUtil::create_error_response(ErrorCode::INVALID_PARAMS, "无效的公告ID"));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void AnnouncementController::handle_create_announcement(http_request message) {
    try {
        auto current_user_id = get_current_user_id(message);
        
        message.extract_json().then([this, current_user_id](pplx::task<json::value> task) {
            try {
                auto json = task.get();
                
                Announcement announcement;
                announcement.set_title(utility::conversions::to_utf8string(json[U("title")].as_string()));
                announcement.set_content(utility::conversions::to_utf8string(json[U("content")].as_string()));
                announcement.set_author_id(std::stoll(current_user_id));
                
                if (json.has_field(U("priority")) && !json[U("priority")].is_null()) {
                    announcement.set_priority(utility::conversions::to_utf8string(json[U("priority")].as_string()));
                }
                
                if (json.has_field(U("type")) && !json[U("type")].is_null()) {
                    announcement.set_type(utility::conversions::to_utf8string(json[U("type")].as_string()));
                }
                
                if (json.has_field(U("publish_time")) && !json[U("publish_time")].is_null()) {
                    announcement.set_publish_time(utility::conversions::to_utf8string(json[U("publish_time")].as_string()));
                }
                
                if (json.has_field(U("expire_time")) && !json[U("expire_time")].is_null()) {
                    announcement.set_expire_time(utility::conversions::to_utf8string(json[U("expire_time")].as_string()));
                }
                
                auto created_announcement = announcement_service_->create_announcement(announcement);
                message.reply(status_codes::Created, HttpResponseUtil::create_success_response("announcement", created_announcement.to_json()));
            } catch (const ValidationException& e) {
                message.reply(status_codes::BadRequest, HttpResponseUtil::create_validation_error_response(e.get_errors()));
            } catch (const std::exception& e) {
                message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
            }
        });
    } catch (const InvalidTokenException& e) {
        message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::UNAUTHORIZED, e.what()));
    } catch (const TokenExpiredException& e) {
        message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::TOKEN_EXPIRED, e.what()));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void AnnouncementController::handle_update_announcement(http_request message) {
    try {
        auto path = message.request_uri().path();
        auto id_str = path.substr(path.find_last_of('/') + 1);
        int64_t id = std::stoll(utility::conversions::to_utf8string(id_str));
        
        message.extract_json().then([this, id](pplx::task<json::value> task) {
            try {
                auto json = task.get();
                
                Announcement announcement;
                announcement.set_id(id);
                
                if (json.has_field(U("title")) && !json[U("title")].is_null()) {
                    announcement.set_title(utility::conversions::to_utf8string(json[U("title")].as_string()));
                }
                
                if (json.has_field(U("content")) && !json[U("content")].is_null()) {
                    announcement.set_content(utility::conversions::to_utf8string(json[U("content")].as_string()));
                }
                
                if (json.has_field(U("priority")) && !json[U("priority")].is_null()) {
                    announcement.set_priority(utility::conversions::to_utf8string(json[U("priority")].as_string()));
                }
                
                if (json.has_field(U("type")) && !json[U("type")].is_null()) {
                    announcement.set_type(utility::conversions::to_utf8string(json[U("type")].as_string()));
                }
                
                if (json.has_field(U("expire_time")) && !json[U("expire_time")].is_null()) {
                    announcement.set_expire_time(utility::conversions::to_utf8string(json[U("expire_time")].as_string()));
                }
                
                auto updated_announcement = announcement_service_->update_announcement(announcement);
                message.reply(status_codes::OK, HttpResponseUtil::create_success_response("announcement", updated_announcement.to_json()));
            } catch (const AnnouncementNotFoundException& e) {
                message.reply(status_codes::NotFound, HttpResponseUtil::create_error_response(ErrorCode::ANNOUNCEMENT_NOT_FOUND, e.what()));
            } catch (const ValidationException& e) {
                message.reply(status_codes::BadRequest, HttpResponseUtil::create_validation_error_response(e.get_errors()));
            } catch (const std::exception& e) {
                message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
            }
        });
    } catch (const std::invalid_argument& e) {
        message.reply(status_codes::BadRequest, HttpResponseUtil::create_error_response(ErrorCode::INVALID_PARAMS, "无效的公告ID"));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void AnnouncementController::handle_delete_announcement(http_request message) {
    try {
        auto path = message.request_uri().path();
        auto id_str = path.substr(path.find_last_of('/') + 1);
        int64_t id = std::stoll(utility::conversions::to_utf8string(id_str));
        
        announcement_service_->delete_announcement(id);
        message.reply(status_codes::NoContent);
    } catch (const AnnouncementNotFoundException& e) {
        message.reply(status_codes::NotFound, HttpResponseUtil::create_error_response(ErrorCode::ANNOUNCEMENT_NOT_FOUND, e.what()));
    } catch (const std::invalid_argument& e) {
        message.reply(status_codes::BadRequest, HttpResponseUtil::create_error_response(ErrorCode::INVALID_PARAMS, "无效的公告ID"));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void AnnouncementController::handle_publish_announcement(http_request message) {
    try {
        auto path = message.request_uri().path();
        auto id_str = path.substr(path.find_last_of('/') + 1);
        int64_t id = std::stoll(utility::conversions::to_utf8string(id_str));
        
        announcement_service_->publish_announcement(id);
        message.reply(status_codes::OK, HttpResponseUtil::create_success_response("message", "公告发布成功"));
    } catch (const AnnouncementNotFoundException& e) {
        message.reply(status_codes::NotFound, HttpResponseUtil::create_error_response(ErrorCode::ANNOUNCEMENT_NOT_FOUND, e.what()));
    } catch (const std::invalid_argument& e) {
        message.reply(status_codes::BadRequest, HttpResponseUtil::create_error_response(ErrorCode::INVALID_PARAMS, "无效的公告ID"));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void AnnouncementController::handle_unpublish_announcement(http_request message) {
    try {
        auto path = message.request_uri().path();
        auto id_str = path.substr(path.find_last_of('/') + 1);
        int64_t id = std::stoll(utility::conversions::to_utf8string(id_str));
        
        announcement_service_->unpublish_announcement(id);
        message.reply(status_codes::OK, HttpResponseUtil::create_success_response("message", "公告取消发布成功"));
    } catch (const AnnouncementNotFoundException& e) {
        message.reply(status_codes::NotFound, HttpResponseUtil::create_error_response(ErrorCode::ANNOUNCEMENT_NOT_FOUND, e.what()));
    } catch (const std::invalid_argument& e) {
        message.reply(status_codes::BadRequest, HttpResponseUtil::create_error_response(ErrorCode::INVALID_PARAMS, "无效的公告ID"));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void AnnouncementController::handle_get_unread_announcements(http_request message) {
    try {
        auto current_user_id = get_current_user_id(message);
        QueryParams params;
        
        if (!message.request_uri().query().empty()) {
            uri_builder ub(U("?") + message.request_uri().query());
            for (auto& query : ub.query()) {
                params[utility::conversions::to_utf8string(query.first)] = 
                    utility::conversions::to_utf8string(query.second);
            }
        }
        
        auto result = read_receipt_service_->get_unread_announcements(std::stoll(current_user_id), params);
        message.reply(status_codes::OK, HttpResponseUtil::create_pagination_response("unread_announcements", result));
    } catch (const InvalidTokenException& e) {
        message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::UNAUTHORIZED, e.what()));
    } catch (const TokenExpiredException& e) {
        message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::TOKEN_EXPIRED, e.what()));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void AnnouncementController::handle_get_read_announcements(http_request message) {
    try {
        auto current_user_id = get_current_user_id(message);
        QueryParams params;
        
        if (!message.request_uri().query().empty()) {
            uri_builder ub(U("?") + message.request_uri().query());
            for (auto& query : ub.query()) {
                params[utility::conversions::to_utf8string(query.first)] = 
                    utility::conversions::to_utf8string(query.second);
            }
        }
        
        auto result = read_receipt_service_->get_read_announcements(std::stoll(current_user_id), params);
        message.reply(status_codes::OK, HttpResponseUtil::create_pagination_response("read_announcements", result));
    } catch (const InvalidTokenException& e) {
        message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::UNAUTHORIZED, e.what()));
    } catch (const TokenExpiredException& e) {
        message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::TOKEN_EXPIRED, e.what()));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void AnnouncementController::handle_mark_as_read(http_request message) {
    try {
        auto current_user_id = get_current_user_id(message);
        auto path = message.request_uri().path();
        auto id_str = path.substr(path.find_last_of('/') + 1);
        int64_t announcement_id = std::stoll(utility::conversions::to_utf8string(id_str));
        
        read_receipt_service_->mark_as_read(std::stoll(current_user_id), announcement_id);
        message.reply(status_codes::OK, HttpResponseUtil::create_success_response("message", "标记已读成功"));
    } catch (const InvalidTokenException& e) {
        message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::UNAUTHORIZED, e.what()));
    } catch (const TokenExpiredException& e) {
        message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::TOKEN_EXPIRED, e.what()));
    } catch (const AnnouncementNotFoundException& e) {
        message.reply(status_codes::NotFound, HttpResponseUtil::create_error_response(ErrorCode::ANNOUNCEMENT_NOT_FOUND, e.what()));
    } catch (const std::invalid_argument& e) {
        message.reply(status_codes::BadRequest, HttpResponseUtil::create_error_response(ErrorCode::INVALID_PARAMS, "无效的公告ID"));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void AnnouncementController::handle_batch_mark_as_read(http_request message) {
    try {
        auto current_user_id = get_current_user_id(message);
        
        message.extract_json().then([this, current_user_id](pplx::task<json::value> task) {
            try {
                auto json = task.get();
                auto announcement_ids = json[U("announcement_ids")].as_array();
                
                std::vector<int64_t> ids;
                for (auto& id : announcement_ids) {
                    ids.push_back(static_cast<int64_t>(id.as_number().to_int64()));
                }
                
                read_receipt_service_->batch_mark_as_read(std::stoll(current_user_id), ids);
                message.reply(status_codes::OK, HttpResponseUtil::create_success_response("message", "批量标记已读成功"));
            } catch (const AnnouncementNotFoundException& e) {
                message.reply(status_codes::NotFound, HttpResponseUtil::create_error_response(ErrorCode::ANNOUNCEMENT_NOT_FOUND, e.what()));
            } catch (const std::exception& e) {
                message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
            }
        });
    } catch (const InvalidTokenException& e) {
        message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::UNAUTHORIZED, e.what()));
    } catch (const TokenExpiredException& e) {
        message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::TOKEN_EXPIRED, e.what()));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void AnnouncementController::handle_get_announcement_statistics(http_request message) {
    try {
        auto current_user_id = get_current_user_id(message);
        QueryParams params;
        
        if (!message.request_uri().query().empty()) {
            uri_builder ub(U("?") + message.request_uri().query());
            for (auto& query : ub.query()) {
                params[utility::conversions::to_utf8string(query.first)] = 
                    utility::conversions::to_utf8string(query.second);
            }
        }
        
        auto stats = read_receipt_service_->get_announcement_statistics(params);
        
        json::value response;
        response[U("total_announcements")] = json::value::number(static_cast<double>(stats.total_announcements));
        response[U("published_announcements")] = json::value::number(static_cast<double>(stats.published_announcements));
        response[U("total_read")] = json::value::number(static_cast<double>(stats.total_read));
        response[U("total_unread")] = json::value::number(static_cast<double>(stats.total_unread));
        response[U("avg_read_time")] = json::value::string(utility::conversions::to_string_t(stats.avg_read_time));
        
        message.reply(status_codes::OK, HttpResponseUtil::create_success_response("statistics", response));
    } catch (const InvalidTokenException& e) {
        message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::UNAUTHORIZED, e.what()));
    } catch (const TokenExpiredException& e) {
        message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::TOKEN_EXPIRED, e.what()));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void AnnouncementController::register_routes(uri_builder& uri_builder, std::function<void(http_listener&)> add_route) {
    auto base_path = uri_builder.to_string();
    
    // 公告管理路由
    add_route(http_listener(base_path + U("/announcements"))
        .support(methods::GET, [this](http_request message) { this->handle_get_announcements(message); })
        .support(methods::POST, [this](http_request message) { this->handle_create_announcement(message); }));
    
    add_route(http_listener(base_path + U("/announcements/{id}"))
        .support(methods::GET, [this](http_request message) { this->handle_get_announcement_by_id(message); })
        .support(methods::PUT, [this](http_request message) { this->handle_update_announcement(message); })
        .support(methods::DEL, [this](http_request message) { this->handle_delete_announcement(message); }));
    
    add_route(http_listener(base_path + U("/announcements/{id}/publish"))
        .support(methods::POST, [this](http_request message) { this->handle_publish_announcement(message); }));
    
    add_route(http_listener(base_path + U("/announcements/{id}/unpublish"))
        .support(methods::POST, [this](http_request message) { this->handle_unpublish_announcement(message); }));
    
    // 阅读记录路由
    add_route(http_listener(base_path + U("/announcements/unread"))
        .support(methods::GET, [this](http_request message) { this->handle_get_unread_announcements(message); }));
    
    add_route(http_listener(base_path + U("/announcements/read"))
        .support(methods::GET, [this](http_request message) { this->handle_get_read_announcements(message); }));
    
    add_route(http_listener(base_path + U("/announcements/{id}/mark-read"))
        .support(methods::POST, [this](http_request message) { this->handle_mark_as_read(message); }));
    
    add_route(http_listener(base_path + U("/announcements/batch-mark-read"))
        .support(methods::POST, [this](http_request message) { this->handle_batch_mark_as_read(message); }));
    
    // 统计路由
    add_route(http_listener(base_path + U("/announcements/statistics"))
        .support(methods::GET, [this](http_request message) { this->handle_get_announcement_statistics(message); }));
}
