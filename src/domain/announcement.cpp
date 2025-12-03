#include "announcement.h"
#include <sstream>
#include <regex>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <iomanip>
#include <cctype>

namespace domain {

using namespace std::string_literals;

// 正则表达式定义
const std::regex Announcement::get_tag_regex() {
    return std::regex("^[a-zA-Z0-9_.-]+$");
}

const std::regex Announcement::get_color_regex() {
    return std::regex("^#([A-Fa-f0-9]{6}|[A-Fa-f0-9]{3})$");
}

// Announcement 构造函数实现
Announcement::Announcement() 
    : id_(0), author_id_(0), status_(AnnouncementStatus::DRAFT), 
      priority_(AnnouncementPriority::MEDIUM), type_(AnnouncementType::NOTICE), 
      pinned_(false), read_count_(0) {
}

Announcement::Announcement(
    const std::string& title,
    const std::string& content,
    long long author_id,
    AnnouncementStatus status,
    AnnouncementPriority priority,
    AnnouncementType type
) : id_(0), title_(title), content_(content), author_id_(author_id), 
    status_(status), priority_(priority), type_(type), 
    pinned_(false), read_count_(0) {
}

Announcement::Announcement(
    long long id,
    const std::string& title,
    const std::string& content,
    long long author_id,
    AnnouncementStatus status,
    AnnouncementPriority priority,
    AnnouncementType type
) : id_(id), title_(title), content_(content), author_id_(author_id), 
    status_(status), priority_(priority), type_(type), 
    pinned_(false), read_count_(0) {
}

Announcement::~Announcement() {
}

// Getter 和 Setter 实现
long long Announcement::get_id() const {
    return id_;
}

void Announcement::set_id(long long id) {
    id_ = id;
}

const std::string& Announcement::get_title() const {
    return title_;
}

void Announcement::set_title(const std::string& title) {
    title_ = title;
}

const std::string& Announcement::get_content() const {
    return content_;
}

void Announcement::set_content(const std::string& content) {
    content_ = content;
}

long long Announcement::get_author_id() const {
    return author_id_;
}

void Announcement::set_author_id(long long author_id) {
    author_id_ = author_id;
}

AnnouncementStatus Announcement::get_status() const {
    return status_;
}

void Announcement::set_status(AnnouncementStatus status) {
    status_ = status;
}

AnnouncementPriority Announcement::get_priority() const {
    return priority_;
}

void Announcement::set_priority(AnnouncementPriority priority) {
    priority_ = priority;
}

AnnouncementType Announcement::get_type() const {
    return type_;
}

void Announcement::set_type(AnnouncementType type) {
    type_ = type;
}

const std::optional<std::string>& Announcement::get_summary() const {
    return summary_;
}

void Announcement::set_summary(const std::optional<std::string>& summary) {
    summary_ = summary;
}

std::string Announcement::generate_summary(size_t max_length) const {
    if (content_.empty()) {
        return "";
    }
    
    std::string trimmed_content = content_;
    // 移除HTML标签
    trimmed_content = std::regex_replace(trimmed_content, std::regex("<.*?>"), "");
    
    if (trimmed_content.length() <= max_length) {
        return trimmed_content;
    }
    
    // 截断到max_length字符，确保不破坏单词
    std::string summary = trimmed_content.substr(0, max_length);
    size_t last_space = summary.find_last_of(" ");
    
    if (last_space != std::string::npos && last_space > max_length / 2) {
        summary = summary.substr(0, last_space);
    }
    
    summary += "...";
    return summary;
}

const std::vector<std::string>& Announcement::get_tags() const {
    return tags_;
}

void Announcement::set_tags(const std::vector<std::string>& tags) {
    tags_ = tags;
}

void Announcement::add_tag(const std::string& tag) {
    if (std::find(tags_.begin(), tags_.end(), tag) == tags_.end()) {
        tags_.push_back(tag);
    }
}

void Announcement::remove_tag(const std::string& tag) {
    auto it = std::find(tags_.begin(), tags_.end(), tag);
    if (it != tags_.end()) {
        tags_.erase(it);
    }
}

bool Announcement::has_tag(const std::string& tag) const {
    return std::find(tags_.begin(), tags_.end(), tag) != tags_.end();
}

const std::vector<std::string>& Announcement::get_departments() const {
    return departments_;
}

void Announcement::set_departments(const std::vector<std::string>& departments) {
    departments_ = departments;
}

bool Announcement::is_public() const {
    return departments_.empty();
}

void Announcement::set_public() {
    clear_departments_();
}

const Announcement::PermissionSet& Announcement::get_read_permissions() const {
    return read_permissions_;
}

void Announcement::set_read_permissions(const PermissionSet& permissions) {
    read_permissions_ = permissions;
}

void Announcement::add_read_permission(Permission permission) {
    read_permissions_.insert(permission);
}

void Announcement::remove_read_permission(Permission permission) {
    read_permissions_.erase(permission);
}

bool Announcement::has_read_permission(Permission permission) const {
    return read_permissions_.count(permission) > 0;
}

const std::optional<std::string>& Announcement::get_created_at() const {
    return created_at_;
}

void Announcement::set_created_at(const std::optional<std::string>& created_at) {
    created_at_ = created_at;
}

const std::optional<std::string>& Announcement::get_updated_at() const {
    return updated_at_;
}

void Announcement::set_updated_at(const std::optional<std::string>& updated_at) {
    updated_at_ = updated_at;
}

const std::optional<std::string>& Announcement::get_published_at() const {
    return published_at_;
}

void Announcement::set_published_at(const std::optional<std::string>& published_at) {
    published_at_ = published_at;
}

const std::optional<std::string>& Announcement::get_expires_at() const {
    return expires_at_;
}

void Announcement::set_expires_at(const std::optional<std::string>& expires_at) {
    expires_at_ = expires_at;
}

bool Announcement::is_expired() const {
    if (!expires_at_) {
        return false;
    }
    
    // 简单的过期检查，实际应用中应该解析时间戳进行比较
    return false;
}

bool Announcement::is_pinned() const {
    return pinned_;
}

void Announcement::set_pinned(bool pinned) {
    pinned_ = pinned;
}

int Announcement::get_read_count() const {
    return read_count_;
}

void Announcement::set_read_count(int count) {
    read_count_ = count;
}

void Announcement::increment_read_count(int increment) {
    read_count_ += increment;
}

const std::vector<std::string>& Announcement::get_attachments() const {
    return attachments_;
}

void Announcement::set_attachments(const std::vector<std::string>& attachments) {
    attachments_ = attachments;
}

void Announcement::add_attachment(const std::string& attachment) {
    attachments_.push_back(attachment);
}

void Announcement::remove_attachment(const std::string& attachment) {
    auto it = std::find(attachments_.begin(), attachments_.end(), attachment);
    if (it != attachments_.end()) {
        attachments_.erase(it);
    }
}

const std::optional<std::string>& Announcement::get_password() const {
    return password_;
}

void Announcement::set_password(const std::optional<std::string>& password) {
    password_ = password;
}

bool Announcement::verify_password(const std::string& password) const {
    if (!password_) {
        return true; // 没有密码时，任何密码都通过
    }
    return password == *password_;
}

const std::optional<std::string>& Announcement::get_color() const {
    return color_;
}

void Announcement::set_color(const std::optional<std::string>& color) {
    if (color && !is_valid_hex_color_(*color)) {
        // 无效的颜色格式，忽略或抛出异常
        return;
    }
    color_ = color;
}

// 枚举转换方法
std::string Announcement::status_to_string() const {
    return status_to_string(status_);
}

std::string Announcement::priority_to_string() const {
    return priority_to_string(priority_);
}

std::string Announcement::type_to_string() const {
    return type_to_string(type_);
}

std::string Announcement::status_to_string(AnnouncementStatus status) {
    switch (status) {
        case AnnouncementStatus::DRAFT: return "draft";
        case AnnouncementStatus::PUBLISHED: return "published";
        case AnnouncementStatus::ARCHIVED: return "archived";
        case AnnouncementStatus::DELETED: return "deleted";
        case AnnouncementStatus::PENDING: return "pending";
        case AnnouncementStatus::REJECTED: return "rejected";
        default: return "unknown";
    }
}

std::string Announcement::priority_to_string(AnnouncementPriority priority) {
    switch (priority) {
        case AnnouncementPriority::LOW: return "low";
        case AnnouncementPriority::MEDIUM: return "medium";
        case AnnouncementPriority::HIGH: return "high";
        case AnnouncementPriority::URGENT: return "urgent";
        default: return "unknown";
    }
}

std::string Announcement::type_to_string(AnnouncementType type) {
    switch (type) {
        case AnnouncementType::NOTICE: return "notice";
        case AnnouncementType::ANNOUNCEMENT: return "announcement";
        case AnnouncementType::UPDATE: return "update";
        case AnnouncementType::ALERT: return "alert";
        case AnnouncementType::NEWS: return "news";
        case AnnouncementType::EVENT: return "event";
        case AnnouncementType::MEMO: return "memo";
        case AnnouncementType::POLICY: return "policy";
        default: return "unknown";
    }
}

std::optional<AnnouncementStatus> Announcement::string_to_status(const std::string& status_str) {
    std::string lower_status = status_str;
    std::transform(lower_status.begin(), lower_status.end(), lower_status.begin(), ::tolower);
    
    if (lower_status == "draft") return AnnouncementStatus::DRAFT;
    if (lower_status == "published") return AnnouncementStatus::PUBLISHED;
    if (lower_status == "archived") return AnnouncementStatus::ARCHIVED;
    if (lower_status == "deleted") return AnnouncementStatus::DELETED;
    if (lower_status == "pending") return AnnouncementStatus::PENDING;
    if (lower_status == "rejected") return AnnouncementStatus::REJECTED;
    
    return std::nullopt;
}

std::optional<AnnouncementPriority> Announcement::string_to_priority(const std::string& priority_str) {
    std::string lower_priority = priority_str;
    std::transform(lower_priority.begin(), lower_priority.end(), lower_priority.begin(), ::tolower);
    
    if (lower_priority == "low") return AnnouncementPriority::LOW;
    if (lower_priority == "medium") return AnnouncementPriority::MEDIUM;
    if (lower_priority == "high") return AnnouncementPriority::HIGH;
    if (lower_priority == "urgent") return AnnouncementPriority::URGENT;
    
    return std::nullopt;
}

std::optional<AnnouncementType> Announcement::string_to_type(const std::string& type_str) {
    std::string lower_type = type_str;
    std::transform(lower_type.begin(), lower_type.end(), lower_type.begin(), ::tolower);
    
    if (lower_type == "notice") return AnnouncementType::NOTICE;
    if (lower_type == "announcement") return AnnouncementType::ANNOUNCEMENT;
    if (lower_type == "update") return AnnouncementType::UPDATE;
    if (lower_type == "alert") return AnnouncementType::ALERT;
    if (lower_type == "news") return AnnouncementType::NEWS;
    if (lower_type == "event") return AnnouncementType::EVENT;
    if (lower_type == "memo") return AnnouncementType::MEMO;
    if (lower_type == "policy") return AnnouncementType::POLICY;
    
    return std::nullopt;
}

// 验证方法
bool Announcement::is_valid_title(const std::string& title) {
    if (title.empty() || title.length() < 1 || title.length() > 200) {
        return false;
    }
    return true;
}

bool Announcement::is_valid_content(const std::string& content) {
    if (content.empty() || content.length() < 1) {
        return false;
    }
    return true;
}

// 私有辅助函数
void Announcement::clear_departments_() {
    departments_.clear();
}

bool Announcement::is_valid_hex_color_(const std::string& color) const {
    if (color.empty()) {
        return false;
    }
    
    return std::regex_match(color, get_color_regex());
}

// 公告比较运算符
bool operator==(const Announcement& lhs, const Announcement& rhs) {
    return lhs.get_id() == rhs.get_id();
}

bool operator!=(const Announcement& lhs, const Announcement& rhs) {
    return !(lhs == rhs);
}

} // namespace domain
