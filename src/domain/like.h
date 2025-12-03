#ifndef LIKE_H
#define LIKE_H

#include <cstdint>
#include <string>
#include <chrono>
#include <optional>

/**
 * @brief 点赞状态
 */
enum class LikeStatus {
    ACTIVE = 1,    // 点赞有效
    CANCELLED = 2  // 点赞已取消
};

/**
 * @brief 点赞实体类
 * 表示用户对公告的点赞记录
 */
class Like {
private:
    int64_t id_;
    int64_t user_id_;
    int64_t announcement_id_;
    LikeStatus status_;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point updated_at_;

public:
    Like();
    Like(int64_t user_id, int64_t announcement_id);
    
    // Getters
    int64_t get_id() const;
    int64_t get_user_id() const;
    int64_t get_announcement_id() const;
    LikeStatus get_status() const;
    std::chrono::system_clock::time_point get_created_at() const;
    std::chrono::system_clock::time_point get_updated_at() const;
    std::string get_created_at_str() const;
    std::string get_updated_at_str() const;
    
    // Setters
    void set_id(int64_t id);
    void set_user_id(int64_t user_id);
    void set_announcement_id(int64_t announcement_id);
    void set_status(LikeStatus status);
    void set_created_at(const std::chrono::system_clock::time_point& created_at);
    void set_updated_at(const std::chrono::system_clock::time_point& updated_at);
    
    // 状态管理方法
    void activate();
    void cancel();
    bool is_active() const;
    bool is_cancelled() const;
    
    // 数据验证
    bool is_valid() const;
    std::string validate() const;
    
    // 辅助功能
    std::string to_string() const;
    
    // 枚举转换
    static LikeStatus from_string(const std::string& str);
    static std::string to_string(LikeStatus status);
};

/**
 * @brief 点赞统计信息
 */
struct LikeStatistics {
    int64_t announcement_id_;
    int64_t like_count_;
    int64_t unique_users_;
    bool user_liked_;  // 当前用户是否点赞
    
    LikeStatistics();
    LikeStatistics(int64_t announcement_id, int64_t like_count, int64_t unique_users, bool user_liked = false);
    
    std::string to_string() const;
};

/**
 * @brief 点赞过滤条件
 */
struct LikeFilter {
    std::optional<int64_t> user_id_;
    std::optional<int64_t> announcement_id_;
    std::optional<LikeStatus> status_;
    
    LikeFilter();
    std::string to_query_condition() const;
};

#endif // LIKE_H
