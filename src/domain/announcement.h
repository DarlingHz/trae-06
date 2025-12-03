#pragma once

#include <string>
#include <vector>
#include <optional>
#include <set>
#include <unordered_set>
#include <utility>
#include <algorithm>
#include <sstream>
#include <regex>

namespace domain {

/**
 * @enum AnnouncementStatus
 * @brief 公告状态枚举
 */
enum class AnnouncementStatus {
    DRAFT,          // 草稿
    PUBLISHED,      // 已发布
    ARCHIVED,       // 已归档
    DELETED,        // 已删除
    PENDING,        // 待审核
    REJECTED        // 已拒绝
};

/**
 * @enum AnnouncementPriority
 * @brief 公告优先级枚举
 */
enum class AnnouncementPriority {
    LOW,    // 低优先级
    MEDIUM, // 中优先级
    HIGH,   // 高优先级
    URGENT  // 紧急
};

/**
 * @enum AnnouncementType
 * @brief 公告类型枚举
 */
enum class AnnouncementType {
    NOTICE,         // 通知
    ANNOUNCEMENT,   // 公告
    UPDATE,         // 更新
    ALERT,          // 警告
    NEWS,           // 新闻
    EVENT,          // 活动
    MEMO,           // 备忘录
    POLICY          // 政策
};

/**
 * @class Announcement
 * @brief 公告数据模型
 * 
 * 表示系统中的公告信息，包含标题、内容、状态、优先级、类型等属性，
 * 以及创建时间、更新时间、发布时间、过期时间等时间信息。
 */
class Announcement {
public:
    // 声明友元函数用于权限哈希
    struct PermissionHash;

    /**
     * @brief 权限集合类型定义
     */
    using Permission = int;
    using PermissionSet = std::unordered_set<Permission, PermissionHash>;

    /**
     * @brief 默认构造函数
     */
    Announcement();

    /**
     * @brief 带参数的构造函数
     * @param title 公告标题
     * @param content 公告内容
     * @param author_id 作者ID
     * @param status 公告状态
     * @param priority 公告优先级
     * @param type 公告类型
     */
    Announcement(
        const std::string& title,
        const std::string& content,
        long long author_id,
        AnnouncementStatus status = AnnouncementStatus::DRAFT,
        AnnouncementPriority priority = AnnouncementPriority::MEDIUM,
        AnnouncementType type = AnnouncementType::NOTICE
    );

    /**
     * @brief 带ID的构造函数
     * @param id 公告ID
     * @param title 公告标题
     * @param content 公告内容
     * @param author_id 作者ID
     * @param status 公告状态
     * @param priority 公告优先级
     * @param type 公告类型
     */
    Announcement(
        long long id,
        const std::string& title,
        const std::string& content,
        long long author_id,
        AnnouncementStatus status = AnnouncementStatus::DRAFT,
        AnnouncementPriority priority = AnnouncementPriority::MEDIUM,
        AnnouncementType type = AnnouncementType::NOTICE
    );

    /**
     * @brief 析构函数
     */
    virtual ~Announcement();

    /**
     * @brief 获取公告ID
     * @return 公告ID
     */
    long long get_id() const;

    /**
     * @brief 设置公告ID
     * @param id 公告ID
     */
    void set_id(long long id);

    /**
     * @brief 获取公告标题
     * @return 公告标题
     */
    const std::string& get_title() const;

    /**
     * @brief 设置公告标题
     * @param title 公告标题
     */
    void set_title(const std::string& title);

    /**
     * @brief 获取公告内容
     * @return 公告内容
     */
    const std::string& get_content() const;

    /**
     * @brief 设置公告内容
     * @param content 公告内容
     */
    void set_content(const std::string& content);

    /**
     * @brief 获取作者ID
     * @return 作者ID
     */
    long long get_author_id() const;

    /**
     * @brief 设置作者ID
     * @param author_id 作者ID
     */
    void set_author_id(long long author_id);

    /**
     * @brief 获取公告状态
     * @return 公告状态
     */
    AnnouncementStatus get_status() const;

    /**
     * @brief 设置公告状态
     * @param status 公告状态
     */
    void set_status(AnnouncementStatus status);

    /**
     * @brief 获取公告优先级
     * @return 公告优先级
     */
    AnnouncementPriority get_priority() const;

    /**
     * @brief 设置公告优先级
     * @param priority 公告优先级
     */
    void set_priority(AnnouncementPriority priority);

    /**
     * @brief 获取公告类型
     * @return 公告类型
     */
    AnnouncementType get_type() const;

    /**
     * @brief 设置公告类型
     * @param type 公告类型
     */
    void set_type(AnnouncementType type);

    /**
     * @brief 获取公告摘要
     * @return 公告摘要（可选）
     */
    const std::optional<std::string>& get_summary() const;

    /**
     * @brief 设置公告摘要
     * @param summary 公告摘要
     */
    void set_summary(const std::optional<std::string>& summary);

    /**
     * @brief 生成公告摘要
     * @param max_length 摘要最大长度（默认100字符）
     * @return 公告摘要
     */
    std::string generate_summary(size_t max_length = 100) const;

    /**
     * @brief 获取公告标签
     * @return 公告标签列表
     */
    const std::vector<std::string>& get_tags() const;

    /**
     * @brief 设置公告标签
     * @param tags 公告标签列表
     */
    void set_tags(const std::vector<std::string>& tags);

    /**
     * @brief 添加公告标签
     * @param tag 公告标签
     */
    void add_tag(const std::string& tag);

    /**
     * @brief 移除公告标签
     * @param tag 公告标签
     */
    void remove_tag(const std::string& tag);

    /**
     * @brief 检查是否包含标签
     * @param tag 公告标签
     * @return 如果包含返回true，否则返回false
     */
    bool has_tag(const std::string& tag) const;

    /**
     * @brief 获取部门可见性
     * @return 部门可见性列表
     */
    const std::vector<std::string>& get_departments() const;

    /**
     * @brief 设置部门可见性
     * @param departments 部门可见性列表
     */
    void set_departments(const std::vector<std::string>& departments);

    /**
     * @brief 检查是否对所有部门可见
     * @return 如果可见返回true，否则返回false
     */
    bool is_public() const;

    /**
     * @brief 设置对所有部门可见
     */
    void set_public();

    /**
     * @brief 获取阅读权限
     * @return 阅读权限
     */
    const PermissionSet& get_read_permissions() const;

    /**
     * @brief 设置阅读权限
     * @param permissions 阅读权限集合
     */
    void set_read_permissions(const PermissionSet& permissions);

    /**
     * @brief 添加阅读权限
     * @param permission 阅读权限
     */
    void add_read_permission(Permission permission);

    /**
     * @brief 移除阅读权限
     * @param permission 阅读权限
     */
    void remove_read_permission(Permission permission);

    /**
     * @brief 检查是否有阅读权限
     * @param permission 阅读权限
     * @return 如果有返回true，否则返回false
     */
    bool has_read_permission(Permission permission) const;

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
     * @brief 获取发布时间
     * @return 发布时间（可选）
     */
    const std::optional<std::string>& get_published_at() const;

    /**
     * @brief 设置发布时间
     * @param published_at 发布时间
     */
    void set_published_at(const std::optional<std::string>& published_at);

    /**
     * @brief 获取过期时间
     * @return 过期时间（可选）
     */
    const std::optional<std::string>& get_expires_at() const;

    /**
     * @brief 设置过期时间
     * @param expires_at 过期时间
     */
    void set_expires_at(const std::optional<std::string>& expires_at);

    /**
     * @brief 检查是否已过期
     * @return 如果已过期返回true，否则返回false
     */
    bool is_expired() const;

    /**
     * @brief 检查是否是置顶公告
     * @return 如果是置顶公告返回true，否则返回false
     */
    bool is_pinned() const;

    /**
     * @brief 设置置顶状态
     * @param pinned 是否置顶
     */
    void set_pinned(bool pinned);

    /**
     * @brief 获取阅读计数
     * @return 阅读计数
     */
    int get_read_count() const;

    /**
     * @brief 设置阅读计数
     * @param count 阅读计数
     */
    void set_read_count(int count);

    /**
     * @brief 增加阅读计数
     * @param increment 增量（默认1）
     */
    void increment_read_count(int increment = 1);

    /**
     * @brief 获取附件URL列表
     * @return 附件URL列表
     */
    const std::vector<std::string>& get_attachments() const;

    /**
     * @brief 设置附件URL列表
     * @param attachments 附件URL列表
     */
    void set_attachments(const std::vector<std::string>& attachments);

    /**
     * @brief 添加附件URL
     * @param attachment 附件URL
     */
    void add_attachment(const std::string& attachment);

    /**
     * @brief 移除附件URL
     * @param attachment 附件URL
     */
    void remove_attachment(const std::string& attachment);

    /**
     * @brief 获取访问密码
     * @return 访问密码（可选）
     */
    const std::optional<std::string>& get_password() const;

    /**
     * @brief 设置访问密码
     * @param password 访问密码
     */
    void set_password(const std::optional<std::string>& password);

    /**
     * @brief 验证访问密码
     * @param password 待验证的密码
     * @return 如果验证通过返回true，否则返回false
     */
    bool verify_password(const std::string& password) const;

    /**
     * @brief 获取颜色主题
     * @return 颜色主题（可选）
     */
    const std::optional<std::string>& get_color() const;

    /**
     * @brief 设置颜色主题
     * @param color 颜色主题（如 #FF0000 或 red）
     */
    void set_color(const std::optional<std::string>& color);

    /**
     * @brief 将状态转换为字符串
     * @return 状态字符串
     */
    std::string status_to_string() const;

    /**
     * @brief 将优先级转换为字符串
     * @return 优先级字符串
     */
    std::string priority_to_string() const;

    /**
     * @brief 将类型转换为字符串
     * @return 类型字符串
     */
    std::string type_to_string() const;

    /**
     * @brief 将字符串转换为状态
     * @param status_str 状态字符串
     * @return 状态枚举（可选）
     */
    static std::optional<AnnouncementStatus> string_to_status(const std::string& status_str);

    /**
     * @brief 将字符串转换为优先级
     * @param priority_str 优先级字符串
     * @return 优先级枚举（可选）
     */
    static std::optional<AnnouncementPriority> string_to_priority(const std::string& priority_str);

    /**
     * @brief 将字符串转换为类型
     * @param type_str 类型字符串
     * @return 类型枚举（可选）
     */
    static std::optional<AnnouncementType> string_to_type(const std::string& type_str);

    /**
     * @brief 将状态转换为字符串
     * @param status 状态枚举
     * @return 状态字符串
     */
    static std::string status_to_string(AnnouncementStatus status);

    /**
     * @brief 将优先级转换为字符串
     * @param priority 优先级枚举
     * @return 优先级字符串
     */
    static std::string priority_to_string(AnnouncementPriority priority);

    /**
     * @brief 将类型转换为字符串
     * @param type 类型枚举
     * @return 类型字符串
     */
    static std::string type_to_string(AnnouncementType type);

    /**
     * @brief 检查公告标题是否有效
     * @param title 公告标题
     * @return 如果有效返回true，否则返回false
     */
    static bool is_valid_title(const std::string& title);

    /**
     * @brief 检查公告内容是否有效
     * @param content 公告内容
     * @return 如果有效返回true，否则返回false
     */
    static bool is_valid_content(const std::string& content);

    /**
     * @brief 获取标签的正则表达式
     * @return 标签正则表达式
     */
    static std::regex get_tag_regex();

    /**
     * @brief 获取颜色的正则表达式
     * @return 颜色正则表达式
     */
    static std::regex get_color_regex();

    /**
     * @brief 权限哈希结构体
     */
    struct PermissionHash {
        std::size_t operator()(Permission permission) const {
            return static_cast<std::size_t>(permission);
        }
    };

private:
    long long id_;                     // 公告ID
    std::string title_;                // 公告标题
    std::string content_;              // 公告内容
    long long author_id_;              // 作者ID
    AnnouncementStatus status_;        // 公告状态
    AnnouncementPriority priority_;    // 公告优先级
    AnnouncementType type_;            // 公告类型
    std::optional<std::string> summary_; // 公告摘要
    std::vector<std::string> tags_;    // 公告标签
    std::vector<std::string> departments_; // 部门可见性
    PermissionSet read_permissions_;   // 阅读权限
    std::optional<std::string> created_at_;   // 创建时间
    std::optional<std::string> updated_at_;   // 更新时间
    std::optional<std::string> published_at_; // 发布时间
    std::optional<std::string> expires_at_;   // 过期时间
    bool pinned_;                      // 是否置顶
    int read_count_;                   // 阅读计数
    std::vector<std::string> attachments_; // 附件URL列表
    std::optional<std::string> password_;    // 访问密码
    std::optional<std::string> color_;       // 颜色主题

    /**
     * @brief 清空部门可见性
     */
    void clear_departments_();

    /**
     * @brief 检查字符串是否是有效的十六进制颜色
     * @param color 颜色字符串
     * @return 如果有效返回true，否则返回false
     */
    bool is_valid_hex_color_(const std::string& color) const;
};

/**
 * @brief 比较两个公告是否相等
 * @param lhs 左操作数
 * @param rhs 右操作数
 * @return 如果相等返回true，否则返回false
 */
bool operator==(const Announcement& lhs, const Announcement& rhs);

/**
 * @brief 比较两个公告是否不相等
 * @param lhs 左操作数
 * @param rhs 右操作数
 * @return 如果不相等返回true，否则返回false
 */
bool operator!=(const Announcement& lhs, const Announcement& rhs);

} // namespace domain
