#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include <optional>
#include <algorithm>

namespace domain {

// 用户角色枚举
enum class UserRole {
    ADMIN = 1,          // 系统管理员
    MANAGER = 2,        // 部门经理
    EMPLOYEE = 3,       // 普通员工
    AUDITOR = 4,        // 审计人员
    SUPER_ADMIN = 5,    // 超级管理员
    GUEST = 6           // 访客
};

// 用户状态枚举
enum class UserStatus {
    ACTIVE = 1,     // 激活
    INACTIVE = 2,   // 未激活
    SUSPENDED = 3,  // 已停用
    PENDING = 4,    // 待审批
    DELETED = 5     // 已删除（逻辑删除）
};

// 权限枚举
enum class Permission {
    // 用户管理
    USER_READ = 1,         // 查看用户
    USER_CREATE = 2,       // 创建用户
    USER_UPDATE = 3,       // 更新用户
    USER_DELETE = 4,       // 删除用户
    USER_IMPORT = 5,       // 导入用户
    USER_EXPORT = 6,       // 导出用户
    
    // 公告管理
    ANN_READ = 10,         // 查看公告
    ANN_CREATE = 11,       // 创建公告
    ANN_UPDATE = 12,       // 更新公告
    ANN_DELETE = 13,       // 删除公告
    ANN_PUBLISH = 14,      // 发布公告
    ANN_ARCHIVE = 15,      // 归档公告
    
    // 系统管理
    CONFIG_READ = 20,      // 查看配置
    CONFIG_UPDATE = 21,    // 更新配置
    LOG_VIEW = 22,         // 查看日志
    LOG_EXPORT = 23,       // 导出日志
    BACKUP = 24,           // 备份数据
    RESTORE = 25,          // 恢复数据
    
    // 审批管理
    APPROVAL_REQUEST = 30, // 提交审批
    APPROVAL_APPROVE = 31, // 审批通过
    APPROVAL_REJECT = 32,  // 审批拒绝
    APPROVAL_VIEW = 33,    // 查看审批
    
    // 其他权限
    DASHBOARD = 40,        // 查看仪表板
    REPORT = 41,           // 生成报告
    NOTIFICATION = 42,     // 接收通知
    SETTINGS = 43          // 个人设置
};

// 用户类
class User {
public:
    using PermissionSet = std::unordered_set<Permission>;
    using Role = UserRole;
    using Status = UserStatus;
    
    // 构造函数
    User();
    User(
        const std::string& username,
        const std::string& email,
        const std::string& password_hash,
        Role role = Role::EMPLOYEE,
        Status status = Status::ACTIVE
    );
    
    // 带ID的构造函数（用于数据库查询）
    User(
        long long id,
        const std::string& username,
        const std::string& email,
        const std::string& password_hash,
        Role role = Role::EMPLOYEE,
        Status status = Status::ACTIVE
    );
    
    // 析构函数
    virtual ~User();
    
    // 获取ID
    long long get_id() const;
    
    // 设置ID
    void set_id(long long id);
    
    // 获取用户名
    const std::string& get_username() const;
    
    // 设置用户名
    void set_username(const std::string& username);
    
    // 获取邮箱
    const std::string& get_email() const;
    
    // 设置邮箱
    void set_email(const std::string& email);
    
    // 获取密码哈希
    const std::string& get_password_hash() const;
    
    // 设置密码哈希
    void set_password_hash(const std::string& password_hash);
    
    // 获取角色
    Role get_role() const;
    
    // 设置角色
    void set_role(Role role);
    
    // 获取状态
    Status get_status() const;
    
    // 设置状态
    void set_status(Status status);
    
    // 获取名字
    const std::optional<std::string>& get_first_name() const;
    
    // 设置名字
    void set_first_name(const std::optional<std::string>& first_name);
    
    // 获取姓氏
    const std::optional<std::string>& get_last_name() const;
    
    // 设置姓氏
    void set_last_name(const std::optional<std::string>& last_name);
    
    // 获取全名
    std::string get_full_name() const;
    
    // 获取部门
    const std::optional<std::string>& get_department() const;
    
    // 设置部门
    void set_department(const std::optional<std::string>& department);
    
    // 获取职位
    const std::optional<std::string>& get_position() const;
    
    // 设置职位
    void set_position(const std::optional<std::string>& position);
    
    // 获取手机号码
    const std::optional<std::string>& get_phone() const;
    
    // 设置手机号码
    void set_phone(const std::optional<std::string>& phone);
    
    // 获取创建时间
    const std::optional<std::string>& get_created_at() const;
    
    // 设置创建时间
    void set_created_at(const std::optional<std::string>& created_at);
    
    // 获取更新时间
    const std::optional<std::string>& get_updated_at() const;
    
    // 设置更新时间
    void set_updated_at(const std::optional<std::string>& updated_at);
    
    // 获取最后登录时间
    const std::optional<std::string>& get_last_login_at() const;
    
    // 设置最后登录时间
    void set_last_login_at(const std::optional<std::string>& last_login_at);
    
    // 检查用户是否处于激活状态
    bool is_active() const;
    
    // 检查用户是否为管理员
    bool is_admin() const;
    
    // 检查用户是否为超级管理员
    bool is_super_admin() const;
    
    // 检查用户是否为经理
    bool is_manager() const;
    
    // 检查用户是否为普通员工
    bool is_employee() const;
    
    // 检查用户是否为审计人员
    bool is_auditor() const;
    
    // 检查用户是否为访客
    bool is_guest() const;
    
    // 获取权限集合
    PermissionSet get_permissions() const;
    
    // 检查用户是否有特定权限
    bool has_permission(Permission permission) const;
    
    // 检查用户是否有任意权限
    bool has_any_permission(const PermissionSet& permissions) const;
    
    // 检查用户是否有所有权限
    bool has_all_permissions(const PermissionSet& permissions) const;
    
    // 检查用户是否有权限访问公告
    bool can_view_announcements() const;
    
    // 检查用户是否有权限创建公告
    bool can_create_announcements() const;
    
    // 检查用户是否有权限更新公告
    bool can_update_announcements() const;
    
    // 检查用户是否有权限删除公告
    bool can_delete_announcements() const;
    
    // 检查用户是否有权限发布公告
    bool can_publish_announcements() const;
    
    // 检查用户是否有权限查看用户
    bool can_view_users() const;
    
    // 检查用户是否有权限创建用户
    bool can_create_users() const;
    
    // 检查用户是否有权限更新用户
    bool can_update_users() const;
    
    // 检查用户是否有权限删除用户
    bool can_delete_users() const;
    
    // 检查用户是否有权限查看配置
    bool can_view_config() const;
    
    // 检查用户是否有权限更新配置
    bool can_update_config() const;
    
    // 检查用户是否有权限查看日志
    bool can_view_logs() const;
    
    // 检查用户是否有权限导出日志
    bool can_export_logs() const;
    
    // 检查用户是否有权限备份数据
    bool can_backup_data() const;
    
    // 检查用户是否有权限恢复数据
    bool can_restore_data() const;
    
    // 获取角色的字符串表示
    std::string role_to_string() const;
    
    // 获取状态的字符串表示
    std::string status_to_string() const;
    
    // 角色转字符串（静态方法）
    static std::string role_to_string(Role role);
    
    // 状态转字符串（静态方法）
    static std::string status_to_string(Status status);
    
    // 字符串转角色
    static std::optional<Role> string_to_role(const std::string& role_str);
    
    // 字符串转状态
    static std::optional<Status> string_to_status(const std::string& status_str);
    
    // 获取权限的字符串表示
    static std::string permission_to_string(Permission permission);
    
    // 字符串转权限
    static std::optional<Permission> string_to_permission(const std::string& permission_str);
    
    // 获取指定角色的默认权限
    static PermissionSet get_default_permissions(Role role);
    
    // 验证用户名格式
    static bool is_valid_username(const std::string& username);
    
    // 验证邮箱格式
    static bool is_valid_email(const std::string& email);
    
    // 验证密码强度
    static bool is_valid_password(const std::string& password);
    
private:
    long long id_;
    std::string username_;
    std::string email_;
    std::string password_hash_;
    Role role_;
    Status status_;
    std::optional<std::string> first_name_;
    std::optional<std::string> last_name_;
    std::optional<std::string> department_;
    std::optional<std::string> position_;
    std::optional<std::string> phone_;
    std::optional<std::string> created_at_;
    std::optional<std::string> updated_at_;
    std::optional<std::string> last_login_at_;
    
    // 私有辅助函数
    void update_permissions_();
};

// 用户集合类型
using UserList = std::vector<User>;
using UserPtr = std::shared_ptr<User>;
using UserPtrList = std::vector<UserPtr>;

// 用户比较函数
bool operator==(const User& lhs, const User& rhs);
bool operator!=(const User& lhs, const User& rhs);

// 权限比较函数
bool operator==(Permission lhs, Permission rhs);

// 权限哈希函数（用于unordered_set）
struct PermissionHash {
    std::size_t operator()(Permission permission) const;
};

} // namespace domain

#endif // USER_H
