#ifndef SERVICE_H
#define SERVICE_H

#include <memory>
#include <optional>
#include "database.h"
#include "lru_cache.h"
#include "data_models.h"

/**
 * @brief 业务逻辑服务类，封装所有业务逻辑
 */
class Service {
public:
    /**
     * @brief 构造函数
     * @param database 数据库对象指针
     * @param cache 文档版本缓存对象指针
     */
    Service(std::shared_ptr<Database> database, std::shared_ptr<LRUCache<std::pair<int, int>, DocumentVersion>> cache);

    // 用户管理相关方法
    /**
     * @brief 创建用户
     * @param name 用户名
     * @param email 用户邮箱
     * @return 创建成功的用户对象，如果失败则返回std::nullopt
     */
    std::optional<User> createUser(const std::string& name, const std::string& email);

    /**
     * @brief 根据ID获取用户
     * @param id 用户ID
     * @return 用户对象，如果不存在则返回std::nullopt
     */
    std::optional<User> getUserById(int id);

    // 文档管理相关方法
    /**
     * @brief 创建文档
     * @param owner_id 所有者ID
     * @param title 文档标题
     * @param tags 文档标签
     * @return 创建成功的文档对象，如果失败则返回std::nullopt
     */
    std::optional<Document> createDocument(int owner_id, const std::string& title, const std::vector<std::string>& tags);

    /**
     * @brief 根据ID获取文档详情（含最新版本信息）
     * @param id 文档ID
     * @return 文档详情对象，如果失败则返回std::nullopt
     */
    std::optional<DocumentDetail> getDocumentDetailById(int id);

    /**
     * @brief 获取文档列表（带过滤与分页）
     * @param owner_id 按拥有者过滤（可选）
     * @param tag 按标签过滤（可选）
     * @param keyword 按标题关键字搜索（可选）
     * @param page 页码（默认为1）
     * @param page_size 每页大小（默认为10）
     * @return 分页结果
     */
    PaginationResult<Document> getDocuments(std::optional<int> owner_id = std::nullopt,
                                               std::optional<std::string> tag = std::nullopt,
                                               std::optional<std::string> keyword = std::nullopt,
                                               int page = 1,
                                               int page_size = 10);

    // 文档版本管理相关方法
    /**
     * @brief 为文档新增一个版本
     * @param document_id 文档ID
     * @param content 文档内容
     * @return 创建成功的文档版本对象，如果失败则返回std::nullopt
     */
    std::optional<DocumentVersion> createDocumentVersion(int document_id, const std::string& content);

    /**
     * @brief 获取文档的版本列表（带分页）
     * @param document_id 文档ID
     * @param page 页码（默认为1）
     * @param page_size 每页大小（默认为10）
     * @param order_by_version 按版本号排序（默认为true）
     * @return 分页结果
     */
    PaginationResult<DocumentVersion> getDocumentVersions(int document_id,
                                                              int page = 1,
                                                              int page_size = 10,
                                                              bool order_by_version = true);

    /**
     * @brief 获取文档的指定版本详情（优先从缓存读取）
     * @param document_id 文档ID
     * @param version_number 版本号
     * @return 文档版本对象，如果失败则返回std::nullopt
     */
    std::optional<DocumentVersion> getDocumentVersionByNumber(int document_id, int version_number);

    // 评论管理相关方法
    /**
     * @brief 为文档或某个版本添加评论
     * @param document_id 文档ID
     * @param author_id 作者ID
     * @param content 评论内容
     * @param version_number 版本号（可选，不提供时表示对整个文档评论）
     * @return 创建成功的评论对象，如果失败则返回std::nullopt
     */
    std::optional<Comment> createComment(int document_id, int author_id, const std::string& content, std::optional<int> version_number = std::nullopt);

    /**
     * @brief 获取文档的评论列表（带过滤与分页）
     * @param document_id 文档ID
     * @param version_number 按版本号过滤（可选）
     * @param page 页码（默认为1）
     * @param page_size 每页大小（默认为10）
     * @return 分页结果
     */
    PaginationResult<Comment> getComments(int document_id,
                                             std::optional<int> version_number = std::nullopt,
                                             int page = 1,
                                             int page_size = 10);

    // 统计相关方法
    /**
     * @brief 获取统计信息
     * @return 统计信息对象
     */
    Metrics getMetrics();

private:
    std::shared_ptr<Database> database_; ///< 数据库对象指针
    std::shared_ptr<LRUCache<std::pair<int, int>, DocumentVersion>> cache_; ///< 文档版本缓存对象指针
};

#endif // SERVICE_H