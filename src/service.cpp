#include "service.h"
#include <iostream>

Service::Service(std::shared_ptr<Database> database, std::shared_ptr<LRUCache<std::pair<int, int>, DocumentVersion>> cache)
    : database_(database), cache_(cache) {
}

// 用户管理相关方法
std::optional<User> Service::createUser(const std::string& name, const std::string& email) {
    if (name.empty() || email.empty()) {
        std::cerr << "Name and email cannot be empty" << std::endl;
        return std::nullopt;
    }

    User user;
    user.setName(name);
    user.setEmail(email);

    return database_->createUser(user);
}

std::optional<User> Service::getUserById(int id) {
    if (id <= 0) {
        std::cerr << "Invalid user ID" << std::endl;
        return std::nullopt;
    }

    return database_->getUserById(id);
}

// 文档管理相关方法
std::optional<Document> Service::createDocument(int owner_id, const std::string& title, const std::vector<std::string>& tags) {
    if (owner_id <= 0) {
        std::cerr << "Invalid owner ID" << std::endl;
        return std::nullopt;
    }

    if (title.empty()) {
        std::cerr << "Title cannot be empty" << std::endl;
        return std::nullopt;
    }

    // 检查用户是否存在
    auto user = database_->getUserById(owner_id);
    if (!user) {
        std::cerr << "User not found: " << owner_id << std::endl;
        return std::nullopt;
    }

    Document document;
    document.setOwnerId(owner_id);
    document.setTitle(title);
    document.setTags(tags);

    return database_->createDocument(document);
}

std::optional<DocumentDetail> Service::getDocumentDetailById(int id) {
    if (id <= 0) {
        std::cerr << "Invalid document ID" << std::endl;
        return std::nullopt;
    }

    // 获取文档基础信息
    auto document = database_->getDocumentById(id);
    if (!document) {
        std::cerr << "Document not found: " << id << std::endl;
        return std::nullopt;
    }

    // 获取文档最新版本
    auto latest_version = database_->getLatestDocumentVersion(id);
    if (!latest_version) {
        std::cerr << "No versions found for document: " << id << std::endl;
        return std::nullopt;
    }

    // 构建文档详情对象
    DocumentDetail document_detail(*document, *latest_version);

    return document_detail;
}

PaginationResult<Document> Service::getDocuments(std::optional<int> owner_id,
                                                   std::optional<std::string> tag,
                                                   std::optional<std::string> keyword,
                                                   int page,
                                                   int page_size) {
    if (page <= 0) {
        page = 1;
    }

    if (page_size <= 0) {
        page_size = 10;
    }

    return database_->getDocuments(owner_id, tag, keyword, page, page_size);
}

// 文档版本管理相关方法
std::optional<DocumentVersion> Service::createDocumentVersion(int document_id, const std::string& content) {
    if (document_id <= 0) {
        std::cerr << "Invalid document ID" << std::endl;
        return std::nullopt;
    }

    if (content.empty()) {
        std::cerr << "Content cannot be empty" << std::endl;
        return std::nullopt;
    }

    // 检查文档是否存在
    auto document = database_->getDocumentById(document_id);
    if (!document) {
        std::cerr << "Document not found: " << document_id << std::endl;
        return std::nullopt;
    }

    DocumentVersion version;
    version.setDocumentId(document_id);
    version.setContent(content);

    auto created_version = database_->createDocumentVersion(version);

    // 如果创建成功，将新版本写入缓存
    if (created_version) {
        cache_->put({document_id, created_version->getVersionNumber()}, *created_version);
    }

    return created_version;
}

PaginationResult<DocumentVersion> Service::getDocumentVersions(int document_id,
                                                                   int page,
                                                                   int page_size,
                                                                   bool order_by_version) {
    if (document_id <= 0) {
        std::cerr << "Invalid document ID" << std::endl;
        return PaginationResult<DocumentVersion>();
    }

    if (page <= 0) {
        page = 1;
    }

    if (page_size <= 0) {
        page_size = 10;
    }

    return database_->getDocumentVersions(document_id, page, page_size, order_by_version);
}

std::optional<DocumentVersion> Service::getDocumentVersionByNumber(int document_id, int version_number) {
    if (document_id <= 0) {
        std::cerr << "Invalid document ID" << std::endl;
        return std::nullopt;
    }

    if (version_number <= 0) {
        std::cerr << "Invalid version number" << std::endl;
        return std::nullopt;
    }

    // 构建缓存键
    auto cache_key = std::make_pair(document_id, version_number);

    // 优先从缓存读取
    auto cached_version = cache_->get(cache_key);
    if (cached_version) {
        return cached_version;
    }

    // 缓存未命中，从数据库读取
    auto version = database_->getDocumentVersionByNumber(document_id, version_number);

    // 如果读取成功，将版本写入缓存
    if (version) {
        cache_->put(cache_key, *version);
    }

    return version;
}

// 评论管理相关方法
std::optional<Comment> Service::createComment(int document_id, int author_id, const std::string& content, std::optional<int> version_number) {
    if (document_id <= 0) {
        std::cerr << "Invalid document ID" << std::endl;
        return std::nullopt;
    }

    if (author_id <= 0) {
        std::cerr << "Invalid author ID" << std::endl;
        return std::nullopt;
    }

    if (content.empty()) {
        std::cerr << "Content cannot be empty" << std::endl;
        return std::nullopt;
    }

    // 检查文档是否存在
    auto document = database_->getDocumentById(document_id);
    if (!document) {
        std::cerr << "Document not found: " << document_id << std::endl;
        return std::nullopt;
    }

    // 如果指定了版本号，检查版本是否存在
    if (version_number) {
        auto version = database_->getDocumentVersionByNumber(document_id, *version_number);
        if (!version) {
            std::cerr << "Document version not found: " << document_id << " - " << *version_number << std::endl;
            return std::nullopt;
        }
    }

    // 检查用户是否存在
    auto user = database_->getUserById(author_id);
    if (!user) {
        std::cerr << "User not found: " << author_id << std::endl;
        return std::nullopt;
    }

    Comment comment;
    comment.setDocumentId(document_id);
    comment.setAuthorId(author_id);
    comment.setContent(content);
    comment.setVersionNumber(version_number);

    return database_->createComment(comment);
}

PaginationResult<Comment> Service::getComments(int document_id,
                                                 std::optional<int> version_number,
                                                 int page,
                                                 int page_size) {
    if (document_id <= 0) {
        std::cerr << "Invalid document ID" << std::endl;
        return PaginationResult<Comment>();
    }

    if (page <= 0) {
        page = 1;
    }

    if (page_size <= 0) {
        page_size = 10;
    }

    return database_->getComments(document_id, version_number, page, page_size);
}

// 统计相关方法
Metrics Service::getMetrics() {
    return database_->getMetrics();
}