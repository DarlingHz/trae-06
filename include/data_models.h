#ifndef DATA_MODELS_H
#define DATA_MODELS_H

#include <string>
#include <vector>
#include <optional>
#include <ctime>

/**
 * @brief 用户数据模型
 */
class User {
public:
    User() = default;
    User(int id, const std::string& name, const std::string& email, const std::string& created_at)
        : id_(id), name_(name), email_(email), created_at_(created_at) {}

    // Getter方法
    int getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getEmail() const { return email_; }
    const std::string& getCreatedAt() const { return created_at_; }

    // Setter方法
    void setId(int id) { id_ = id; }
    void setName(const std::string& name) { name_ = name; }
    void setEmail(const std::string& email) { email_ = email; }
    void setCreatedAt(const std::string& created_at) { created_at_ = created_at; }

private:
    int id_ = 0;                      ///< 用户ID
    std::string name_;                ///< 用户姓名
    std::string email_;               ///< 用户邮箱
    std::string created_at_;          ///< 创建时间
};

/**
 * @brief 文档数据模型
 */
class Document {
public:
    Document() = default;
    Document(int id, int owner_id, const std::string& title, const std::vector<std::string>& tags,
             const std::string& created_at, const std::string& updated_at)
        : id_(id), owner_id_(owner_id), title_(title), tags_(tags),
          created_at_(created_at), updated_at_(updated_at) {}

    // Getter方法
    int getId() const { return id_; }
    int getOwnerId() const { return owner_id_; }
    const std::string& getTitle() const { return title_; }
    const std::vector<std::string>& getTags() const { return tags_; }
    const std::string& getCreatedAt() const { return created_at_; }
    const std::string& getUpdatedAt() const { return updated_at_; }

    // Setter方法
    void setId(int id) { id_ = id; }
    void setOwnerId(int owner_id) { owner_id_ = owner_id; }
    void setTitle(const std::string& title) { title_ = title; }
    void setTags(const std::vector<std::string>& tags) { tags_ = tags; }
    void setCreatedAt(const std::string& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::string& updated_at) { updated_at_ = updated_at; }

private:
    int id_ = 0;                      ///< 文档ID
    int owner_id_ = 0;                ///< 所有者ID
    std::string title_;                ///< 文档标题
    std::vector<std::string> tags_;   ///< 文档标签
    std::string created_at_;           ///< 创建时间
    std::string updated_at_;           ///< 更新时间
};

/**
 * @brief 文档版本数据模型
 */
class DocumentVersion {
public:
    DocumentVersion() = default;
    DocumentVersion(int id, int document_id, int version_number, const std::string& content,
                    const std::string& created_at)
        : id_(id), document_id_(document_id), version_number_(version_number), content_(content),
          created_at_(created_at) {}

    // Getter方法
    int getId() const { return id_; }
    int getDocumentId() const { return document_id_; }
    int getVersionNumber() const { return version_number_; }
    const std::string& getContent() const { return content_; }
    const std::string& getCreatedAt() const { return created_at_; }

    // Setter方法
    void setId(int id) { id_ = id; }
    void setDocumentId(int document_id) { document_id_ = document_id; }
    void setVersionNumber(int version_number) { version_number_ = version_number; }
    void setContent(const std::string& content) { content_ = content; }
    void setCreatedAt(const std::string& created_at) { created_at_ = created_at; }

private:
    int id_ = 0;                      ///< 版本ID
    int document_id_ = 0;             ///< 文档ID
    int version_number_ = 0;          ///< 版本号
    std::string content_;              ///< 文档内容
    std::string created_at_;           ///< 创建时间
};

/**
 * @brief 评论数据模型
 */
class Comment {
public:
    Comment() = default;
    Comment(int id, int document_id, std::optional<int> version_number, int author_id,
            const std::string& content, const std::string& created_at)
        : id_(id), document_id_(document_id), version_number_(version_number), author_id_(author_id),
          content_(content), created_at_(created_at) {}

    // Getter方法
    int getId() const { return id_; }
    int getDocumentId() const { return document_id_; }
    std::optional<int> getVersionNumber() const { return version_number_; }
    int getAuthorId() const { return author_id_; }
    const std::string& getContent() const { return content_; }
    const std::string& getCreatedAt() const { return created_at_; }

    // Setter方法
    void setId(int id) { id_ = id; }
    void setDocumentId(int document_id) { document_id_ = document_id; }
    void setVersionNumber(std::optional<int> version_number) { version_number_ = version_number; }
    void setAuthorId(int author_id) { author_id_ = author_id; }
    void setContent(const std::string& content) { content_ = content; }
    void setCreatedAt(const std::string& created_at) { created_at_ = created_at; }

private:
    int id_ = 0;                      ///< 评论ID
    int document_id_ = 0;             ///< 文档ID
    std::optional<int> version_number_; ///< 版本号（可选）
    int author_id_ = 0;               ///< 作者ID
    std::string content_;              ///< 评论内容
    std::string created_at_;           ///< 创建时间
};

/**
 * @brief 文档详情数据模型（包含最新版本信息）
 */
class DocumentDetail {
public:
    DocumentDetail() = default;
    DocumentDetail(const Document& document, const DocumentVersion& latest_version)
        : document_(document), latest_version_(latest_version) {}

    // Getter方法
    const Document& getDocument() const { return document_; }
    const DocumentVersion& getLatestVersion() const { return latest_version_; }

    // Setter方法
    void setDocument(const Document& document) { document_ = document; }
    void setLatestVersion(const DocumentVersion& latest_version) { latest_version_ = latest_version; }

private:
    Document document_;                ///< 文档基础信息
    DocumentVersion latest_version_;   ///< 最新版本信息
};

/**
 * @brief 分页结果数据模型
 * @tparam T 结果类型
 */
template<typename T>
class PaginationResult {
public:
    PaginationResult() = default;
    PaginationResult(const std::vector<T>& items, int page, int page_size, int total)
        : items_(items), page_(page), page_size_(page_size), total_(total) {}

    // Getter方法
    const std::vector<T>& getItems() const { return items_; }
    int getPage() const { return page_; }
    int getPageSize() const { return page_size_; }
    int getTotal() const { return total_; }

    // Setter方法
    void setItems(const std::vector<T>& items) { items_ = items; }
    void setPage(int page) { page_ = page; }
    void setPageSize(int page_size) { page_size_ = page_size; }
    void setTotal(int total) { total_ = total; }

private:
    std::vector<T> items_;            ///< 结果列表
    int page_ = 1;                     ///< 当前页码
    int page_size_ = 10;               ///< 每页大小
    int total_ = 0;                     ///< 总记录数
};

/**
 * @brief 统计信息数据模型
 */
class Metrics {
public:
    Metrics() = default;
    Metrics(int total_users, int total_documents, int total_versions, int total_comments,
            const std::vector<std::pair<int, int>>& top_documents_by_versions)
        : total_users_(total_users), total_documents_(total_documents), total_versions_(total_versions),
          total_comments_(total_comments), top_documents_by_versions_(top_documents_by_versions) {}

    // Getter方法
    int getTotalUsers() const { return total_users_; }
    int getTotalDocuments() const { return total_documents_; }
    int getTotalVersions() const { return total_versions_; }
    int getTotalComments() const { return total_comments_; }
    const std::vector<std::pair<int, int>>& getTopDocumentsByVersions() const { return top_documents_by_versions_; }

    // Setter方法
    void setTotalUsers(int total_users) { total_users_ = total_users; }
    void setTotalDocuments(int total_documents) { total_documents_ = total_documents; }
    void setTotalVersions(int total_versions) { total_versions_ = total_versions; }
    void setTotalComments(int total_comments) { total_comments_ = total_comments; }
    void setTopDocumentsByVersions(const std::vector<std::pair<int, int>>& top_documents) { top_documents_by_versions_ = top_documents; }

private:
    int total_users_ = 0;              ///< 总用户数
    int total_documents_ = 0;          ///< 总文档数
    int total_versions_ = 0;           ///< 总版本数
    int total_comments_ = 0;           ///< 总评论数
    std::vector<std::pair<int, int>> top_documents_by_versions_; ///< 按版本数排序的前N个文档
};

#endif // DATA_MODELS_H