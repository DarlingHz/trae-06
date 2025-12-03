#ifndef LIKE_REPOSITORY_H
#define LIKE_REPOSITORY_H

#include <vector>
#include <optional>
#include <string>
#include <memory>
#include "../domain/like.h"
#include "sqlite3/sqlite3.h"

/**
 * @brief 点赞数据访问层
 * 处理点赞数据的持久化操作
 */
class LikeRepository {
private:
    sqlite3* db_;
    std::string db_path_;
    
    /**
     * @brief 打开数据库连接
     */
    void open();
    
    /**
     * @brief 关闭数据库连接
     */
    void close();
    
    /**
     * @brief 从SQLite结果行转换为Like对象
     */
    Like from_row(sqlite3_stmt* stmt) const;

public:
    /**
     * @brief 构造函数
     * @param db_path 数据库文件路径
     */
    explicit LikeRepository(const std::string& db_path);
    
    /**
     * @brief 析构函数
     */
    ~LikeRepository();
    
    /**
     * @brief 创建点赞表
     * @param db_path 数据库文件路径
     */
    static void create_table(const std::string& db_path);
    
    /**
     * @brief 创建点赞记录
     * @param like 点赞对象
     * @return 创建后的点赞对象（包含自动生成的ID）
     */
    Like create(const Like& like);
    
    /**
     * @brief 更新点赞记录
     * @param like 点赞对象
     * @return 更新后的点赞对象
     */
    Like update(const Like& like);
    
    /**
     * @brief 删除点赞记录
     * @param id 点赞ID
     */
    void delete_by_id(int64_t id);
    
    /**
     * @brief 根据ID获取点赞记录
     * @param id 点赞ID
     * @return 点赞对象（如果存在）
     */
    std::optional<Like> get_by_id(int64_t id);
    
    /**
     * @brief 根据用户ID和公告ID获取点赞记录
     * @param user_id 用户ID
     * @param announcement_id 公告ID
     * @return 点赞对象（如果存在）
     */
    std::optional<Like> get_by_user_and_announcement(int64_t user_id, int64_t announcement_id);
    
    /**
     * @brief 根据过滤条件查询点赞记录
     * @param filter 过滤条件
     * @param offset 偏移量
     * @param limit 限制数量
     * @return 点赞列表
     */
    std::vector<Like> find_by_filter(const LikeFilter& filter, int64_t offset = 0, int64_t limit = 100);
    
    /**
     * @brief 统计公告的点赞数量
     * @param announcement_id 公告ID
     * @return 点赞数量
     */
    int64_t count_by_announcement(int64_t announcement_id);
    
    /**
     * @brief 统计用户对公告的点赞数量
     * @param user_id 用户ID
     * @return 点赞数量
     */
    int64_t count_by_user(int64_t user_id);
    
    /**
     * @brief 检查用户是否已点赞公告
     * @param user_id 用户ID
     * @param announcement_id 公告ID
     * @return 是否已点赞
     */
    bool exists_by_user_and_announcement(int64_t user_id, int64_t announcement_id);
    
    /**
     * @brief 批量获取公告的点赞统计
     * @param announcement_ids 公告ID列表
     * @return 点赞统计列表
     */
    std::vector<std::pair<int64_t, int64_t>> get_counts_for_announcements(const std::vector<int64_t>& announcement_ids);
    
    /**
     * @brief 删除公告的所有点赞记录
     * @param announcement_id 公告ID
     */
    void delete_by_announcement(int64_t announcement_id);
    
    /**
     * @brief 删除用户的所有点赞记录
     * @param user_id 用户ID
     */
    void delete_by_user(int64_t user_id);
};

#endif // LIKE_REPOSITORY_H
