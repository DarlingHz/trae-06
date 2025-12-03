#include "read_receipt.h"
#include <sstream>
#include <regex>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <algorithm>

namespace domain {

using namespace std::string_literals;

// ReadReceipt 构造函数实现
ReadReceipt::ReadReceipt() 
    : id_(0), user_id_(0), announcement_id_(0), is_read_(false), deleted_(false), version_(1) {
}

ReadReceipt::ReadReceipt(long long user_id, long long announcement_id) 
    : id_(0), user_id_(user_id), announcement_id_(announcement_id), 
      is_read_(false), deleted_(false), version_(1) {
}

ReadReceipt::ReadReceipt(long long id, long long user_id, long long announcement_id) 
    : id_(id), user_id_(user_id), announcement_id_(announcement_id), 
      is_read_(false), deleted_(false), version_(1) {
}

ReadReceipt::ReadReceipt(long long user_id, long long announcement_id, const std::string& read_at) 
    : id_(0), user_id_(user_id), announcement_id_(announcement_id), 
      read_at_(read_at), is_read_(true), deleted_(false), version_(1) {
}

ReadReceipt::ReadReceipt(long long id, long long user_id, long long announcement_id, const std::string& read_at) 
    : id_(id), user_id_(user_id), announcement_id_(announcement_id), 
      read_at_(read_at), is_read_(true), deleted_(false), version_(1) {
}

ReadReceipt::~ReadReceipt() {
}

// Getter 和 Setter 实现
long long ReadReceipt::get_id() const {
    return id_;
}

void ReadReceipt::set_id(long long id) {
    id_ = id;
}

long long ReadReceipt::get_user_id() const {
    return user_id_;
}

void ReadReceipt::set_user_id(long long user_id) {
    user_id_ = user_id;
}

long long ReadReceipt::get_announcement_id() const {
    return announcement_id_;
}

void ReadReceipt::set_announcement_id(long long announcement_id) {
    announcement_id_ = announcement_id;
}

const std::optional<std::string>& ReadReceipt::get_read_at() const {
    return read_at_;
}

void ReadReceipt::set_read_at(const std::optional<std::string>& read_at) {
    read_at_ = read_at;
}

const std::optional<std::string>& ReadReceipt::get_created_at() const {
    return created_at_;
}

void ReadReceipt::set_created_at(const std::optional<std::string>& created_at) {
    created_at_ = created_at;
}

const std::optional<std::string>& ReadReceipt::get_updated_at() const {
    return updated_at_;
}

void ReadReceipt::set_updated_at(const std::optional<std::string>& updated_at) {
    updated_at_ = updated_at;
}

const std::optional<int>& ReadReceipt::get_read_duration() const {
    return read_duration_;
}

void ReadReceipt::set_read_duration(const std::optional<int>& read_duration) {
    if (read_duration && *read_duration < 0) {
        read_duration_ = 0;
    } else {
        read_duration_ = read_duration;
    }
}

const std::optional<std::string>& ReadReceipt::get_location() const {
    return location_;
}

void ReadReceipt::set_location(const std::optional<std::string>& location) {
    location_ = location;
}

const std::optional<std::string>& ReadReceipt::get_device_info() const {
    return device_info_;
}

void ReadReceipt::set_device_info(const std::optional<std::string>& device_info) {
    device_info_ = device_info;
}

const std::optional<std::string>& ReadReceipt::get_ip_address() const {
    return ip_address_;
}

void ReadReceipt::set_ip_address(const std::optional<std::string>& ip_address) {
    if (ip_address && !validate_ip_(*ip_address)) {
        // 无效的IP地址，设置为空或保留原有的IP地址
        return;
    }
    ip_address_ = ip_address;
}

const std::optional<std::string>& ReadReceipt::get_user_agent() const {
    return user_agent_;
}

void ReadReceipt::set_user_agent(const std::optional<std::string>& user_agent) {
    user_agent_ = user_agent;
}

bool ReadReceipt::is_read() const {
    return is_read_;
}

void ReadReceipt::set_read(bool read) {
    is_read_ = read;
}

const std::optional<std::string>& ReadReceipt::get_last_read_at() const {
    return last_read_at_;
}

void ReadReceipt::set_last_read_at(const std::optional<std::string>& last_read_at) {
    last_read_at_ = last_read_at;
}

const std::optional<int>& ReadReceipt::get_read_progress() const {
    return read_progress_;
}

void ReadReceipt::set_read_progress(const std::optional<int>& read_progress) {
    if (read_progress) {
        if (*read_progress < 0) {
            read_progress_ = 0;
        } else if (*read_progress > 100) {
            read_progress_ = 100;
        } else {
            read_progress_ = read_progress;
        }
    } else {
        read_progress_ = std::nullopt;
    }
}

const std::optional<std::string>& ReadReceipt::get_note() const {
    return note_;
}

void ReadReceipt::set_note(const std::optional<std::string>& note) {
    note_ = note;
}

bool ReadReceipt::is_progress_complete() const {
    if (read_progress_) {
        return *read_progress_ >= 95;
    }
    return is_read_;
}

void ReadReceipt::update_read_progress(int progress) {
    if (validate_progress_(progress)) {
        read_progress_ = progress;
    }
}

void ReadReceipt::mark_as_read(const std::string& read_at, std::optional<int> progress, std::optional<int> duration) {
    is_read_ = true;
    read_at_ = read_at;
    last_read_at_ = read_at;
    updated_at_ = read_at;
    
    if (progress && validate_progress_(*progress)) {
        read_progress_ = progress;
    }
    
    if (duration && *duration >= 0) {
        read_duration_ = duration;
    }
    
    increment_version();
}

bool ReadReceipt::is_deleted() const {
    return deleted_;
}

void ReadReceipt::set_deleted(bool deleted) {
    deleted_ = deleted;
}

const std::optional<std::string>& ReadReceipt::get_deleted_at() const {
    return deleted_at_;
}

void ReadReceipt::set_deleted_at(const std::optional<std::string>& deleted_at) {
    deleted_at_ = deleted_at;
}

int ReadReceipt::get_version() const {
    return version_;
}

void ReadReceipt::set_version(int version) {
    if (version >= 1) {
        version_ = version;
    }
}

void ReadReceipt::increment_version() {
    version_++;
}

// 静态验证方法
bool ReadReceipt::is_valid_ip(const std::string& ip) {
    if (ip.empty()) {
        return false;
    }
    
    const std::regex ipv4_regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    const std::regex ipv6_regex("^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6}))$");
    
    return std::regex_match(ip, ipv4_regex) || std::regex_match(ip, ipv6_regex);
}

bool ReadReceipt::is_valid_progress(int progress) {
    return progress >= 0 && progress <= 100;
}

std::string ReadReceipt::get_current_time_iso() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
    
    return oss.str();
}

std::optional<std::chrono::system_clock::time_point> ReadReceipt::parse_iso_time(const std::string& time_str) {
    std::tm tm = {};
    std::istringstream ss(time_str);
    
    // 尝试解析 ISO 8601 格式
    if (ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ")) {
        auto time_point = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        return time_point;
    }
    
    return std::nullopt;
}

// 私有辅助函数
bool ReadReceipt::validate_ip_(const std::string& ip) const {
    return is_valid_ip(ip);
}

bool ReadReceipt::validate_progress_(int progress) const {
    return is_valid_progress(progress);
}

// 阅读记录比较运算符
bool operator==(const ReadReceipt& lhs, const ReadReceipt& rhs) {
    return lhs.id_ == rhs.id_;
}

bool operator!=(const ReadReceipt& lhs, const ReadReceipt& rhs) {
    return !(lhs == rhs);
}

// ReadReceiptFilter 方法实现
void ReadReceiptFilter::reset() {
    user_id.reset();
    announcement_id.reset();
    is_read.reset();
    created_before.reset();
    created_after.reset();
    read_before.reset();
    read_after.reset();
    min_read_duration.reset();
    max_read_duration.reset();
    min_read_progress.reset();
    max_read_progress.reset();
    user_ids.reset();
    announcement_ids.reset();
    with_deleted.reset();
}

bool ReadReceiptFilter::has_conditions() const {
    return user_id.has_value() ||
           announcement_id.has_value() ||
           is_read.has_value() ||
           created_before.has_value() ||
           created_after.has_value() ||
           read_before.has_value() ||
           read_after.has_value() ||
           min_read_duration.has_value() ||
           max_read_duration.has_value() ||
           min_read_progress.has_value() ||
           max_read_progress.has_value() ||
           user_ids.has_value() ||
           announcement_ids.has_value() ||
           with_deleted.has_value();
}

std::string ReadReceiptFilter::to_string() const {
    std::ostringstream oss;
    oss << "ReadReceiptFilter{";
    
    if (user_id) oss << "user_id=" << *user_id << ",";
    if (announcement_id) oss << "announcement_id=" << *announcement_id << ",";
    if (is_read) oss << "is_read=" << std::boolalpha << *is_read << std::noboolalpha << ",";
    if (created_before) oss << "created_before='" << *created_before << "',";
    if (created_after) oss << "created_after='" << *created_after << "',";
    if (read_before) oss << "read_before='" << *read_before << "',";
    if (read_after) oss << "read_after='" << *read_after << "',";
    if (min_read_duration) oss << "min_read_duration=" << *min_read_duration << ",";
    if (max_read_duration) oss << "max_read_duration=" << *max_read_duration << ",";
    if (min_read_progress) oss << "min_read_progress=" << *min_read_progress << ",";
    if (max_read_progress) oss << "max_read_progress=" << *max_read_progress << ",";
    if (with_deleted) oss << "with_deleted=" << std::boolalpha << *with_deleted << std::noboolalpha << ",";
    
    auto str = oss.str();
    if (!str.empty() && str.back() == ',') {
        str.pop_back();
    }
    str += "}";
    
    return str;
}

// ReadReceiptStatistics 方法实现
double ReadReceiptStatistics::get_read_rate() const {
    if (total_records == 0) {
        return 0.0;
    }
    return static_cast<double>(read_records) / total_records * 100.0;
}

double ReadReceiptStatistics::get_completion_rate() const {
    if (read_records == 0) {
        return 0.0;
    }
    return average_read_progress;
}

void ReadReceiptStatistics::reset() {
    total_records = 0;
    read_records = 0;
    unread_records = 0;
    average_read_progress = 0.0;
    average_read_duration = 0.0;
    user_read_counts.clear();
    announcement_read_counts.clear();
    daily_read_counts.clear();
    hourly_read_counts.clear();
}

std::string ReadReceiptStatistics::to_string() const {
    std::ostringstream oss;
    oss << "ReadReceiptStatistics{";
    oss << "total_records=" << total_records << ",";
    oss << "read_records=" << read_records << ",";
    oss << "unread_records=" << unread_records << ",";
    oss << "read_rate=" << std::fixed << std::setprecision(2) << get_read_rate() << "%,";
    oss << "average_read_progress=" << std::fixed << std::setprecision(2) << average_read_progress << "%,";
    oss << "average_read_duration=" << std::fixed << std::setprecision(2) << average_read_duration << "s";
    oss << "}";
    
    return oss.str();
}

} // namespace domain
