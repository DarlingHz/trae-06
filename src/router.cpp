#include "router.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <optional>

using json = nlohmann::json;

Router::Router(std::shared_ptr<Service> service) : service_(service) {
}

void Router::init(httplib::Server& server) {
    // 用户管理相关路由
    server.Post("/users", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleCreateUser(req, res);
    });

    // 文档管理相关路由
    server.Post("/documents", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleCreateDocument(req, res);
    });

    server.Get("/documents", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetDocuments(req, res);
    });

    server.Get("/documents/(\\d+)", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetDocumentDetail(req, res);
    });

    // 文档版本管理相关路由
    server.Post("/documents/(\\d+)/versions", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleCreateDocumentVersion(req, res);
    });

    server.Get("/documents/(\\d+)/versions", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetDocumentVersions(req, res);
    });

    server.Get("/documents/(\\d+)/versions/(\\d+)", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetDocumentVersion(req, res);
    });

    // 评论管理相关路由
    server.Post("/documents/(\\d+)/comments", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleCreateComment(req, res);
    });

    server.Get("/documents/(\\d+)/comments", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetComments(req, res);
    });

    // 统计与健康检查相关路由
    server.Get("/metrics", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetMetrics(req, res);
    });

    server.Get("/healthz", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleHealthCheck(req, res);
    });
}

// 用户管理相关路由
void Router::handleCreateUser(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析请求体
        json req_body = json::parse(req.body);

        // 验证请求参数
        if (!req_body.contains("name") || !req_body.contains("email")) {
            sendErrorResponse(res, 1001, "Missing required parameters: name and email");
            return;
        }

        std::string name = req_body["name"].get<std::string>();
        std::string email = req_body["email"].get<std::string>();

        // 调用业务逻辑层方法
        auto user = service_->createUser(name, email);

        if (!user) {
            sendErrorResponse(res, 1002, "Failed to create user");
            return;
        }

        // 构建响应
        json res_body;
        res_body["id"] = user->getId();
        res_body["name"] = user->getName();
        res_body["email"] = user->getEmail();
        res_body["created_at"] = user->getCreatedAt();

        sendSuccessResponse(res, res_body.dump());
    } catch (const json::parse_error& e) {
        sendErrorResponse(res, 1000, "Invalid JSON format in request body");
    } catch (const std::exception& e) {
        sendErrorResponse(res, 500, "Internal server error");
        std::cerr << "Error handling create user request: " << e.what() << std::endl;
    }
}

// 文档管理相关路由
void Router::handleCreateDocument(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析请求体
        json req_body = json::parse(req.body);

        // 验证请求参数
        if (!req_body.contains("owner_id") || !req_body.contains("title")) {
            sendErrorResponse(res, 2001, "Missing required parameters: owner_id and title");
            return;
        }

        int owner_id = req_body["owner_id"].get<int>();
        std::string title = req_body["title"].get<std::string>();
        std::vector<std::string> tags;

        if (req_body.contains("tags")) {
            tags = req_body["tags"].get<std::vector<std::string>>();
        }

        // 调用业务逻辑层方法
        auto document = service_->createDocument(owner_id, title, tags);

        if (!document) {
            sendErrorResponse(res, 2002, "Failed to create document");
            return;
        }

        // 构建响应
        json res_body;
        res_body["id"] = document->getId();
        res_body["owner_id"] = document->getOwnerId();
        res_body["title"] = document->getTitle();
        res_body["tags"] = document->getTags();
        res_body["created_at"] = document->getCreatedAt();
        res_body["updated_at"] = document->getUpdatedAt();

        sendSuccessResponse(res, res_body.dump());
    } catch (const json::parse_error& e) {
        sendErrorResponse(res, 1000, "Invalid JSON format in request body");
    } catch (const std::exception& e) {
        sendErrorResponse(res, 500, "Internal server error");
        std::cerr << "Error handling create document request: " << e.what() << std::endl;
    }
}

void Router::handleGetDocuments(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析查询参数
        std::optional<int> owner_id;
        std::optional<std::string> tag;
        std::optional<std::string> keyword;
        int page = 1;
        int page_size = 10;

        if (req.has_param("owner_id")) {
            owner_id = std::stoi(req.get_param_value("owner_id"));
        }

        if (req.has_param("tag")) {
            tag = req.get_param_value("tag");
        }

        if (req.has_param("keyword")) {
            keyword = req.get_param_value("keyword");
        }

        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
        }

        if (req.has_param("page_size")) {
            page_size = std::stoi(req.get_param_value("page_size"));
        }

        // 调用业务逻辑层方法
        auto result = service_->getDocuments(owner_id, tag, keyword, page, page_size);

        // 构建响应
        json res_body;
        json items;

        for (const auto& document : result.getItems()) {
            json item;
            item["id"] = document.getId();
            item["owner_id"] = document.getOwnerId();
            item["title"] = document.getTitle();
            item["tags"] = document.getTags();
            item["updated_at"] = document.getUpdatedAt();
            items.push_back(item);
        }

        res_body["items"] = items;
        res_body["page"] = result.getPage();
        res_body["page_size"] = result.getPageSize();
        res_body["total"] = result.getTotal();

        sendSuccessResponse(res, res_body.dump());
    } catch (const std::exception& e) {
        sendErrorResponse(res, 500, "Internal server error");
        std::cerr << "Error handling get documents request: " << e.what() << std::endl;
    }
}

void Router::handleGetDocumentDetail(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析文档ID
        int document_id = std::stoi(req.matches[1]);

        // 调用业务逻辑层方法
        auto document_detail = service_->getDocumentDetailById(document_id);

        if (!document_detail) {
            sendErrorResponse(res, 2003, "Document not found", 404);
            return;
        }

        // 构建响应
        json res_body;
        const auto& document = document_detail->getDocument();
        const auto& latest_version = document_detail->getLatestVersion();

        res_body["id"] = document.getId();
        res_body["owner_id"] = document.getOwnerId();
        res_body["title"] = document.getTitle();
        res_body["tags"] = document.getTags();
        res_body["created_at"] = document.getCreatedAt();
        res_body["updated_at"] = document.getUpdatedAt();
        res_body["latest_version"] = {
            {"version_number", latest_version.getVersionNumber()},
            {"content", latest_version.getContent()},
            {"created_at", latest_version.getCreatedAt()}
        };

        sendSuccessResponse(res, res_body.dump());
    } catch (const std::invalid_argument& e) {
        sendErrorResponse(res, 2004, "Invalid document ID");
    } catch (const std::exception& e) {
        sendErrorResponse(res, 500, "Internal server error");
        std::cerr << "Error handling get document detail request: " << e.what() << std::endl;
    }
}

// 文档版本管理相关路由
void Router::handleCreateDocumentVersion(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析文档ID
        int document_id = std::stoi(req.matches[1]);

        // 解析请求体
        json req_body = json::parse(req.body);

        // 验证请求参数
        if (!req_body.contains("content")) {
            sendErrorResponse(res, 3001, "Missing required parameter: content");
            return;
        }

        std::string content = req_body["content"].get<std::string>();

        // 调用业务逻辑层方法
        auto version = service_->createDocumentVersion(document_id, content);

        if (!version) {
            sendErrorResponse(res, 3002, "Failed to create document version");
            return;
        }

        // 构建响应
        json res_body;
        res_body["id"] = version->getId();
        res_body["document_id"] = version->getDocumentId();
        res_body["version_number"] = version->getVersionNumber();
        res_body["content"] = version->getContent();
        res_body["created_at"] = version->getCreatedAt();

        sendSuccessResponse(res, res_body.dump());
    } catch (const json::parse_error& e) {
        sendErrorResponse(res, 1000, "Invalid JSON format in request body");
    } catch (const std::invalid_argument& e) {
        sendErrorResponse(res, 2004, "Invalid document ID");
    } catch (const std::exception& e) {
        sendErrorResponse(res, 500, "Internal server error");
        std::cerr << "Error handling create document version request: " << e.what() << std::endl;
    }
}

void Router::handleGetDocumentVersions(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析文档ID
        int document_id = std::stoi(req.matches[1]);

        // 解析查询参数
        int page = 1;
        int page_size = 10;
        bool order_by_version = true;

        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
        }

        if (req.has_param("page_size")) {
            page_size = std::stoi(req.get_param_value("page_size"));
        }

        if (req.has_param("order")) {
            std::string order = req.get_param_value("order");
            order_by_version = (order == "version");
        }

        // 调用业务逻辑层方法
        auto result = service_->getDocumentVersions(document_id, page, page_size, order_by_version);

        // 构建响应
        json res_body;
        json items;

        for (const auto& version : result.getItems()) {
            json item;
            item["id"] = version.getId();
            item["version_number"] = version.getVersionNumber();
            item["created_at"] = version.getCreatedAt();
            items.push_back(item);
        }

        res_body["items"] = items;
        res_body["page"] = result.getPage();
        res_body["page_size"] = result.getPageSize();
        res_body["total"] = result.getTotal();

        sendSuccessResponse(res, res_body.dump());
    } catch (const std::invalid_argument& e) {
        sendErrorResponse(res, 2004, "Invalid document ID");
    } catch (const std::exception& e) {
        sendErrorResponse(res, 500, "Internal server error");
        std::cerr << "Error handling get document versions request: " << e.what() << std::endl;
    }
}

void Router::handleGetDocumentVersion(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析文档ID和版本号
        int document_id = std::stoi(req.matches[1]);
        int version_number = std::stoi(req.matches[2]);

        // 调用业务逻辑层方法
        auto version = service_->getDocumentVersionByNumber(document_id, version_number);

        if (!version) {
            sendErrorResponse(res, 3003, "Document version not found", 404);
            return;
        }

        // 构建响应
        json res_body;
        res_body["id"] = version->getId();
        res_body["document_id"] = version->getDocumentId();
        res_body["version_number"] = version->getVersionNumber();
        res_body["content"] = version->getContent();
        res_body["created_at"] = version->getCreatedAt();

        sendSuccessResponse(res, res_body.dump());
    } catch (const std::invalid_argument& e) {
        sendErrorResponse(res, 3004, "Invalid document ID or version number");
    } catch (const std::exception& e) {
        sendErrorResponse(res, 500, "Internal server error");
        std::cerr << "Error handling get document version request: " << e.what() << std::endl;
    }
}

// 评论管理相关路由
void Router::handleCreateComment(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析文档ID
        int document_id = std::stoi(req.matches[1]);

        // 解析请求体
        json req_body = json::parse(req.body);

        // 验证请求参数
        if (!req_body.contains("author_id") || !req_body.contains("content")) {
            sendErrorResponse(res, 4001, "Missing required parameters: author_id and content");
            return;
        }

        int author_id = req_body["author_id"].get<int>();
        std::string content = req_body["content"].get<std::string>();
        std::optional<int> version_number;

        if (req_body.contains("version_number")) {
            version_number = req_body["version_number"].get<int>();
        }

        // 调用业务逻辑层方法
        auto comment = service_->createComment(document_id, author_id, content, version_number);

        if (!comment) {
            sendErrorResponse(res, 4002, "Failed to create comment");
            return;
        }

        // 构建响应
        json res_body;
        res_body["id"] = comment->getId();
        res_body["document_id"] = comment->getDocumentId();
        res_body["author_id"] = comment->getAuthorId();
        res_body["content"] = comment->getContent();
        res_body["created_at"] = comment->getCreatedAt();

        if (comment->getVersionNumber()) {
            res_body["version_number"] = *comment->getVersionNumber();
        }

        sendSuccessResponse(res, res_body.dump());
    } catch (const json::parse_error& e) {
        sendErrorResponse(res, 1000, "Invalid JSON format in request body");
    } catch (const std::invalid_argument& e) {
        sendErrorResponse(res, 2004, "Invalid document ID");
    } catch (const std::exception& e) {
        sendErrorResponse(res, 500, "Internal server error");
        std::cerr << "Error handling create comment request: " << e.what() << std::endl;
    }
}

void Router::handleGetComments(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析文档ID
        int document_id = std::stoi(req.matches[1]);

        // 解析查询参数
        std::optional<int> version_number;
        int page = 1;
        int page_size = 10;

        if (req.has_param("version_number")) {
            version_number = std::stoi(req.get_param_value("version_number"));
        }

        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
        }

        if (req.has_param("page_size")) {
            page_size = std::stoi(req.get_param_value("page_size"));
        }

        // 调用业务逻辑层方法
        auto result = service_->getComments(document_id, version_number, page, page_size);

        // 构建响应
        json res_body;
        json items;

        for (const auto& comment : result.getItems()) {
            json item;
            item["id"] = comment.getId();
            item["document_id"] = comment.getDocumentId();
            item["author_id"] = comment.getAuthorId();
            item["content"] = comment.getContent();
            item["created_at"] = comment.getCreatedAt();

            if (comment.getVersionNumber()) {
                item["version_number"] = *comment.getVersionNumber();
            }

            items.push_back(item);
        }

        res_body["items"] = items;
        res_body["page"] = result.getPage();
        res_body["page_size"] = result.getPageSize();
        res_body["total"] = result.getTotal();

        sendSuccessResponse(res, res_body.dump());
    } catch (const std::invalid_argument& e) {
        sendErrorResponse(res, 2004, "Invalid document ID");
    } catch (const std::exception& e) {
        sendErrorResponse(res, 500, "Internal server error");
        std::cerr << "Error handling get comments request: " << e.what() << std::endl;
    }
}

// 统计与健康检查相关路由
void Router::handleGetMetrics(const httplib::Request& req, httplib::Response& res) {
    try {
        // 调用业务逻辑层方法
        auto metrics = service_->getMetrics();

        // 构建响应
        json res_body;
        res_body["total_users"] = metrics.getTotalUsers();
        res_body["total_documents"] = metrics.getTotalDocuments();
        res_body["total_versions"] = metrics.getTotalVersions();
        res_body["total_comments"] = metrics.getTotalComments();

        json top_documents;
        for (const auto& doc : metrics.getTopDocumentsByVersions()) {
            json item;
            item["document_id"] = doc.first;
            item["version_count"] = doc.second;
            top_documents.push_back(item);
        }

        res_body["top_documents_by_versions"] = top_documents;

        sendSuccessResponse(res, res_body.dump());
    } catch (const std::exception& e) {
        sendErrorResponse(res, 500, "Internal server error");
        std::cerr << "Error handling get metrics request: " << e.what() << std::endl;
    }
}

void Router::handleHealthCheck(const httplib::Request& req, httplib::Response& res) {
    try {
        // 构建响应
        json res_body;
        res_body["status"] = "ok";

        sendSuccessResponse(res, res_body.dump());
    } catch (const std::exception& e) {
        sendErrorResponse(res, 500, "Internal server error");
        std::cerr << "Error handling health check request: " << e.what() << std::endl;
    }
}

// 辅助方法
void Router::sendSuccessResponse(httplib::Response& res, const std::string& data) {
    res.set_content(data, "application/json");
    res.status = 200;
}

void Router::sendErrorResponse(httplib::Response& res, int error_code, const std::string& message, int status_code) {
    json res_body;
    res_body["error_code"] = error_code;
    res_body["message"] = message;

    res.set_content(res_body.dump(), "application/json");
    res.status = status_code;
}