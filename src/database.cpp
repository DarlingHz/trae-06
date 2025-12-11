#include "database.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstring>

Database::Database(const std::string& db_path) : db_path_(db_path) {
}

Database::~Database() {
    close();
}

bool Database::open() {
    if (is_open_) {
        return true;
    }

    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    is_open_ = true;
    return true;
}

void Database::close() {
    if (is_open_ && db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
        is_open_ = false;
    }
}

bool Database::init(const std::string& init_sql_file) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return false;
    }

    // 读取初始化SQL文件
    std::ifstream ifs(init_sql_file);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open init SQL file: " << init_sql_file << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << ifs.rdbuf();
    std::string sql = buffer.str();

    // 执行初始化SQL
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to initialize database: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool Database::execute(const std::string& sql) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return false;
    }

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to execute SQL: " << err_msg << std::endl;
        std::cerr << "SQL: " << sql << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

// 静态回调函数，用于将std::function转换为普通函数指针
static int static_callback(void* data, int argc, char** argv, char** col_names) {
    auto callback = static_cast<std::function<int(void*, int, char**, char**)>*>(data);
    return (*callback)(data, argc, argv, col_names);
}

bool Database::query(const std::string& sql, std::function<int(void*, int, char**, char**)> callback, void* user_data) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return false;
    }

    char* err_msg = nullptr;
    // 将std::function对象作为data参数传递给sqlite3_exec函数
    int rc = sqlite3_exec(db_, sql.c_str(), static_callback, &callback, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to query SQL: " << err_msg << std::endl;
        std::cerr << "SQL: " << sql << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

int Database::getLastInsertId() {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return 0;
    }

    return sqlite3_last_insert_rowid(db_);
}

std::string Database::tagsToString(const std::vector<std::string>& tags) {
    std::string tags_str;
    for (size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) {
            tags_str += ",";
        }
        tags_str += tags[i];
    }
    return tags_str;
}

std::vector<std::string> Database::stringToTags(const std::string& tags_str) {
    std::vector<std::string> tags;
    std::stringstream ss(tags_str);
    std::string tag;

    while (std::getline(ss, tag, ',')) {
        if (!tag.empty()) {
            tags.push_back(tag);
        }
    }

    return tags;
}

// 用户相关操作
std::optional<User> Database::createUser(const User& user) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return std::nullopt;
    }

    // 检查邮箱是否已存在
    auto existing_user = getUserByEmail(user.getEmail());
    if (existing_user) {
        std::cerr << "User with email already exists: " << user.getEmail() << std::endl;
        return std::nullopt;
    }

    // 准备SQL语句
    std::string sql = "INSERT INTO users (name, email) VALUES ('";
    sql += user.getName();
    sql += "', '";
    sql += user.getEmail();
    sql += "')";

    // 执行SQL语句
    if (!execute(sql)) {
        return std::nullopt;
    }

    // 获取新创建的用户ID
    int user_id = getLastInsertId();

    // 查询新创建的用户信息
    return getUserById(user_id);
}

std::optional<User> Database::getUserById(int id) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return std::nullopt;
    }

    // 准备SQL语句
    std::string sql = "SELECT * FROM users WHERE id = " + std::to_string(id);

    // 存储查询结果
    User user;
    bool found = false;

    // 执行查询
    if (!query(sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto [user_ptr, found_ptr] = *static_cast<std::pair<User*, bool*>*>(data);

        // 填充用户对象
        for (int i = 0; i < argc; ++i) {
            std::string col_name = col_names[i];
            std::string col_value = argv[i] ? argv[i] : "";

            if (col_name == "id") {
                user_ptr->setId(std::stoi(col_value));
            } else if (col_name == "name") {
                user_ptr->setName(col_value);
            } else if (col_name == "email") {
                user_ptr->setEmail(col_value);
            } else if (col_name == "created_at") {
                user_ptr->setCreatedAt(col_value);
            }
        }

        *found_ptr = true;
        return 0;
    }, new std::pair<User*, bool*>{&user, &found})) {
        return std::nullopt;
    }

    // 清理内存 - 不需要手动清理，因为我们使用的是栈上的对象

    if (found) {
        return user;
    } else {
        return std::nullopt;
    }
}

std::optional<User> Database::getUserByEmail(const std::string& email) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return std::nullopt;
    }

    // 准备SQL语句
    std::string sql = "SELECT * FROM users WHERE email = '" + email + "'";

    // 存储查询结果
    User user;
    bool found = false;

    // 执行查询
    if (!query(sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto [user_ptr, found_ptr] = *static_cast<std::pair<User*, bool*>*>(data);

        // 填充用户对象
        for (int i = 0; i < argc; ++i) {
            std::string col_name = col_names[i];
            std::string col_value = argv[i] ? argv[i] : "";

            if (col_name == "id") {
                user_ptr->setId(std::stoi(col_value));
            } else if (col_name == "name") {
                user_ptr->setName(col_value);
            } else if (col_name == "email") {
                user_ptr->setEmail(col_value);
            } else if (col_name == "created_at") {
                user_ptr->setCreatedAt(col_value);
            }
        }

        *found_ptr = true;
        return 0;
    }, new std::pair<User*, bool*>{&user, &found})) {
        return std::nullopt;
    }

    // 清理内存 - 不需要手动清理，因为我们使用的是栈上的对象

    if (found) {
        return user;
    } else {
        return std::nullopt;
    }
}

// 文档相关操作
std::optional<Document> Database::createDocument(const Document& document) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return std::nullopt;
    }

    // 检查用户是否存在
    auto user = getUserById(document.getOwnerId());
    if (!user) {
        std::cerr << "User not found: " << document.getOwnerId() << std::endl;
        return std::nullopt;
    }

    // 准备SQL语句
    std::string sql = "INSERT INTO documents (owner_id, title, tags) VALUES (";
    sql += std::to_string(document.getOwnerId());
    sql += ", '";
    sql += document.getTitle();
    sql += "', '";
    sql += tagsToString(document.getTags());
    sql += "')";

    // 执行SQL语句
    if (!execute(sql)) {
        return std::nullopt;
    }

    // 获取新创建的文档ID
    int document_id = getLastInsertId();

    // 查询新创建的文档信息
    return getDocumentById(document_id);
}

std::optional<Document> Database::getDocumentById(int id) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return std::nullopt;
    }

    // 准备SQL语句
    std::string sql = "SELECT * FROM documents WHERE id = " + std::to_string(id);

    // 存储查询结果
    Document document;
    bool found = false;

    // 执行查询
    auto data_ptr = new std::pair<Document*, bool*>{&document, &found};
    if (!query(sql, [this](void* data, int argc, char** argv, char** col_names) -> int {
        auto [document_ptr, found_ptr] = *static_cast<std::pair<Document*, bool*>*>(data);

        // 填充文档对象
        for (int i = 0; i < argc; ++i) {
            std::string col_name = col_names[i];
            std::string col_value = argv[i] ? argv[i] : "";

            if (col_name == "id") {
                document_ptr->setId(std::stoi(col_value));
            } else if (col_name == "owner_id") {
                document_ptr->setOwnerId(std::stoi(col_value));
            } else if (col_name == "title") {
                document_ptr->setTitle(col_value);
            } else if (col_name == "tags") {
                document_ptr->setTags(this->stringToTags(col_value));
            } else if (col_name == "created_at") {
                document_ptr->setCreatedAt(col_value);
            } else if (col_name == "updated_at") {
                document_ptr->setUpdatedAt(col_value);
            }
        }

        *found_ptr = true;
        return 0;
    }, data_ptr)) {
        delete data_ptr;
        return std::nullopt;
    }

    // 清理内存
    delete data_ptr;

    if (found) {
        return document;
    } else {
        return std::nullopt;
    }
}

PaginationResult<Document> Database::getDocuments(std::optional<int> owner_id,
                                                      std::optional<std::string> tag,
                                                      std::optional<std::string> keyword,
                                                      int page,
                                                      int page_size) {
    PaginationResult<Document> result;

    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return result;
    }

    // 构建查询条件
    std::string where_clause = "WHERE 1=1";

    if (owner_id) {
        where_clause += " AND owner_id = " + std::to_string(*owner_id);
    }

    if (tag) {
        where_clause += " AND tags LIKE '%" + *tag + "%'";
    }

    if (keyword) {
        where_clause += " AND title LIKE '%" + *keyword + "%'";
    }

    // 获取总记录数
    std::string count_sql = "SELECT COUNT(*) FROM documents " + where_clause;
    int total = 0;

    if (!query(count_sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto total_ptr = static_cast<int*>(data);
        *total_ptr = std::stoi(argv[0]);
        return 0;
    }, &total)) {
        return result;
    }

    // 计算分页参数
    int offset = (page - 1) * page_size;

    // 构建查询SQL
    std::string sql = "SELECT * FROM documents " + where_clause + " ORDER BY updated_at DESC LIMIT " + std::to_string(page_size) + " OFFSET " + std::to_string(offset);

    // 存储查询结果
    std::vector<Document> documents;

    // 执行查询
    if (!query(sql, [this](void* data, int argc, char** argv, char** col_names) -> int {
        auto documents_ptr = static_cast<std::vector<Document>*>(data);
        Document document;

        // 填充文档对象
        for (int i = 0; i < argc; ++i) {
            std::string col_name = col_names[i];
            std::string col_value = argv[i] ? argv[i] : "";

            if (col_name == "id") {
                document.setId(std::stoi(col_value));
            } else if (col_name == "owner_id") {
                document.setOwnerId(std::stoi(col_value));
            } else if (col_name == "title") {
                document.setTitle(col_value);
            } else if (col_name == "tags") {
                document.setTags(this->stringToTags(col_value));
            } else if (col_name == "created_at") {
                document.setCreatedAt(col_value);
            } else if (col_name == "updated_at") {
                document.setUpdatedAt(col_value);
            }
        }

        documents_ptr->push_back(document);
        return 0;
    }, &documents)) {
        return result;
    }

    // 填充结果
    result.setItems(documents);
    result.setPage(page);
    result.setPageSize(page_size);
    result.setTotal(total);

    return result;
}

bool Database::updateDocument(const Document& document) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return false;
    }

    // 检查文档是否存在
    auto existing_document = getDocumentById(document.getId());
    if (!existing_document) {
        std::cerr << "Document not found: " << document.getId() << std::endl;
        return false;
    }

    // 准备SQL语句
    std::string sql = "UPDATE documents SET owner_id = " + std::to_string(document.getOwnerId());
    sql += ", title = '" + document.getTitle() + "'";
    sql += ", tags = '" + tagsToString(document.getTags()) + "'";
    sql += ", updated_at = CURRENT_TIMESTAMP";
    sql += " WHERE id = " + std::to_string(document.getId());

    // 执行SQL语句
    return execute(sql);
}

// 文档版本相关操作
std::optional<DocumentVersion> Database::createDocumentVersion(const DocumentVersion& version) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return std::nullopt;
    }

    // 检查文档是否存在
    auto document = getDocumentById(version.getDocumentId());
    if (!document) {
        std::cerr << "Document not found: " << version.getDocumentId() << std::endl;
        return std::nullopt;
    }

    // 获取下一个版本号
    std::string sql = "SELECT MAX(version_number) FROM document_versions WHERE document_id = " + std::to_string(version.getDocumentId());
    int next_version_number = 1;

    if (!query(sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto version_ptr = static_cast<int*>(data);
        if (argv[0] != nullptr) {
            *version_ptr = std::stoi(argv[0]) + 1;
        }
        return 0;
    }, &next_version_number)) {
        return std::nullopt;
    }

    // 准备SQL语句
    sql = "INSERT INTO document_versions (document_id, version_number, content) VALUES (";
    sql += std::to_string(version.getDocumentId());
    sql += ", " + std::to_string(next_version_number);
    sql += ", '" + version.getContent() + "')";

    // 执行SQL语句
    if (!execute(sql)) {
        return std::nullopt;
    }

    // 获取新创建的版本ID
    int version_id = getLastInsertId();

    // 更新文档的更新时间
    std::string update_doc_sql = "UPDATE documents SET updated_at = CURRENT_TIMESTAMP WHERE id = " + std::to_string(version.getDocumentId());
    execute(update_doc_sql);

    // 查询新创建的版本信息
    return getDocumentVersionById(version_id);
}

std::optional<DocumentVersion> Database::getDocumentVersionById(int id) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return std::nullopt;
    }

    // 准备SQL语句
    std::string sql = "SELECT * FROM document_versions WHERE id = " + std::to_string(id);

    // 存储查询结果
    DocumentVersion version;
    bool found = false;

    // 执行查询
    if (!query(sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto [version_ptr, found_ptr] = *static_cast<std::pair<DocumentVersion*, bool*>*>(data);

        // 填充版本对象
        for (int i = 0; i < argc; ++i) {
            std::string col_name = col_names[i];
            std::string col_value = argv[i] ? argv[i] : "";

            if (col_name == "id") {
                version_ptr->setId(std::stoi(col_value));
            } else if (col_name == "document_id") {
                version_ptr->setDocumentId(std::stoi(col_value));
            } else if (col_name == "version_number") {
                version_ptr->setVersionNumber(std::stoi(col_value));
            } else if (col_name == "content") {
                version_ptr->setContent(col_value);
            } else if (col_name == "created_at") {
                version_ptr->setCreatedAt(col_value);
            }
        }

        *found_ptr = true;
        return 0;
    }, new std::pair<DocumentVersion*, bool*>{&version, &found})) {
        return std::nullopt;
    }

    // 清理内存 - 不需要手动清理，因为我们使用的是栈上的对象

    if (found) {
        return version;
    } else {
        return std::nullopt;
    }
}

std::optional<DocumentVersion> Database::getDocumentVersionByNumber(int document_id, int version_number) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return std::nullopt;
    }

    // 准备SQL语句
    std::string sql = "SELECT * FROM document_versions WHERE document_id = " + std::to_string(document_id) + " AND version_number = " + std::to_string(version_number);

    // 存储查询结果
    DocumentVersion version;
    bool found = false;

    // 执行查询
    if (!query(sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto [version_ptr, found_ptr] = *static_cast<std::pair<DocumentVersion*, bool*>*>(data);

        // 填充版本对象
        for (int i = 0; i < argc; ++i) {
            std::string col_name = col_names[i];
            std::string col_value = argv[i] ? argv[i] : "";

            if (col_name == "id") {
                version_ptr->setId(std::stoi(col_value));
            } else if (col_name == "document_id") {
                version_ptr->setDocumentId(std::stoi(col_value));
            } else if (col_name == "version_number") {
                version_ptr->setVersionNumber(std::stoi(col_value));
            } else if (col_name == "content") {
                version_ptr->setContent(col_value);
            } else if (col_name == "created_at") {
                version_ptr->setCreatedAt(col_value);
            }
        }

        *found_ptr = true;
        return 0;
    }, new std::pair<DocumentVersion*, bool*>{&version, &found})) {
        return std::nullopt;
    }

    // 清理内存 - 不需要手动清理，因为我们使用的是栈上的对象

    if (found) {
        return version;
    } else {
        return std::nullopt;
    }
}

std::optional<DocumentVersion> Database::getLatestDocumentVersion(int document_id) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return std::nullopt;
    }

    // 准备SQL语句
    std::string sql = "SELECT * FROM document_versions WHERE document_id = " + std::to_string(document_id) + " ORDER BY version_number DESC LIMIT 1";

    // 存储查询结果
    DocumentVersion version;
    bool found = false;

    // 执行查询
    if (!query(sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto [version_ptr, found_ptr] = *static_cast<std::pair<DocumentVersion*, bool*>*>(data);

        // 填充版本对象
        for (int i = 0; i < argc; ++i) {
            std::string col_name = col_names[i];
            std::string col_value = argv[i] ? argv[i] : "";

            if (col_name == "id") {
                version_ptr->setId(std::stoi(col_value));
            } else if (col_name == "document_id") {
                version_ptr->setDocumentId(std::stoi(col_value));
            } else if (col_name == "version_number") {
                version_ptr->setVersionNumber(std::stoi(col_value));
            } else if (col_name == "content") {
                version_ptr->setContent(col_value);
            } else if (col_name == "created_at") {
                version_ptr->setCreatedAt(col_value);
            }
        }

        *found_ptr = true;
        return 0;
    }, new std::pair<DocumentVersion*, bool*>{&version, &found})) {
        return std::nullopt;
    }

    // 清理内存 - 不需要手动清理，因为我们使用的是栈上的对象

    if (found) {
        return version;
    } else {
        return std::nullopt;
    }
}

PaginationResult<DocumentVersion> Database::getDocumentVersions(int document_id,
                                                                     int page,
                                                                     int page_size,
                                                                     bool order_by_version) {
    PaginationResult<DocumentVersion> result;

    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return result;
    }

    // 获取总记录数
    std::string count_sql = "SELECT COUNT(*) FROM document_versions WHERE document_id = " + std::to_string(document_id);
    int total = 0;

    if (!query(count_sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto total_ptr = static_cast<int*>(data);
        *total_ptr = std::stoi(argv[0]);
        return 0;
    }, &total)) {
        return result;
    }

    // 计算分页参数
    int offset = (page - 1) * page_size;

    // 构建查询SQL
    std::string order_by_clause = order_by_version ? "ORDER BY version_number DESC" : "ORDER BY created_at DESC";
    std::string sql = "SELECT * FROM document_versions WHERE document_id = " + std::to_string(document_id) + " " + order_by_clause + " LIMIT " + std::to_string(page_size) + " OFFSET " + std::to_string(offset);

    // 存储查询结果
    std::vector<DocumentVersion> versions;

    // 执行查询
    if (!query(sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto versions_ptr = static_cast<std::vector<DocumentVersion>*>(data);
        DocumentVersion version;

        // 填充版本对象
        for (int i = 0; i < argc; ++i) {
            std::string col_name = col_names[i];
            std::string col_value = argv[i] ? argv[i] : "";

            if (col_name == "id") {
                version.setId(std::stoi(col_value));
            } else if (col_name == "document_id") {
                version.setDocumentId(std::stoi(col_value));
            } else if (col_name == "version_number") {
                version.setVersionNumber(std::stoi(col_value));
            } else if (col_name == "content") {
                version.setContent(col_value);
            } else if (col_name == "created_at") {
                version.setCreatedAt(col_value);
            }
        }

        versions_ptr->push_back(version);
        return 0;
    }, &versions)) {
        return result;
    }

    // 填充结果
    result.setItems(versions);
    result.setPage(page);
    result.setPageSize(page_size);
    result.setTotal(total);

    return result;
}

// 评论相关操作
std::optional<Comment> Database::createComment(const Comment& comment) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return std::nullopt;
    }

    // 检查文档是否存在
    auto document = getDocumentById(comment.getDocumentId());
    if (!document) {
        std::cerr << "Document not found: " << comment.getDocumentId() << std::endl;
        return std::nullopt;
    }

    // 如果指定了版本号，检查版本是否存在
    if (comment.getVersionNumber()) {
        auto version = getDocumentVersionByNumber(comment.getDocumentId(), *comment.getVersionNumber());
        if (!version) {
            std::cerr << "Document version not found: " << comment.getDocumentId() << " - " << *comment.getVersionNumber() << std::endl;
            return std::nullopt;
        }
    }

    // 检查用户是否存在
    auto user = getUserById(comment.getAuthorId());
    if (!user) {
        std::cerr << "User not found: " << comment.getAuthorId() << std::endl;
        return std::nullopt;
    }

    // 准备SQL语句
    std::string sql = "INSERT INTO comments (document_id, version_number, author_id, content) VALUES (";
    sql += std::to_string(comment.getDocumentId());
    sql += ", ";

    if (comment.getVersionNumber()) {
        sql += std::to_string(*comment.getVersionNumber());
    } else {
        sql += "NULL";
    }

    sql += ", " + std::to_string(comment.getAuthorId());
    sql += ", '" + comment.getContent() + "')";

    // 执行SQL语句
    if (!execute(sql)) {
        return std::nullopt;
    }

    // 获取新创建的评论ID
    int comment_id = getLastInsertId();

    // 查询新创建的评论信息
    return getCommentById(comment_id);
}

std::optional<Comment> Database::getCommentById(int id) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return std::nullopt;
    }

    // 准备SQL语句
    std::string sql = "SELECT * FROM comments WHERE id = " + std::to_string(id);

    // 存储查询结果
    Comment comment;
    bool found = false;

    // 执行查询
    if (!query(sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto [comment_ptr, found_ptr] = *static_cast<std::pair<Comment*, bool*>*>(data);

        // 填充评论对象
        for (int i = 0; i < argc; ++i) {
            std::string col_name = col_names[i];
            std::string col_value = argv[i] ? argv[i] : "";

            if (col_name == "id") {
                comment_ptr->setId(std::stoi(col_value));
            } else if (col_name == "document_id") {
                comment_ptr->setDocumentId(std::stoi(col_value));
            } else if (col_name == "version_number") {
                if (argv[i] != nullptr) {
                    comment_ptr->setVersionNumber(std::stoi(col_value));
                } else {
                    comment_ptr->setVersionNumber(std::nullopt);
                }
            } else if (col_name == "author_id") {
                comment_ptr->setAuthorId(std::stoi(col_value));
            } else if (col_name == "content") {
                comment_ptr->setContent(col_value);
            } else if (col_name == "created_at") {
                comment_ptr->setCreatedAt(col_value);
            }
        }

        *found_ptr = true;
        return 0;
    }, new std::pair<Comment*, bool*>{&comment, &found})) {
        return std::nullopt;
    }

    // 清理内存 - 不需要手动清理，因为我们使用的是栈上的对象

    if (found) {
        return comment;
    } else {
        return std::nullopt;
    }
}

PaginationResult<Comment> Database::getComments(int document_id,
                                                    std::optional<int> version_number,
                                                    int page,
                                                    int page_size) {
    PaginationResult<Comment> result;

    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return result;
    }

    // 构建查询条件
    std::string where_clause = "WHERE document_id = " + std::to_string(document_id);

    if (version_number) {
        where_clause += " AND version_number = " + std::to_string(*version_number);
    }

    // 获取总记录数
    std::string count_sql = "SELECT COUNT(*) FROM comments " + where_clause;
    int total = 0;

    if (!query(count_sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto total_ptr = static_cast<int*>(data);
        *total_ptr = std::stoi(argv[0]);
        return 0;
    }, &total)) {
        return result;
    }

    // 计算分页参数
    int offset = (page - 1) * page_size;

    // 构建查询SQL
    std::string sql = "SELECT * FROM comments " + where_clause + " ORDER BY created_at DESC LIMIT " + std::to_string(page_size) + " OFFSET " + std::to_string(offset);

    // 存储查询结果
    std::vector<Comment> comments;

    // 执行查询
    if (!query(sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto comments_ptr = static_cast<std::vector<Comment>*>(data);
        Comment comment;

        // 填充评论对象
        for (int i = 0; i < argc; ++i) {
            std::string col_name = col_names[i];
            std::string col_value = argv[i] ? argv[i] : "";

            if (col_name == "id") {
                comment.setId(std::stoi(col_value));
            } else if (col_name == "document_id") {
                comment.setDocumentId(std::stoi(col_value));
            } else if (col_name == "version_number") {
                if (argv[i] != nullptr) {
                    comment.setVersionNumber(std::stoi(col_value));
                } else {
                    comment.setVersionNumber(std::nullopt);
                }
            } else if (col_name == "author_id") {
                comment.setAuthorId(std::stoi(col_value));
            } else if (col_name == "content") {
                comment.setContent(col_value);
            } else if (col_name == "created_at") {
                comment.setCreatedAt(col_value);
            }
        }

        comments_ptr->push_back(comment);
        return 0;
    }, &comments)) {
        return result;
    }

    // 填充结果
    result.setItems(comments);
    result.setPage(page);
    result.setPageSize(page_size);
    result.setTotal(total);

    return result;
}

// 统计相关操作
Metrics Database::getMetrics() {
    Metrics metrics;

    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return metrics;
    }

    // 获取总用户数
    std::string sql = "SELECT COUNT(*) FROM users";
    int total_users = 0;

    if (query(sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto total_ptr = static_cast<int*>(data);
        *total_ptr = std::stoi(argv[0]);
        return 0;
    }, &total_users)) {
        metrics.setTotalUsers(total_users);
    }

    // 获取总文档数
    sql = "SELECT COUNT(*) FROM documents";
    int total_documents = 0;

    if (query(sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto total_ptr = static_cast<int*>(data);
        *total_ptr = std::stoi(argv[0]);
        return 0;
    }, &total_documents)) {
        metrics.setTotalDocuments(total_documents);
    }

    // 获取总版本数
    sql = "SELECT COUNT(*) FROM document_versions";
    int total_versions = 0;

    if (query(sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto total_ptr = static_cast<int*>(data);
        *total_ptr = std::stoi(argv[0]);
        return 0;
    }, &total_versions)) {
        metrics.setTotalVersions(total_versions);
    }

    // 获取总评论数
    sql = "SELECT COUNT(*) FROM comments";
    int total_comments = 0;

    if (query(sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto total_ptr = static_cast<int*>(data);
        *total_ptr = std::stoi(argv[0]);
        return 0;
    }, &total_comments)) {
        metrics.setTotalComments(total_comments);
    }

    // 获取按版本数排序的前10个文档
    sql = "SELECT document_id, COUNT(*) as version_count FROM document_versions GROUP BY document_id ORDER BY version_count DESC LIMIT 10";
    std::vector<std::pair<int, int>> top_documents;

    if (query(sql, [](void* data, int argc, char** argv, char** col_names) -> int {
        auto top_documents_ptr = static_cast<std::vector<std::pair<int, int>>*>(data);
        int document_id = std::stoi(argv[0]);
        int version_count = std::stoi(argv[1]);
        top_documents_ptr->emplace_back(document_id, version_count);
        return 0;
    }, &top_documents)) {
        metrics.setTopDocumentsByVersions(top_documents);
    }

    return metrics;
}