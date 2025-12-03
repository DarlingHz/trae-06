#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <sstream>
#include <regex>
#include <algorithm>

namespace domain {

/**
 * @class ReadReceipt
 * @brief 阅读记录数据模型
 * 
 * 表示用户阅读公告的记录，包含用户ID、公告ID、阅读时间等信息，
 * 用于跟踪用户是否已阅读公告以及阅读详情。
 */
class ReadReceipt {
public:
    /**
     * @brief 默认构造函数
     */
    ReadReceipt();

    /**
     * @brief 带参数的构造函数
     * @param user_id 用户ID
     * @param announcement_id 公告ID
     */
    ReadReceipt(long long user_id, long long announcement_id);

    /**
     * @brief 带ID的构造函数
     * @param id 阅读记录ID
     * @param user_id 用户ID
     * @param announcement_id 公告ID
     */
    ReadReceipt(long long id, long long user_id, long long announcement_id);

    /**
     * @brief 带时间信息的构造函数
     * @param user_id 用户ID
     * @param announcement_id 公告ID
     * @param read_at 阅读时间
     */
    ReadReceipt(long long user_id, long long announcement_id, const std::string& read_at);

    /**
     * @brief 带完整信息的构造函数
     * @param id 阅读记录ID
     * @param user_id 用户ID
     * @param announcement_id 公告ID
     * @param read_at 阅读时间
     */
    ReadReceipt(long long id, long long user_id, long long announcement_id, const std::string& read_at);

    /**
     * @brief 析构函数
     */
    virtual ~ReadReceipt();

    /**
     * @brief 获取阅读记录ID
     * @return 阅读记录ID
     */
    long long get_id() const;

    /**
     * @brief 设置阅读记录ID
     * @param id 阅读记录ID
     */
    void set_id(long long id);

    /**
     * @brief 获取用户ID
     * @return 用户ID
     */
    long long get_user_id() const;

    /**
     * @brief 设置用户ID
     * @param user_id 用户ID
     */
    void set_user_id(long long user_id);

    /**
     * @brief 获取公告ID
     * @return 公告ID
     */
    long long get_announcement_id() const;

    /**
     * @brief 设置公告ID
     * @param announcement_id 公告ID
     */
    void set_announcement_id(long long announcement_id);

    /**
     * @brief 获取阅读时间
     * @return 阅读时间（可选）
     */
    const std::optional<std::string>& get_read_at() const;

    /**
     * @brief 设置阅读时间
     * @param read_at 阅读时间
     */
    void set_read_at(const std::optional<std::string>& read_at);

    /**
     * @brief 获取创建时间
     * @return 创建时间（可选）
     */
    const std::optional<std::string>& get_created_at() const;

    /**
     * @brief 设置创建时间
     * @param created_at 创建时间
     */
    void set_created_at(const std::optional<std::string>& created_at);

    /**
     * @brief 获取更新时间
     * @return 更新时间（可选）
     */
    const std::optional<std::string>& get_updated_at() const;

    /**
     * @brief 设置更新时间
     * @param updated_at 更新时间
     */
    void set_updated_at(const std::optional<std::string>& updated_at);

    /**
     * @brief 获取阅读持续时间（秒）
     * @return 阅读持续时间（可选）
     */
    const std::optional<int>& get_read_duration() const;

    /**
     * @brief 设置阅读持续时间（秒）
     * @param read_duration 阅读持续时间（秒）
     */
    void set_read_duration(const std::optional<int>& read_duration);

    /**
     * @brief 获取用户位置信息
     * @return 用户位置信息（可选）
     */
    const std::optional<std::string>& get_location() const;

    /**
     * @brief 设置用户位置信息
     * @param location 用户位置信息
     */
    void set_location(const std::optional<std::string>& location);

    /**
     * @brief 获取用户设备信息
     * @return 用户设备信息（可选）
     */
    const std::optional<std::string>& get_device_info() const;

    /**
     * @brief 设置用户设备信息
     * @param device_info 用户设备信息
     */
    void set_device_info(const std::optional<std::string>& device_info);

    /**
     * @brief 获取IP地址
     * @return IP地址（可选）
     */
    const std::optional<std::string>& get_ip_address() const;

    /**
     * @brief 设置IP地址
     * @param ip_address IP地址
     */
    void set_ip_address(const std::optional<std::string>& ip_address);

    /**
     * @brief 获取用户代理信息
     * @return 用户代理信息（可选）
     */
    const std::optional<std::string>& get_user_agent() const;

    /**
     * @brief 设置用户代理信息
     * @param user_agent 用户代理信息
     */
    void set_user_agent(const std::optional<std::string>& user_agent);

    /**
     * @brief 获取阅读状态
     * @return 是否已阅读
     */
    bool is_read() const;

    /**
     * @brief 设置阅读状态
     * @param read 是否已阅读
     */
    void set_read(bool read);

    /**
     * @brief 获取最后阅读时间
     * @return 最后阅读时间（可选）
     */
    const std::optional<std::string>& get_last_read_at() const;

    /**
     * @brief 设置最后阅读时间
     * @param last_read_at 最后阅读时间
     */
    void set_last_read_at(const std::optional<std::string>& last_read_at);

    /**
     * @brief 获取阅读进度（百分比）
     * @return 阅读进度（可选，0-100）
     */
    const std::optional<int>& get_read_progress() const;

    /**
     * @brief 设置阅读进度（百分比）
     * @param read_progress 阅读进度（0-100）
     */
    void set_read_progress(const std::optional<int>& read_progress);

    /**
     * @brief 获取用户备注
     * @return 用户备注（可选）
     */
    const std::optional<std::string>& get_note() const;

    /**
     * @brief 设置用户备注
     * @param note 用户备注
     */
    void set_note(const std::optional<std::string>& note);

    /**
     * @brief 检查阅读进度是否完成
     * @return 如果阅读进度 >= 95% 返回true，否则返回false
     */
    bool is_progress_complete() const;

    /**
     * @brief 更新阅读进度
     * @param progress 新的阅读进度（0-100）
     */
    void update_read_progress(int progress);

    /**
     * @brief 更新阅读状态
     * @param read_at 阅读时间
     * @param progress 阅读进度（可选）
     * @param duration 阅读持续时间（可选）
     */
    void mark_as_read(const std::string& read_at, std::optional<int> progress = std::nullopt, std::optional<int> duration = std::nullopt);

    /**
     * @brief 获取记录是否被软删除
     * @return 是否被软删除
     */
    bool is_deleted() const;

    /**
     * @brief 设置软删除状态
     * @param deleted 是否被软删除
     */
    void set_deleted(bool deleted);

    /**
     * @brief 获取软删除时间
     * @return 软删除时间（可选）
     */
    const std::optional<std::string>& get_deleted_at() const;

    /**
     * @brief 设置软删除时间
     * @param deleted_at 软删除时间
     */
    void set_deleted_at(const std::optional<std::string>& deleted_at);

    /**
     * @brief 获取版本号
     * @return 版本号
     */
    int get_version() const;

    /**
     * @brief 设置版本号
     * @param version 版本号
     */
    void set_version(int version);

    /**
     * @brief 递增版本号
     */
    void increment_version();

    /**
     * @brief 检查IP地址是否有效
     * @param ip IP地址
     * @return 如果有效返回true，否则返回false
     */
    static bool is_valid_ip(const std::string& ip);

    /**
     * @brief 检查阅读进度是否有效
     * @param progress 阅读进度
     * @return 如果有效返回true，否则返回false
     */
    static bool is_valid_progress(int progress);

    /**
     * @brief 获取ISO时间格式字符串
     * @return 当前时间的ISO格式字符串
     */
    static std::string get_current_time_iso();

    /**
     * @brief 解析ISO时间格式字符串
     * @param time_str ISO时间格式字符串
     * @return 时间点（可选）
     */
    static std::optional<std::chrono::system_clock::time_point> parse_iso_time(const std::string& time_str);

    /**
     * @brief 比较两个阅读记录是否相等
     * @param lhs 左操作数
     * @param rhs 右操作数
     * @return 如果相等返回true，否则返回false
     */
    friend bool operator==(const ReadReceipt& lhs, const ReadReceipt& rhs);

    /**
     * @brief 比较两个阅读记录是否不相等
     * @param lhs 左操作数
     * @param rhs 右操作数
     * @return 如果不相等返回true，否则返回false
     */
    friend bool operator!=(const ReadReceipt& lhs, const ReadReceipt& rhs);

private:
    long long id_;                             // 阅读记录ID
    long long user_id_;                        // 用户ID
    long long announcement_id_;                // 公告ID
    std::optional<std::string> read_at_;       // 阅读时间
    std::optional<std::string> created_at_;    // 创建时间
    std::optional<std::string> updated_at_;    // 更新时间
    std::optional<int> read_duration_;         // 阅读持续时间（秒）
    std::optional<std::string> location_;      // 用户位置信息
    std::optional<std::string> device_info_;   // 用户设备信息
    std::optional<std::string> ip_address_;    // IP地址
    std::optional<std::string> user_agent_;    // 用户代理信息
    bool is_read_;                             // 是否已阅读
    std::optional<std::string> last_read_at_;  // 最后阅读时间
    std::optional<int> read_progress_;         // 阅读进度（百分比，0-100）
    std::optional<std::string> note_;          // 用户备注
    bool deleted_;                             // 是否被软删除
    std::optional<std::string> deleted_at_;    // 软删除时间
    int version_;                              // 版本号

    /**
     * @brief 验证IP地址格式
     * @param ip IP地址
     * @return 如果有效返回true，否则返回false
     */
    bool validate_ip_(const std::string& ip) const;

    /**
     * @brief 验证阅读进度
     * @param progress 阅读进度
     * @return 如果有效返回true，否则返回false
     */
    bool validate_progress_(int progress) const;
};

/**
 * @struct ReadReceiptFilter
 * @brief 阅读记录查询过滤器
 * 
 * 用于构建阅读记录的查询条件，可以根据用户ID、公告ID、阅读状态、时间范围等进行过滤。
 */
struct ReadReceiptFilter {
    std::optional<long long> user_id;                          // 用户ID
    std::optional<long long> announcement_id;                  // 公告ID
    std::optional<bool> is_read;                               // 是否已阅读
    std::optional<std::string> created_before;                  // 创建时间之前
    std::optional<std::string> created_after;                   // 创建时间之后
    std::optional<std::string> read_before;                     // 阅读时间之前
    std::optional<std::string> read_after;                      // 阅读时间之后
    std::optional<int> min_read_duration;                       // 最小阅读持续时间（秒）
    std::optional<int> max_read_duration;                       // 最大阅读持续时间（秒）
    std::optional<int> min_read_progress;                       // 最小阅读进度（百分比）
    std::optional<int> max_read_progress;                       // 最大阅读进度（百分比）
    std::optional<std::vector<long long>> user_ids;             // 用户ID列表
    std::optional<std::vector<long long>> announcement_ids;     // 公告ID列表
    std::optional<bool> with_deleted;                           // 是否包含已删除记录

    /**
     * @brief 重置过滤器
     */
    void reset();

    /**
     * @brief 检查是否有任何过滤条件
     * @return 如果有过滤条件返回true，否则返回false
     */
    bool has_conditions() const;

    /**
     * @brief 获取过滤条件的字符串表示
     * @return 过滤条件的字符串表示
     */
    std::string to_string() const;
};

/**
 * @struct ReadReceiptStatistics
 * @brief 阅读记录统计信息
 * 
 * 用于存储阅读记录的统计数据，如总阅读数、未读数、平均阅读进度等。
 */
struct ReadReceiptStatistics {
    long long total_records = 0;                 // 总记录数
    long long read_records = 0;                  // 已读记录数
    long long unread_records = 0;                // 未读记录数
    double average_read_progress = 0.0;          // 平均阅读进度（百分比）
    double average_read_duration = 0.0;          // 平均阅读持续时间（秒）
    std::vector<std::pair<long long, long long>> user_read_counts; // 用户阅读数量统计
    std::vector<std::pair<long long, long long>> announcement_read_counts; // 公告阅读数量统计
    std::vector<std::pair<std::string, long long>> daily_read_counts; // 每日阅读数量统计
    std::vector<std::pair<std::string, long long>> hourly_read_counts; // 每小时阅读数量统计

    /**
     * @brief 获取阅读率
     * @return 阅读率（百分比）
     */
    double get_read_rate() const;

    /**
     * @brief 计算阅读完成率
     * @return 阅读完成率（百分比）
     */
    double get_completion_rate() const;

    /**
     * @brief 重置统计信息
     */
    void reset();

    /**
     * @brief 获取统计信息的字符串表示
     * @return 统计信息的字符串表示
     */
    std::string to_string() const;
};

} // namespace domain
