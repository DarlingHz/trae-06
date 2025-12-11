#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include "data_models.h"

/**
 * @brief 数据库访问类，封装SQLite3的操作
 */
class Database {
public:
    /**
     * @brief 构造函数
     * @param db_path 数据库文件路径
     */
    explicit Database(const std::string& db_path);

    /**
     * @brief 析构函数
     */
    ~Database();

    /**
     * @brief 打开数据库连接
     * @return 是否打开成功
     */
    bool open();

    /**
     * @brief 关闭数据库连接
     */
    void close();

    /**
     * @brief 初始化数据库（创建表）
     * @param init_sql_file 初始化SQL文件路径
     * @return 是否初始化成功
     */
    bool init(const std::string& init_sql_file);

    // 用户相关操作
    /**
     * @brief 创建用户
     * @param user 用户对象（id字段将被忽略）
     * @return 创建成功的用户对象（包含id字段）
     */
    std::optional<User> createUser(const User& user);

    /**
     * @brief 根据ID获取用户
     * @param id 用户ID
     * @return 用户对象，如果不存在则返回std::nullopt
     */
    std::optional<User> getUserById(int id);

    /**
     * @brief 根据邮箱获取用户
     * @param email 用户邮箱
     * @return 用户对象，如果不存在则返回std::nullopt
     */
    std::optional<User> getUserByEmail(const std::string& email);

    // 文档相关操作
    /**
     * @brief 创建文档
     * @param document 文档对象（id字段将被忽略）
     * @return 创建成功的文档对象（包含id字段）
     */
    std::optional<Document> createDocument(const Document& document);

    /**
     * @brief 根据ID获取文档
     * @param id 文档ID
     * @return 文档对象，如果不存在则返回std::nullopt
     */
    std::optional<Document> getDocumentById(int id);

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

    /**
     * @brief 更新文档
     * @param document 文档对象（id字段必须提供）
     * @return 是否更新成功
     */
    bool updateDocument(const Document& document);

    // 文档版本相关操作
    /**
     * @brief 创建文档版本
     * @param version 文档版本对象（id和version_number字段将被忽略）
     * @return 创建成功的文档版本对象（包含id和version_number字段）
     */
    std::optional<DocumentVersion> createDocumentVersion(const DocumentVersion& version);

    /**
     * @brief 根据ID获取文档版本
     * @param id 文档版本ID
     * @return 文档版本对象，如果不存在则返回std::nullopt
     */
    std::optional<DocumentVersion> getDocumentVersionById(int id);

    /**
     * @brief 根据文档ID和版本号获取文档版本
     * @param document_id 文档ID
     * @param version_number 版本号
     * @return 文档版本对象，如果不存在则返回std::nullopt
     */
    std::optional<DocumentVersion> getDocumentVersionByNumber(int document_id, int version_number);

    /**
     * @brief 获取文档的最新版本
     * @param document_id 文档ID
     * @return 文档版本对象，如果不存在则返回std::nullopt
     */
    std::optional<DocumentVersion> getLatestDocumentVersion(int document_id);

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

    // 评论相关操作
    /**
     * @brief 创建评论
     * @param comment 评论对象（id字段将被忽略）
     * @return 创建成功的评论对象（包含id字段）
     */
    std::optional<Comment> createComment(const Comment& comment);

    /**
     * @brief 根据ID获取评论
     * @param id 评论ID
     * @return 评论对象，如果不存在则返回std::nullopt
     */
    std::optional<Comment> getCommentById(int id);

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

    // 统计相关操作
    /**
     * @brief 获取统计信息
     * @return 统计信息对象
     */
    Metrics getMetrics();

private:
    /**
     * @brief 执行SQL语句（不返回结果）
     * @param sql SQL语句
     * @return 是否执行成功
     */
    bool execute(const std::string& sql);

    /**
     * @brief 执行SQL查询（返回结果）
     * @param sql SQL查询语句
     * @param callback 结果回调函数
     * @param data 传递给回调函数的数据
     * @return 是否执行成功
     */
    bool query(const std::string& sql, std::function<int(void*, int, char**, char**)> callback, void* data);

    /**
     * @brief 从数据库中获取最后插入的ID
     * @return 最后插入的ID
     */
    int getLastInsertId();

    /**
     * @brief 将标签向量转换为逗号分隔的字符串
     * @param tags 标签向量
     * @return 逗号分隔的标签字符串
     */
    std::string tagsToString(const std::vector<std::string>& tags);

    /**
     * @brief 将逗号分隔的标签字符串转换为标签向量
     * @param tags_str 逗号分隔的标签字符串
     * @return 标签向量
     */
    std::vector<std::string> stringToTags(const std::string& tags_str);

private:
    std::string db_path_;              ///< 数据库文件路径
    sqlite3* db_ = nullptr;             ///< SQLite3数据库连接句柄
    bool is_open_ = false;              ///< 数据库连接是否打开
};

#endif // DATABASE_H