#include "user_controller.h"
#include <cpprest/json.h>

UserController::UserController(std::shared_ptr<UserService> user_service, std::shared_ptr<AuthService> auth_service)
    : user_service_(user_service), auth_service_(auth_service) {}

void UserController::handle_get_users(http_request message) {
    try {
        QueryParams params;
        
        // 解析查询参数
        if (!message.request_uri().query().empty()) {
            uri_builder ub(U("?") + message.request_uri().query());
            for (auto& query : ub.query()) {
                params[utility::conversions::to_utf8string(query.first)] = 
                    utility::conversions::to_utf8string(query.second);
            }
        }
        
        auto result = user_service_->get_users(params);
        message.reply(status_codes::OK, HttpResponseUtil::create_pagination_response("users", result));
    } catch (const UserNotFoundException& e) {
        message.reply(status_codes::NotFound, HttpResponseUtil::create_error_response(ErrorCode::USER_NOT_FOUND, e.what()));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void UserController::handle_get_user_by_id(http_request message) {
    try {
        auto path = message.request_uri().path();
        auto id_str = path.substr(path.find_last_of('/') + 1);
        int64_t id = std::stoll(utility::conversions::to_utf8string(id_str));
        
        auto user = user_service_->get_user_by_id(id);
        message.reply(status_codes::OK, HttpResponseUtil::create_success_response("user", user.to_json()));
    } catch (const UserNotFoundException& e) {
        message.reply(status_codes::NotFound, HttpResponseUtil::create_error_response(ErrorCode::USER_NOT_FOUND, e.what()));
    } catch (const std::invalid_argument& e) {
        message.reply(status_codes::BadRequest, HttpResponseUtil::create_error_response(ErrorCode::INVALID_PARAMS, "无效的用户ID"));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void UserController::handle_create_user(http_request message) {
    try {
        message.extract_json().then([this](pplx::task<json::value> task) {
            try {
                auto json = task.get();
                
                User user;
                user.set_username(utility::conversions::to_utf8string(json[U("username")].as_string()));
                user.set_password(utility::conversions::to_utf8string(json[U("password")].as_string()));
                
                if (json.has_field(U("email")) && !json[U("email")].is_null()) {
                    user.set_email(utility::conversions::to_utf8string(json[U("email")].as_string()));
                }
                
                if (json.has_field(U("full_name")) && !json[U("full_name")].is_null()) {
                    user.set_full_name(utility::conversions::to_utf8string(json[U("full_name")].as_string()));
                }
                
                if (json.has_field(U("role")) && !json[U("role")].is_null()) {
                    user.set_role(utility::conversions::to_utf8string(json[U("role")].as_string()));
                }
                
                auto created_user = user_service_->create_user(user);
                message.reply(status_codes::Created, HttpResponseUtil::create_success_response("user", created_user.to_json()));
            } catch (const UserAlreadyExistsException& e) {
                message.reply(status_codes::Conflict, HttpResponseUtil::create_error_response(ErrorCode::USER_ALREADY_EXISTS, e.what()));
            } catch (const ValidationException& e) {
                message.reply(status_codes::BadRequest, HttpResponseUtil::create_validation_error_response(e.get_errors()));
            } catch (const std::exception& e) {
                message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
            }
        });
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void UserController::handle_update_user(http_request message) {
    try {
        auto path = message.request_uri().path();
        auto id_str = path.substr(path.find_last_of('/') + 1);
        int64_t id = std::stoll(utility::conversions::to_utf8string(id_str));
        
        message.extract_json().then([this, id](pplx::task<json::value> task) {
            try {
                auto json = task.get();
                
                User user;
                user.set_id(id);
                
                if (json.has_field(U("username")) && !json[U("username")].is_null()) {
                    user.set_username(utility::conversions::to_utf8string(json[U("username")].as_string()));
                }
                
                if (json.has_field(U("email")) && !json[U("email")].is_null()) {
                    user.set_email(utility::conversions::to_utf8string(json[U("email")].as_string()));
                }
                
                if (json.has_field(U("full_name")) && !json[U("full_name")].is_null()) {
                    user.set_full_name(utility::conversions::to_utf8string(json[U("full_name")].as_string()));
                }
                
                if (json.has_field(U("role")) && !json[U("role")].is_null()) {
                    user.set_role(utility::conversions::to_utf8string(json[U("role")].as_string()));
                }
                
                auto updated_user = user_service_->update_user(user);
                message.reply(status_codes::OK, HttpResponseUtil::create_success_response("user", updated_user.to_json()));
            } catch (const UserNotFoundException& e) {
                message.reply(status_codes::NotFound, HttpResponseUtil::create_error_response(ErrorCode::USER_NOT_FOUND, e.what()));
            } catch (const UserAlreadyExistsException& e) {
                message.reply(status_codes::Conflict, HttpResponseUtil::create_error_response(ErrorCode::USER_ALREADY_EXISTS, e.what()));
            } catch (const ValidationException& e) {
                message.reply(status_codes::BadRequest, HttpResponseUtil::create_validation_error_response(e.get_errors()));
            } catch (const std::exception& e) {
                message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
            }
        });
    } catch (const std::invalid_argument& e) {
        message.reply(status_codes::BadRequest, HttpResponseUtil::create_error_response(ErrorCode::INVALID_PARAMS, "无效的用户ID"));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void UserController::handle_delete_user(http_request message) {
    try {
        auto path = message.request_uri().path();
        auto id_str = path.substr(path.find_last_of('/') + 1);
        int64_t id = std::stoll(utility::conversions::to_utf8string(id_str));
        
        user_service_->delete_user(id);
        message.reply(status_codes::NoContent);
    } catch (const UserNotFoundException& e) {
        message.reply(status_codes::NotFound, HttpResponseUtil::create_error_response(ErrorCode::USER_NOT_FOUND, e.what()));
    } catch (const std::invalid_argument& e) {
        message.reply(status_codes::BadRequest, HttpResponseUtil::create_error_response(ErrorCode::INVALID_PARAMS, "无效的用户ID"));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void UserController::handle_login(http_request message) {
    try {
        message.extract_json().then([this](pplx::task<json::value> task) {
            try {
                auto json = task.get();
                auto username = utility::conversions::to_utf8string(json[U("username")].as_string());
                auto password = utility::conversions::to_utf8string(json[U("password")].as_string());
                
                auto token_info = auth_service_->login(username, password);
                
                json::value response;
                response[U("access_token")] = json::value::string(utility::conversions::to_string_t(token_info.access_token));
                response[U("refresh_token")] = json::value::string(utility::conversions::to_string_t(token_info.refresh_token));
                response[U("token_type")] = json::value::string(U("Bearer"));
                response[U("expires_in")] = json::value::number(static_cast<double>(token_info.expires_in));
                
                message.reply(status_codes::OK, HttpResponseUtil::create_success_response("auth", response));
            } catch (const UserNotFoundException& e) {
                message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::UNAUTHORIZED, "用户名或密码错误"));
            } catch (const InvalidCredentialsException& e) {
                message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::UNAUTHORIZED, "用户名或密码错误"));
            } catch (const std::exception& e) {
                message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
            }
        });
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void UserController::handle_refresh_token(http_request message) {
    try {
        message.extract_json().then([this](pplx::task<json::value> task) {
            try {
                auto json = task.get();
                auto refresh_token = utility::conversions::to_utf8string(json[U("refresh_token")].as_string());
                
                auto token_info = auth_service_->refresh_token(refresh_token);
                
                json::value response;
                response[U("access_token")] = json::value::string(utility::conversions::to_string_t(token_info.access_token));
                response[U("token_type")] = json::value::string(U("Bearer"));
                response[U("expires_in")] = json::value::number(static_cast<double>(token_info.expires_in));
                
                message.reply(status_codes::OK, HttpResponseUtil::create_success_response("auth", response));
            } catch (const InvalidTokenException& e) {
                message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::UNAUTHORIZED, e.what()));
            } catch (const TokenExpiredException& e) {
                message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::TOKEN_EXPIRED, e.what()));
            } catch (const std::exception& e) {
                message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
            }
        });
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void UserController::handle_change_password(http_request message) {
    try {
        auto path = message.request_uri().path();
        auto id_str = path.substr(path.find_last_of('/') + 1);
        int64_t id = std::stoll(utility::conversions::to_utf8string(id_str));
        
        message.extract_json().then([this, id](pplx::task<json::value> task) {
            try {
                auto json = task.get();
                auto old_password = utility::conversions::to_utf8string(json[U("old_password")].as_string());
                auto new_password = utility::conversions::to_utf8string(json[U("new_password")].as_string());
                
                user_service_->change_password(id, old_password, new_password);
                message.reply(status_codes::OK, HttpResponseUtil::create_success_response("message", "密码修改成功"));
            } catch (const UserNotFoundException& e) {
                message.reply(status_codes::NotFound, HttpResponseUtil::create_error_response(ErrorCode::USER_NOT_FOUND, e.what()));
            } catch (const InvalidCredentialsException& e) {
                message.reply(status_codes::BadRequest, HttpResponseUtil::create_error_response(ErrorCode::INVALID_PARAMS, "旧密码错误"));
            } catch (const ValidationException& e) {
                message.reply(status_codes::BadRequest, HttpResponseUtil::create_validation_error_response(e.get_errors()));
            } catch (const std::exception& e) {
                message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
            }
        });
    } catch (const std::invalid_argument& e) {
        message.reply(status_codes::BadRequest, HttpResponseUtil::create_error_response(ErrorCode::INVALID_PARAMS, "无效的用户ID"));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void UserController::handle_reset_password(http_request message) {
    try {
        auto path = message.request_uri().path();
        auto id_str = path.substr(path.find_last_of('/') + 1);
        int64_t id = std::stoll(utility::conversions::to_utf8string(id_str));
        
        message.extract_json().then([this, id](pplx::task<json::value> task) {
            try {
                auto json = task.get();
                auto new_password = utility::conversions::to_utf8string(json[U("new_password")].as_string());
                
                user_service_->reset_password(id, new_password);
                message.reply(status_codes::OK, HttpResponseUtil::create_success_response("message", "密码重置成功"));
            } catch (const UserNotFoundException& e) {
                message.reply(status_codes::NotFound, HttpResponseUtil::create_error_response(ErrorCode::USER_NOT_FOUND, e.what()));
            } catch (const ValidationException& e) {
                message.reply(status_codes::BadRequest, HttpResponseUtil::create_validation_error_response(e.get_errors()));
            } catch (const std::exception& e) {
                message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
            }
        });
    } catch (const std::invalid_argument& e) {
        message.reply(status_codes::BadRequest, HttpResponseUtil::create_error_response(ErrorCode::INVALID_PARAMS, "无效的用户ID"));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void UserController::handle_get_current_user(http_request message) {
    try {
        // 从认证中间件获取用户信息
        auto token = message.headers().find(U("Authorization"));
        if (token == message.headers().end()) {
            message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::UNAUTHORIZED, "缺少授权令牌"));
            return;
        }
        
        auto token_str = utility::conversions::to_utf8string(token->second);
        if (token_str.substr(0, 7) == "Bearer ") {
            token_str = token_str.substr(7);
        }
        
        auto token_info = auth_service_->verify_token(token_str);
        auto user = user_service_->get_user_by_id(std::stoll(token_info.user_id));
        
        message.reply(status_codes::OK, HttpResponseUtil::create_success_response("user", user.to_json()));
    } catch (const InvalidTokenException& e) {
        message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::UNAUTHORIZED, e.what()));
    } catch (const TokenExpiredException& e) {
        message.reply(status_codes::Unauthorized, HttpResponseUtil::create_error_response(ErrorCode::TOKEN_EXPIRED, e.what()));
    } catch (const UserNotFoundException& e) {
        message.reply(status_codes::NotFound, HttpResponseUtil::create_error_response(ErrorCode::USER_NOT_FOUND, e.what()));
    } catch (const std::exception& e) {
        message.reply(status_codes::InternalError, HttpResponseUtil::create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, "服务器内部错误"));
    }
}

void UserController::register_routes(uri_builder& uri_builder, std::function<void(http_listener&)> add_route) {
    auto base_path = uri_builder.to_string();
    
    // 用户管理路由
    add_route(http_listener(base_path + U("/users"))
        .support(methods::GET, [this](http_request message) { this->handle_get_users(message); })
        .support(methods::POST, [this](http_request message) { this->handle_create_user(message); }));
    
    add_route(http_listener(base_path + U("/users/{id}"))
        .support(methods::GET, [this](http_request message) { this->handle_get_user_by_id(message); })
        .support(methods::PUT, [this](http_request message) { this->handle_update_user(message); })
        .support(methods::DEL, [this](http_request message) { this->handle_delete_user(message); }));
    
    add_route(http_listener(base_path + U("/users/{id}/change-password"))
        .support(methods::POST, [this](http_request message) { this->handle_change_password(message); }));
    
    add_route(http_listener(base_path + U("/users/{id}/reset-password"))
        .support(methods::POST, [this](http_request message) { this->handle_reset_password(message); }));
    
    // 认证路由
    add_route(http_listener(base_path + U("/auth/login"))
        .support(methods::POST, [this](http_request message) { this->handle_login(message); }));
    
    add_route(http_listener(base_path + U("/auth/refresh"))
        .support(methods::POST, [this](http_request message) { this->handle_refresh_token(message); }));
    
    add_route(http_listener(base_path + U("/auth/me"))
        .support(methods::GET, [this](http_request message) { this->handle_get_current_user(message); }));
}
