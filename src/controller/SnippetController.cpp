#include "controller/SnippetController.h"
#include "service/SnippetService.h"
#include "service/UserService.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <optional>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

namespace controller {

SnippetController::SnippetController(std::shared_ptr<service::SnippetService> snippet_service,
                                       std::shared_ptr<service::UserService> user_service,
                                       std::shared_ptr<server::HttpServer> http_server)
    : snippet_service_(std::move(snippet_service)),
      user_service_(std::move(user_service)),
      http_server_(std::move(http_server)) {
}

void SnippetController::registerEndpoints() {
    // 注册创建代码片段端点
    http_server_->registerHandler("POST", "/api/snippets", 
        [this](const server::http::request<server::http::string_body>& request, 
               server::http::response<server::http::string_body>& response) {
            handleCreateSnippet(request, response);
        });

    // 注册获取单个代码片段详情端点
    http_server_->registerHandler("GET", "/api/snippets/", 
        [this](const server::http::request<server::http::string_body>& request, 
               server::http::response<server::http::string_body>& response) {
            handleGetSnippetById(request, response);
        });

    // 注册更新代码片段端点
    http_server_->registerHandler("PUT", "/api/snippets/", 
        [this](const server::http::request<server::http::string_body>& request, 
               server::http::response<server::http::string_body>& response) {
            handleUpdateSnippet(request, response);
        });

    // 注册删除代码片段端点
    http_server_->registerHandler("DELETE", "/api/snippets/", 
        [this](const server::http::request<server::http::string_body>& request, 
               server::http::response<server::http::string_body>& response) {
            handleDeleteSnippet(request, response);
        });

    // 注册搜索代码片段端点
    http_server_->registerHandler("GET", "/api/snippets/search", 
        [this](const server::http::request<server::http::string_body>& request, 
               server::http::response<server::http::string_body>& response) {
            handleSearchSnippets(request, response);
        });

    // 注册收藏代码片段端点
    http_server_->registerHandler("POST", "/api/snippets//star", 
        [this](const server::http::request<server::http::string_body>& request, 
               server::http::response<server::http::string_body>& response) {
            handleStarSnippet(request, response);
        });

    // 注册取消收藏代码片段端点
    http_server_->registerHandler("DELETE", "/api/snippets//star", 
        [this](const server::http::request<server::http::string_body>& request, 
               server::http::response<server::http::string_body>& response) {
            handleUnstarSnippet(request, response);
        });

    // 注册列出某个用户的代码片段端点
    http_server_->registerHandler("GET", "/api/users//snippets", 
        [this](const server::http::request<server::http::string_body>& request, 
               server::http::response<server::http::string_body>& response) {
            handleGetUserSnippets(request, response);
        });
}

std::optional<model::User> SnippetController::authenticateUser(const server::http::request<server::http::string_body>& request) {
    // 从请求头中获取 Authorization 字段
    auto authorization_it = request.find(server::http::field::authorization);
    if (authorization_it == request.end()) {
        return std::nullopt;
    }

    std::string authorization_header = authorization_it->value();
    // 验证 Authorization 字段的格式
    if (authorization_header.substr(0, 7) != "Bearer ") {
        return std::nullopt;
    }

    // 提取 token
    std::string token = authorization_header.substr(7);

    // 调用 UserService 中的 validateToken 方法验证 token
    std::optional<int> user_id = user_service_->validateToken(token);
    if (!user_id) {
        return std::nullopt;
    }

    // 调用 UserService 中的 getUserById 方法获取用户信息
    model::User user = user_service_->getUserById(*user_id);

    return user;
}

void SnippetController::handleCreateSnippet(const server::http::request<server::http::string_body>& request, 
                                               server::http::response<server::http::string_body>& response) {
    try {
        // 验证用户身份
        std::optional<model::User> user = authenticateUser(request);
        if (!user) {
            response.result(server::http::status::unauthorized);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "UNAUTHORIZED",
                "message": "Invalid or missing token"
            })";
            return;
        }

        // 解析请求体
        json request_body = json::parse(request.body());

        // 提取请求参数
        std::string title = request_body["title"];
        std::string language = request_body["language"];
        std::string content = request_body["content"];
        std::vector<std::string> tags;
        if (request_body.contains("tags") && request_body["tags"].is_array()) {
            for (const auto& tag : request_body["tags"]) {
                if (tag.is_string()) {
                    tags.push_back(tag);
                }
            }
        }
        bool is_public = true;
        if (request_body.contains("is_public") && request_body["is_public"].is_boolean()) {
            is_public = request_body["is_public"];
        }

        // 创建 Snippet 对象
        model::Snippet snippet;
        snippet.set_owner_id(user->id());
        snippet.set_title(title);
        snippet.set_language(language);
        snippet.set_content(content);
        snippet.set_tags(tags);
        snippet.set_is_public(is_public);

        // 调用 SnippetService 中的 createSnippet 方法
        model::Snippet created_snippet = snippet_service_->createSnippet(snippet, user->id());

        // 生成响应体
        json response_body;
        response_body["id"] = created_snippet.id();
        response_body["owner_id"] = created_snippet.owner_id();
        response_body["title"] = created_snippet.title();
        response_body["language"] = created_snippet.language();
        response_body["content"] = created_snippet.content();
        response_body["tags"] = created_snippet.tags();
        response_body["is_public"] = created_snippet.is_public();
        response_body["created_at"] = created_snippet.created_at().time_since_epoch().count();
        response_body["updated_at"] = created_snippet.updated_at().time_since_epoch().count();
        response_body["star_count"] = created_snippet.star_count();

        // 设置响应
        response.result(server::http::status::created);
        response.set(server::http::field::content_type, "application/json");
        response.body() = response_body.dump();
    } catch (const json::parse_error& e) {
        // 处理 JSON 解析错误
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INVALID_JSON",
            "message": "Failed to parse request body"
        })";
    } catch (const service::ServiceException& e) {
        // 处理业务逻辑错误
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        std::string error_message = R"({
            "error": "BAD_REQUEST",
            "message": ")" + std::string(e.what()) + R"("
        })";
        response.body() = error_message;
    } catch (const std::exception& e) {
        // 处理其他异常
        std::cerr << "Error handling create snippet request: " << e.what() << std::endl;
        response.result(server::http::status::internal_server_error);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INTERNAL_SERVER_ERROR",
            "message": "An internal server error occurred"
        })";
    }
}

void SnippetController::handleGetSnippetById(const server::http::request<server::http::string_body>& request, 
                                                server::http::response<server::http::string_body>& response) {
    try {
        // 从请求路径中提取 snippet ID
        std::string path = std::string(request.target());
        size_t last_slash_pos = path.find_last_of("/");
        if (last_slash_pos == std::string::npos) {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "INVALID_PARAMS",
                "message": "Invalid snippet ID"
            })";
            return;
        }
        std::string id_str = path.substr(last_slash_pos + 1);
        int id = std::stoi(id_str);

        // 验证用户身份（可选，因为公开的片段可以被任何人访问）
        std::optional<model::User> user = authenticateUser(request);

        // 调用 SnippetService 中的 getSnippetById 方法
        std::optional<model::Snippet> snippet = snippet_service_->getSnippetById(id, user ? user->id() : 0);

        if (!snippet) {
            response.result(server::http::status::not_found);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "NOT_FOUND",
                "message": "Snippet not found"
            })";
            return;
        }

        // 生成响应体
        json response_body;
        response_body["id"] = snippet->id();
        response_body["owner_id"] = snippet->owner_id();
        response_body["title"] = snippet->title();
        response_body["language"] = snippet->language();
        response_body["content"] = snippet->content();
        response_body["tags"] = snippet->tags();
        response_body["is_public"] = snippet->is_public();
        response_body["created_at"] = snippet->created_at().time_since_epoch().count();
        response_body["updated_at"] = snippet->updated_at().time_since_epoch().count();
        response_body["star_count"] = snippet->star_count();

        // 设置响应
        response.result(server::http::status::ok);
        response.set(server::http::field::content_type, "application/json");
        response.body() = response_body.dump();
    } catch (const std::invalid_argument& e) {
        // 处理无效的 ID
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INVALID_PARAMS",
            "message": "Invalid snippet ID"
        })";
    } catch (const service::ServiceException& e) {
        // 处理业务逻辑错误
        if (e.what() == std::string("Access denied")) {
            response.result(server::http::status::forbidden);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "FORBIDDEN",
                "message": "Access denied"
            })";
        } else {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            std::string error_message = R"({
                "error": "BAD_REQUEST",
                "message": ")" + std::string(e.what()) + R"("
            })";
            response.body() = error_message;
        }
    } catch (const std::exception& e) {
        // 处理其他异常
        std::cerr << "Error handling get snippet by ID request: " << e.what() << std::endl;
        response.result(server::http::status::internal_server_error);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INTERNAL_SERVER_ERROR",
            "message": "An internal server error occurred"
        })";
    }
}

void SnippetController::handleUpdateSnippet(const server::http::request<server::http::string_body>& request, 
                                               server::http::response<server::http::string_body>& response) {
    try {
        // 从请求路径中提取 snippet ID
        std::string path = std::string(request.target());
        size_t last_slash_pos = path.find_last_of("/");
        if (last_slash_pos == std::string::npos) {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "INVALID_PARAMS",
                "message": "Invalid snippet ID"
            })";
            return;
        }
        std::string id_str = path.substr(last_slash_pos + 1);
        int id = std::stoi(id_str);

        // 验证用户身份
        std::optional<model::User> user = authenticateUser(request);
        if (!user) {
            response.result(server::http::status::unauthorized);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "UNAUTHORIZED",
                "message": "Invalid or missing token"
            })";
            return;
        }

        // 解析请求体
        json request_body = json::parse(request.body());

        // 提取请求参数
        std::string title = request_body["title"];
        std::string language = request_body["language"];
        std::string content = request_body["content"];
        std::vector<std::string> tags;
        if (request_body.contains("tags") && request_body["tags"].is_array()) {
            for (const auto& tag : request_body["tags"]) {
                if (tag.is_string()) {
                    tags.push_back(tag);
                }
            }
        }
        bool is_public = true;
        if (request_body.contains("is_public") && request_body["is_public"].is_boolean()) {
            is_public = request_body["is_public"];
        }

        // 创建 Snippet 对象
        model::Snippet snippet;
        snippet.set_id(id);
        snippet.set_owner_id(user->id());
        snippet.set_title(title);
        snippet.set_language(language);
        snippet.set_content(content);
        snippet.set_tags(tags);
        snippet.set_is_public(is_public);

        // 调用 SnippetService 中的 updateSnippet 方法
        snippet_service_->updateSnippet(snippet, user->id());

        // 设置响应
        response.result(server::http::status::no_content);
    } catch (const json::parse_error& e) {
        // 处理 JSON 解析错误
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INVALID_JSON",
            "message": "Failed to parse request body"
        })";
    } catch (const service::ServiceException& e) {
        // 处理业务逻辑错误
        if (e.what() == std::string("Access denied")) {
            response.result(server::http::status::forbidden);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "FORBIDDEN",
                "message": "Access denied"
            })";
        } else {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            std::string error_message = R"({
                "error": "BAD_REQUEST",
                "message": ")" + std::string(e.what()) + R"("
            })";
            response.body() = error_message;
        }
    } catch (const std::exception& e) {
        // 处理其他异常
        std::cerr << "Error handling update snippet request: " << e.what() << std::endl;
        response.result(server::http::status::internal_server_error);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INTERNAL_SERVER_ERROR",
            "message": "An internal server error occurred"
        })";
    }
}

void SnippetController::handleDeleteSnippet(const server::http::request<server::http::string_body>& request, 
                                               server::http::response<server::http::string_body>& response) {
    try {
        // 从请求路径中提取 snippet ID
        std::string path = std::string(request.target());
        size_t last_slash_pos = path.find_last_of("/");
        if (last_slash_pos == std::string::npos) {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "INVALID_PARAMS",
                "message": "Invalid snippet ID"
            })";
            return;
        }
        std::string id_str = path.substr(last_slash_pos + 1);
        int id = std::stoi(id_str);

        // 验证用户身份
        std::optional<model::User> user = authenticateUser(request);
        if (!user) {
            response.result(server::http::status::unauthorized);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "UNAUTHORIZED",
                "message": "Invalid or missing token"
            })";
            return;
        }

        // 调用 SnippetService 中的 deleteSnippet 方法
        snippet_service_->deleteSnippet(id, user->id());

        // 设置响应
        response.result(server::http::status::no_content);
    } catch (const std::invalid_argument& e) {
        // 处理无效的 ID
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INVALID_PARAMS",
            "message": "Invalid snippet ID"
        })";
    } catch (const service::ServiceException& e) {
        // 处理业务逻辑错误
        if (e.what() == std::string("Access denied")) {
            response.result(server::http::status::forbidden);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "FORBIDDEN",
                "message": "Access denied"
            })";
        } else {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            std::string error_message = R"({
                "error": "BAD_REQUEST",
                "message": ")" + std::string(e.what()) + R"("
            })";
            response.body() = error_message;
        }
    } catch (const std::exception& e) {
        // 处理其他异常
        std::cerr << "Error handling delete snippet request: " << e.what() << std::endl;
        response.result(server::http::status::internal_server_error);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INTERNAL_SERVER_ERROR",
            "message": "An internal server error occurred"
        })";
    }
}

void SnippetController::handleSearchSnippets(const server::http::request<server::http::string_body>& request, 
                                                server::http::response<server::http::string_body>& response) {
    try {
        // 从请求参数中提取搜索条件
        std::string query = "";
        std::string language = "";
        std::string tag = "";
        int page = 1;
        int page_size = 20;

        std::string target = std::string(request.target());
        size_t query_pos = target.find("?");
        if (query_pos != std::string::npos) {
            std::string query_string = target.substr(query_pos + 1);
            std::istringstream iss(query_string);
            std::string param;
            while (std::getline(iss, param, '&')) {
                size_t equal_pos = param.find("=");
                if (equal_pos == std::string::npos) {
                    continue;
                }
                std::string key = param.substr(0, equal_pos);
                std::string value = param.substr(equal_pos + 1);

                if (key == "query") {
                    query = value;
                } else if (key == "language") {
                    language = value;
                } else if (key == "tag") {
                    tag = value;
                } else if (key == "page") {
                    page = std::stoi(value);
                } else if (key == "page_size") {
                    page_size = std::stoi(value);
                }
            }
        }

        // 验证用户身份（可选，因为公开的片段可以被任何人访问）
        std::optional<model::User> user = authenticateUser(request);

        // 创建 SearchParams 对象
        service::SnippetService::SearchParams search_params;
        search_params.q = query;
        search_params.language = language;
        search_params.tag = tag;
        search_params.page = page;
        search_params.page_size = page_size;

        // 调用 SnippetService 中的 searchSnippets 方法
        service::SnippetService::SearchResult search_result = snippet_service_->searchSnippets(search_params, user ? user->id() : 0);

        // 生成响应体
        json response_body;
        response_body["total"] = search_result.total;
        response_body["page"] = page;
        response_body["page_size"] = page_size;
        response_body["snippets"] = json::array();

        for (const auto& snippet : search_result.items) {
            json snippet_json;
            snippet_json["id"] = snippet.id();
            snippet_json["owner_id"] = snippet.owner_id();
            snippet_json["title"] = snippet.title();
            snippet_json["language"] = snippet.language();
            snippet_json["content"] = snippet.content();
            snippet_json["tags"] = snippet.tags();
            snippet_json["is_public"] = snippet.is_public();
            snippet_json["created_at"] = snippet.created_at().time_since_epoch().count();
            snippet_json["updated_at"] = snippet.updated_at().time_since_epoch().count();
            snippet_json["star_count"] = snippet.star_count();
            response_body["snippets"].push_back(snippet_json);
        }

        // 设置响应
        response.result(server::http::status::ok);
        response.set(server::http::field::content_type, "application/json");
        response.body() = response_body.dump();
    } catch (const std::invalid_argument& e) {
        // 处理无效的参数
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INVALID_PARAMS",
            "message": "Invalid request parameters"
        })";
    } catch (const service::ServiceException& e) {
        // 处理业务逻辑错误
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        std::string error_message = R"({
            "error": "BAD_REQUEST",
            "message": ")" + std::string(e.what()) + R"("
        })";
        response.body() = error_message;
    } catch (const std::exception& e) {
        // 处理其他异常
        std::cerr << "Error handling search snippets request: " << e.what() << std::endl;
        response.result(server::http::status::internal_server_error);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INTERNAL_SERVER_ERROR",
            "message": "An internal server error occurred"
        })";
    }
}

void SnippetController::handleStarSnippet(const server::http::request<server::http::string_body>& request, 
                                             server::http::response<server::http::string_body>& response) {
    try {
        // 从请求路径中提取 snippet ID
        std::string path = std::string(request.target());
        size_t star_pos = path.find("/star");
        if (star_pos == std::string::npos) {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "INVALID_PARAMS",
                "message": "Invalid snippet ID"
            })";
            return;
        }
        std::string id_str = path.substr(0, star_pos);
        size_t last_slash_pos = id_str.find_last_of("/");
        if (last_slash_pos == std::string::npos) {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "INVALID_PARAMS",
                "message": "Invalid snippet ID"
            })";
            return;
        }
        id_str = id_str.substr(last_slash_pos + 1);
        int id = std::stoi(id_str);

        // 验证用户身份
        std::optional<model::User> user = authenticateUser(request);
        if (!user) {
            response.result(server::http::status::unauthorized);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "UNAUTHORIZED",
                "message": "Invalid or missing token"
            })";
            return;
        }

        // 调用 SnippetService 中的 starSnippet 方法
        snippet_service_->starSnippet(id, user->id());

        // 设置响应
        response.result(server::http::status::no_content);
    } catch (const std::invalid_argument& e) {
        // 处理无效的 ID
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INVALID_PARAMS",
            "message": "Invalid snippet ID"
        })";
    } catch (const service::ServiceException& e) {
        // 处理业务逻辑错误
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        std::string error_message = R"({
            "error": "BAD_REQUEST",
            "message": ")" + std::string(e.what()) + R"("
        })";
        response.body() = error_message;
    } catch (const std::exception& e) {
        // 处理其他异常
        std::cerr << "Error handling star snippet request: " << e.what() << std::endl;
        response.result(server::http::status::internal_server_error);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INTERNAL_SERVER_ERROR",
            "message": "An internal server error occurred"
        })";
    }
}

void SnippetController::handleUnstarSnippet(const server::http::request<server::http::string_body>& request, 
                                               server::http::response<server::http::string_body>& response) {
    try {
        // 从请求路径中提取 snippet ID
        std::string path = std::string(request.target());
        size_t star_pos = path.find("/star");
        if (star_pos == std::string::npos) {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "INVALID_PARAMS",
                "message": "Invalid snippet ID"
            })";
            return;
        }
        std::string id_str = path.substr(0, star_pos);
        size_t last_slash_pos = id_str.find_last_of("/");
        if (last_slash_pos == std::string::npos) {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "INVALID_PARAMS",
                "message": "Invalid snippet ID"
            })";
            return;
        }
        id_str = id_str.substr(last_slash_pos + 1);
        int id = std::stoi(id_str);

        // 验证用户身份
        std::optional<model::User> user = authenticateUser(request);
        if (!user) {
            response.result(server::http::status::unauthorized);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "UNAUTHORIZED",
                "message": "Invalid or missing token"
            })";
            return;
        }

        // 调用 SnippetService 中的 unstarSnippet 方法
        snippet_service_->unstarSnippet(id, user->id());

        // 设置响应
        response.result(server::http::status::no_content);
    } catch (const std::invalid_argument& e) {
        // 处理无效的 ID
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INVALID_PARAMS",
            "message": "Invalid snippet ID"
        })";
    } catch (const service::ServiceException& e) {
        // 处理业务逻辑错误
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "BAD_REQUEST",
            "message": ")" + std::string(e.what()) + R"("
        })";
    } catch (const std::exception& e) {
        // 处理其他异常
        std::cerr << "Error handling unstar snippet request: " << e.what() << std::endl;
        response.result(server::http::status::internal_server_error);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INTERNAL_SERVER_ERROR",
            "message": "An internal server error occurred"
        })";
    }
}

void SnippetController::handleGetUserSnippets(const server::http::request<server::http::string_body>& request, 
                                                 server::http::response<server::http::string_body>& response) {
    try {
        // 从请求路径中提取用户 ID
        std::string path = std::string(request.target());
        size_t snippets_pos = path.find("/snippets");
        if (snippets_pos == std::string::npos) {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "INVALID_PARAMS",
                "message": "Invalid user ID"
            })";
            return;
        }
        std::string id_str = path.substr(0, snippets_pos);
        size_t last_slash_pos = id_str.find_last_of("/");
        if (last_slash_pos == std::string::npos) {
            response.result(server::http::status::bad_request);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "INVALID_PARAMS",
                "message": "Invalid user ID"
            })";
            return;
        }
        id_str = id_str.substr(last_slash_pos + 1);
        int user_id = std::stoi(id_str);

        // 从请求参数中提取分页参数
        int page = 1;
        int page_size = 20;

        std::string target = std::string(request.target());
        size_t query_pos = target.find("?");
        if (query_pos != std::string::npos) {
            std::string query_string = target.substr(query_pos + 1);
            std::istringstream iss(query_string);
            std::string param;
            while (std::getline(iss, param, '&')) {
                size_t equal_pos = param.find("=");
                if (equal_pos == std::string::npos) {
                    continue;
                }
                std::string key = param.substr(0, equal_pos);
                std::string value = param.substr(equal_pos + 1);

                if (key == "page") {
                    page = std::stoi(value);
                } else if (key == "page_size") {
                    page_size = std::stoi(value);
                }
            }
        }

        // 验证用户身份
        std::optional<model::User> current_user = authenticateUser(request);
        if (!current_user) {
            response.result(server::http::status::unauthorized);
            response.set(server::http::field::content_type, "application/json");
            response.body() = R"({
                "error": "UNAUTHORIZED",
                "message": "Invalid or missing token"
            })";
            return;
        }

        // 调用 SnippetService 中的 getUserSnippets 方法
        service::SnippetService::SearchResult search_result = snippet_service_->getUserSnippets(user_id, current_user->id(), page, page_size);

        // 生成响应体
        json response_body;
        response_body["total"] = search_result.total;
        response_body["page"] = page;
        response_body["page_size"] = page_size;
        response_body["snippets"] = json::array();

        for (const auto& snippet : search_result.items) {
            json snippet_json;
            snippet_json["id"] = snippet.id();
            snippet_json["owner_id"] = snippet.owner_id();
            snippet_json["title"] = snippet.title();
            snippet_json["language"] = snippet.language();
            snippet_json["content"] = snippet.content();
            snippet_json["tags"] = snippet.tags();
            snippet_json["is_public"] = snippet.is_public();
            snippet_json["created_at"] = snippet.created_at().time_since_epoch().count();
            snippet_json["updated_at"] = snippet.updated_at().time_since_epoch().count();
            snippet_json["star_count"] = snippet.star_count();
            response_body["snippets"].push_back(snippet_json);
        }

        // 设置响应
        response.result(server::http::status::ok);
        response.set(server::http::field::content_type, "application/json");
        response.body() = response_body.dump();
    } catch (const std::invalid_argument& e) {
        // 处理无效的参数
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        std::string error_message = R"({
            "error": "BAD_REQUEST",
            "message": ")" + std::string(e.what()) + R"("
        })";
        response.body() = error_message;
    } catch (const service::ServiceException& e) {
        // 处理业务逻辑错误
        response.result(server::http::status::bad_request);
        response.set(server::http::field::content_type, "application/json");
        std::string error_message = R"({
            "error": "BAD_REQUEST",
            "message": ")" + std::string(e.what()) + R"("
        })";
        response.body() = error_message;
    } catch (const std::exception& e) {
        // 处理其他异常
        std::cerr << "Error handling get user snippets request: " << e.what() << std::endl;
        response.result(server::http::status::internal_server_error);
        response.set(server::http::field::content_type, "application/json");
        response.body() = R"({
            "error": "INTERNAL_SERVER_ERROR",
            "message": "An internal server error occurred"
        })";
    }
}

} // namespace controller