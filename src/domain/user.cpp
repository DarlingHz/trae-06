#include "user.h"
#include <sstream>
#include <regex>
#include <cctype>
#include <algorithm>

namespace domain {

using namespace std::string_literals;

// User 构造函数实现
User::User() 
    : id_(0), role_(UserRole::EMPLOYEE), status_(UserStatus::ACTIVE) {
}

User::User(
    const std::string& username,
    const std::string& email,
    const std::string& password_hash,
    UserRole role,
    UserStatus status
) : id_(0), username_(username), email_(email), password_hash_(password_hash), role_(role), status_(status) {
}

User::User(
    long long id,
    const std::string& username,
    const std::string& email,
    const std::string& password_hash,
    UserRole role,
    UserStatus status
) : id_(id), username_(username), email_(email), password_hash_(password_hash), role_(role), status_(status) {
}

User::~User() {
}

// Getter 和 Setter 实现
long long User::get_id() const {
    return id_;
}

void User::set_id(long long id) {
    id_ = id;
}

const std::string& User::get_username() const {
    return username_;
}

void User::set_username(const std::string& username) {
    username_ = username;
}

const std::string& User::get_email() const {
    return email_;
}

void User::set_email(const std::string& email) {
    email_ = email;
}

const std::string& User::get_password_hash() const {
    return password_hash_;
}

void User::set_password_hash(const std::string& password_hash) {
    password_hash_ = password_hash;
}

UserRole User::get_role() const {
    return role_;
}

void User::set_role(UserRole role) {
    role_ = role;
}

UserStatus User::get_status() const {
    return status_;
}

void User::set_status(UserStatus status) {
    status_ = status;
}

const std::optional<std::string>& User::get_first_name() const {
    return first_name_;
}

void User::set_first_name(const std::optional<std::string>& first_name) {
    first_name_ = first_name;
}

const std::optional<std::string>& User::get_last_name() const {
    return last_name_;
}

void User::set_last_name(const std::optional<std::string>& last_name) {
    last_name_ = last_name;
}

std::string User::get_full_name() const {
    if (first_name_ && last_name_) {
        return *first_name_ + " " + *last_name_;
    } else if (first_name_) {
        return *first_name_;
    } else if (last_name_) {
        return *last_name_;
    } else {
        return username_;
    }
}

const std::optional<std::string>& User::get_department() const {
    return department_;
}

void User::set_department(const std::optional<std::string>& department) {
    department_ = department;
}

const std::optional<std::string>& User::get_position() const {
    return position_;
}

void User::set_position(const std::optional<std::string>& position) {
    position_ = position;
}

const std::optional<std::string>& User::get_phone() const {
    return phone_;
}

void User::set_phone(const std::optional<std::string>& phone) {
    phone_ = phone;
}

const std::optional<std::string>& User::get_created_at() const {
    return created_at_;
}

void User::set_created_at(const std::optional<std::string>& created_at) {
    created_at_ = created_at;
}

const std::optional<std::string>& User::get_updated_at() const {
    return updated_at_;
}

void User::set_updated_at(const std::optional<std::string>& updated_at) {
    updated_at_ = updated_at;
}

const std::optional<std::string>& User::get_last_login_at() const {
    return last_login_at_;
}

void User::set_last_login_at(const std::optional<std::string>& last_login_at) {
    last_login_at_ = last_login_at;
}

// 状态检查方法
bool User::is_active() const {
    return status_ == UserStatus::ACTIVE;
}

bool User::is_admin() const {
    return role_ == UserRole::ADMIN || role_ == UserRole::SUPER_ADMIN;
}

bool User::is_super_admin() const {
    return role_ == UserRole::SUPER_ADMIN;
}

bool User::is_manager() const {
    return role_ == UserRole::MANAGER;
}

bool User::is_employee() const {
    return role_ == UserRole::EMPLOYEE;
}

bool User::is_auditor() const {
    return role_ == UserRole::AUDITOR;
}

bool User::is_guest() const {
    return role_ == UserRole::GUEST;
}

// 权限检查方法
User::PermissionSet User::get_permissions() const {
    return get_default_permissions(role_);
}

bool User::has_permission(Permission permission) const {
    const PermissionSet& permissions = get_permissions();
    return permissions.count(permission) > 0;
}

bool User::has_any_permission(const PermissionSet& permissions) const {
    const PermissionSet& user_permissions = get_permissions();
    
    for (const auto& permission : permissions) {
        if (user_permissions.count(permission)) {
            return true;
        }
    }
    return false;
}

bool User::has_all_permissions(const PermissionSet& permissions) const {
    const PermissionSet& user_permissions = get_permissions();
    
    for (const auto& permission : permissions) {
        if (!user_permissions.count(permission)) {
            return false;
        }
    }
    return true;
}

bool User::can_view_announcements() const {
    return has_permission(Permission::ANN_READ);
}

bool User::can_create_announcements() const {
    return has_permission(Permission::ANN_CREATE);
}

bool User::can_update_announcements() const {
    return has_permission(Permission::ANN_UPDATE);
}

bool User::can_delete_announcements() const {
    return has_permission(Permission::ANN_DELETE);
}

bool User::can_publish_announcements() const {
    return has_permission(Permission::ANN_PUBLISH);
}

bool User::can_view_users() const {
    return has_permission(Permission::USER_READ);
}

bool User::can_create_users() const {
    return has_permission(Permission::USER_CREATE);
}

bool User::can_update_users() const {
    return has_permission(Permission::USER_UPDATE);
}

bool User::can_delete_users() const {
    return has_permission(Permission::USER_DELETE);
}

bool User::can_view_config() const {
    return has_permission(Permission::CONFIG_READ);
}

bool User::can_update_config() const {
    return has_permission(Permission::CONFIG_UPDATE);
}

bool User::can_view_logs() const {
    return has_permission(Permission::LOG_VIEW);
}

bool User::can_export_logs() const {
    return has_permission(Permission::LOG_EXPORT);
}

bool User::can_backup_data() const {
    return has_permission(Permission::BACKUP);
}

bool User::can_restore_data() const {
    return has_permission(Permission::RESTORE);
}

// 枚举转换方法
std::string User::role_to_string() const {
    return role_to_string(role_);
}

std::string User::status_to_string() const {
    return status_to_string(status_);
}

std::string User::role_to_string(UserRole role) {
    switch (role) {
        case UserRole::ADMIN: return "admin";
        case UserRole::MANAGER: return "manager";
        case UserRole::EMPLOYEE: return "employee";
        case UserRole::AUDITOR: return "auditor";
        case UserRole::SUPER_ADMIN: return "super_admin";
        case UserRole::GUEST: return "guest";
        default: return "unknown";
    }
}

std::string User::status_to_string(UserStatus status) {
    switch (status) {
        case UserStatus::ACTIVE: return "active";
        case UserStatus::INACTIVE: return "inactive";
        case UserStatus::SUSPENDED: return "suspended";
        case UserStatus::PENDING: return "pending";
        case UserStatus::DELETED: return "deleted";
        default: return "unknown";
    }
}

std::optional<UserRole> User::string_to_role(const std::string& role_str) {
    std::string lower_role = role_str;
    std::transform(lower_role.begin(), lower_role.end(), lower_role.begin(), ::tolower);
    
    if (lower_role == "admin") return UserRole::ADMIN;
    if (lower_role == "manager") return UserRole::MANAGER;
    if (lower_role == "employee") return UserRole::EMPLOYEE;
    if (lower_role == "auditor") return UserRole::AUDITOR;
    if (lower_role == "super_admin" || lower_role == "superadmin") return UserRole::SUPER_ADMIN;
    if (lower_role == "guest") return UserRole::GUEST;
    
    return std::nullopt;
}

std::optional<UserStatus> User::string_to_status(const std::string& status_str) {
    std::string lower_status = status_str;
    std::transform(lower_status.begin(), lower_status.end(), lower_status.begin(), ::tolower);
    
    if (lower_status == "active") return UserStatus::ACTIVE;
    if (lower_status == "inactive") return UserStatus::INACTIVE;
    if (lower_status == "suspended") return UserStatus::SUSPENDED;
    if (lower_status == "pending") return UserStatus::PENDING;
    if (lower_status == "deleted") return UserStatus::DELETED;
    
    return std::nullopt;
}

std::string User::permission_to_string(Permission permission) {
    switch (permission) {
        case Permission::USER_READ: return "user_read";
        case Permission::USER_CREATE: return "user_create";
        case Permission::USER_UPDATE: return "user_update";
        case Permission::USER_DELETE: return "user_delete";
        case Permission::USER_IMPORT: return "user_import";
        case Permission::USER_EXPORT: return "user_export";
        case Permission::ANN_READ: return "ann_read";
        case Permission::ANN_CREATE: return "ann_create";
        case Permission::ANN_UPDATE: return "ann_update";
        case Permission::ANN_DELETE: return "ann_delete";
        case Permission::ANN_PUBLISH: return "ann_publish";
        case Permission::ANN_ARCHIVE: return "ann_archive";
        case Permission::CONFIG_READ: return "config_read";
        case Permission::CONFIG_UPDATE: return "config_update";
        case Permission::LOG_VIEW: return "log_view";
        case Permission::LOG_EXPORT: return "log_export";
        case Permission::BACKUP: return "backup";
        case Permission::RESTORE: return "restore";
        case Permission::APPROVAL_REQUEST: return "approval_request";
        case Permission::APPROVAL_APPROVE: return "approval_approve";
        case Permission::APPROVAL_REJECT: return "approval_reject";
        case Permission::APPROVAL_VIEW: return "approval_view";
        case Permission::DASHBOARD: return "dashboard";
        case Permission::REPORT: return "report";
        case Permission::NOTIFICATION: return "notification";
        case Permission::SETTINGS: return "settings";
        default: return "unknown";
    }
}

std::optional<Permission> User::string_to_permission(const std::string& permission_str) {
    std::string lower_permission = permission_str;
    std::transform(lower_permission.begin(), lower_permission.end(), lower_permission.begin(), ::tolower);
    
    if (lower_permission == "user_read") return Permission::USER_READ;
    if (lower_permission == "user_create") return Permission::USER_CREATE;
    if (lower_permission == "user_update") return Permission::USER_UPDATE;
    if (lower_permission == "user_delete") return Permission::USER_DELETE;
    if (lower_permission == "user_import") return Permission::USER_IMPORT;
    if (lower_permission == "user_export") return Permission::USER_EXPORT;
    if (lower_permission == "ann_read" || lower_permission == "announcement_read") return Permission::ANN_READ;
    if (lower_permission == "ann_create" || lower_permission == "announcement_create") return Permission::ANN_CREATE;
    if (lower_permission == "ann_update" || lower_permission == "announcement_update") return Permission::ANN_UPDATE;
    if (lower_permission == "ann_delete" || lower_permission == "announcement_delete") return Permission::ANN_DELETE;
    if (lower_permission == "ann_publish" || lower_permission == "announcement_publish") return Permission::ANN_PUBLISH;
    if (lower_permission == "ann_archive" || lower_permission == "announcement_archive") return Permission::ANN_ARCHIVE;
    if (lower_permission == "config_read") return Permission::CONFIG_READ;
    if (lower_permission == "config_update") return Permission::CONFIG_UPDATE;
    if (lower_permission == "log_view") return Permission::LOG_VIEW;
    if (lower_permission == "log_export") return Permission::LOG_EXPORT;
    if (lower_permission == "backup") return Permission::BACKUP;
    if (lower_permission == "restore") return Permission::RESTORE;
    if (lower_permission == "approval_request") return Permission::APPROVAL_REQUEST;
    if (lower_permission == "approval_approve") return Permission::APPROVAL_APPROVE;
    if (lower_permission == "approval_reject") return Permission::APPROVAL_REJECT;
    if (lower_permission == "approval_view") return Permission::APPROVAL_VIEW;
    if (lower_permission == "dashboard") return Permission::DASHBOARD;
    if (lower_permission == "report") return Permission::REPORT;
    if (lower_permission == "notification") return Permission::NOTIFICATION;
    if (lower_permission == "settings") return Permission::SETTINGS;
    
    return std::nullopt;
}

User::PermissionSet User::get_default_permissions(UserRole role) {
    PermissionSet permissions;
    
    switch (role) {
        case UserRole::SUPER_ADMIN: {
            // 超级管理员拥有所有权限
            permissions.insert(Permission::USER_READ);
            permissions.insert(Permission::USER_CREATE);
            permissions.insert(Permission::USER_UPDATE);
            permissions.insert(Permission::USER_DELETE);
            permissions.insert(Permission::USER_IMPORT);
            permissions.insert(Permission::USER_EXPORT);
            permissions.insert(Permission::ANN_READ);
            permissions.insert(Permission::ANN_CREATE);
            permissions.insert(Permission::ANN_UPDATE);
            permissions.insert(Permission::ANN_DELETE);
            permissions.insert(Permission::ANN_PUBLISH);
            permissions.insert(Permission::ANN_ARCHIVE);
            permissions.insert(Permission::CONFIG_READ);
            permissions.insert(Permission::CONFIG_UPDATE);
            permissions.insert(Permission::LOG_VIEW);
            permissions.insert(Permission::LOG_EXPORT);
            permissions.insert(Permission::BACKUP);
            permissions.insert(Permission::RESTORE);
            permissions.insert(Permission::APPROVAL_REQUEST);
            permissions.insert(Permission::APPROVAL_APPROVE);
            permissions.insert(Permission::APPROVAL_REJECT);
            permissions.insert(Permission::APPROVAL_VIEW);
            permissions.insert(Permission::DASHBOARD);
            permissions.insert(Permission::REPORT);
            permissions.insert(Permission::NOTIFICATION);
            permissions.insert(Permission::SETTINGS);
            break;
        }
        case UserRole::ADMIN: {
            // 管理员拥有大部分权限
            permissions.insert(Permission::USER_READ);
            permissions.insert(Permission::USER_CREATE);
            permissions.insert(Permission::USER_UPDATE);
            permissions.insert(Permission::USER_DELETE);
            permissions.insert(Permission::USER_IMPORT);
            permissions.insert(Permission::USER_EXPORT);
            permissions.insert(Permission::ANN_READ);
            permissions.insert(Permission::ANN_CREATE);
            permissions.insert(Permission::ANN_UPDATE);
            permissions.insert(Permission::ANN_DELETE);
            permissions.insert(Permission::ANN_PUBLISH);
            permissions.insert(Permission::ANN_ARCHIVE);
            permissions.insert(Permission::CONFIG_READ);
            permissions.insert(Permission::CONFIG_UPDATE);
            permissions.insert(Permission::LOG_VIEW);
            permissions.insert(Permission::LOG_EXPORT);
            permissions.insert(Permission::BACKUP);
            permissions.insert(Permission::RESTORE);
            permissions.insert(Permission::APPROVAL_REQUEST);
            permissions.insert(Permission::APPROVAL_APPROVE);
            permissions.insert(Permission::APPROVAL_REJECT);
            permissions.insert(Permission::APPROVAL_VIEW);
            permissions.insert(Permission::DASHBOARD);
            permissions.insert(Permission::REPORT);
            permissions.insert(Permission::NOTIFICATION);
            permissions.insert(Permission::SETTINGS);
            break;
        }
        case UserRole::MANAGER: {
            // 部门经理权限
            permissions.insert(Permission::USER_READ);
            permissions.insert(Permission::ANN_READ);
            permissions.insert(Permission::ANN_CREATE);
            permissions.insert(Permission::ANN_UPDATE);
            permissions.insert(Permission::ANN_PUBLISH);
            permissions.insert(Permission::APPROVAL_REQUEST);
            permissions.insert(Permission::APPROVAL_APPROVE);
            permissions.insert(Permission::APPROVAL_REJECT);
            permissions.insert(Permission::APPROVAL_VIEW);
            permissions.insert(Permission::DASHBOARD);
            permissions.insert(Permission::REPORT);
            permissions.insert(Permission::NOTIFICATION);
            permissions.insert(Permission::SETTINGS);
            break;
        }
        case UserRole::EMPLOYEE: {
            // 普通员工权限
            permissions.insert(Permission::ANN_READ);
            permissions.insert(Permission::APPROVAL_REQUEST);
            permissions.insert(Permission::APPROVAL_VIEW);
            permissions.insert(Permission::DASHBOARD);
            permissions.insert(Permission::REPORT);
            permissions.insert(Permission::NOTIFICATION);
            permissions.insert(Permission::SETTINGS);
            break;
        }
        case UserRole::AUDITOR: {
            // 审计人员权限
            permissions.insert(Permission::ANN_READ);
            permissions.insert(Permission::USER_READ);
            permissions.insert(Permission::APPROVAL_VIEW);
            permissions.insert(Permission::LOG_VIEW);
            permissions.insert(Permission::LOG_EXPORT);
            permissions.insert(Permission::DASHBOARD);
            permissions.insert(Permission::REPORT);
            permissions.insert(Permission::SETTINGS);
            break;
        }
        case UserRole::GUEST: {
            // 访客权限（只读）
            permissions.insert(Permission::ANN_READ);
            permissions.insert(Permission::SETTINGS);
            break;
        }
    }
    
    return permissions;
}

// 验证方法
bool User::is_valid_username(const std::string& username) {
    if (username.empty() || username.length() < 3 || username.length() > 50) {
        return false;
    }
    
    const std::regex username_regex("^[a-zA-Z0-9_]+$");
    return std::regex_match(username, username_regex);
}

bool User::is_valid_email(const std::string& email) {
    if (email.empty()) {
        return false;
    }
    
    const std::regex email_regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return std::regex_match(email, email_regex);
}

bool User::is_valid_password(const std::string& password) {
    if (password.length() < 8) {
        return false;
    }
    
    bool has_upper = false;
    bool has_lower = false;
    bool has_digit = false;
    bool has_special = false;
    
    for (char c : password) {
        if (std::isupper(c)) has_upper = true;
        else if (std::islower(c)) has_lower = true;
        else if (std::isdigit(c)) has_digit = true;
        else if (!std::isalnum(c)) has_special = true;
    }
    
    return has_upper && has_lower && has_digit && has_special;
}

// 私有辅助函数
void User::update_permissions_() {
    // 此函数在角色变化时更新权限集合
    // 由于权限是基于角色动态计算的，不需要显式存储
}

// 用户比较运算符
bool operator==(const User& lhs, const User& rhs) {
    return lhs.get_id() == rhs.get_id();
}

bool operator!=(const User& lhs, const User& rhs) {
    return !(lhs == rhs);
}

// 权限比较运算符
bool operator==(Permission lhs, Permission rhs) {
    return static_cast<int>(lhs) == static_cast<int>(rhs);
}

// 权限哈希函数实现
struct PermissionHash {
    std::size_t operator()(Permission permission) const {
        return static_cast<std::size_t>(permission);
    }
};

} // namespace domain
