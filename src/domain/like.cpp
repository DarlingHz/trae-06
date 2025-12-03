#include "like.h"
#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace std;
using namespace chrono;

// 构造函数
Like::Like()
    : id_(0), user_id_(0), announcement_id_(0), status_(LikeStatus::ACTIVE),
      created_at_(system_clock::now()), updated_at_(system_clock::now()) {}

Like::Like(int64_t user_id, int64_t announcement_id)
    : id_(0), user_id_(user_id), announcement_id_(announcement_id), 
      status_(LikeStatus::ACTIVE), created_at_(system_clock::now()), 
      updated_at_(system_clock::now()) {}

// Getters
int64_t Like::get_id() const { return id_; }
int64_t Like::get_user_id() const { return user_id_; }
int64_t Like::get_announcement_id() const { return announcement_id_; }
LikeStatus Like::get_status() const { return status_; }
system_clock::time_point Like::get_created_at() const { return created_at_; }
system_clock::time_point Like::get_updated_at() const { return updated_at_; }

string Like::get_created_at_str() const {
    auto time_point = system_clock::to_time_t(created_at_);
    stringstream ss;
    ss << put_time(localtime(&time_point), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

string Like::get_updated_at_str() const {
    auto time_point = system_clock::to_time_t(updated_at_);
    stringstream ss;
    ss << put_time(localtime(&time_point), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Setters
void Like::set_id(int64_t id) { id_ = id; }
void Like::set_user_id(int64_t user_id) { user_id_ = user_id; }
void Like::set_announcement_id(int64_t announcement_id) { announcement_id_ = announcement_id; }
void Like::set_status(LikeStatus status) { 
    status_ = status; 
    updated_at_ = system_clock::now();
}
void Like::set_created_at(const system_clock::time_point& created_at) { created_at_ = created_at; }
void Like::set_updated_at(const system_clock::time_point& updated_at) { updated_at_ = updated_at; }

// 状态管理方法
void Like::activate() {
    status_ = LikeStatus::ACTIVE;
    updated_at_ = system_clock::now();
}

void Like::cancel() {
    status_ = LikeStatus::CANCELLED;
    updated_at_ = system_clock::now();
}

bool Like::is_active() const {
    return status_ == LikeStatus::ACTIVE;
}

bool Like::is_cancelled() const {
    return status_ == LikeStatus::CANCELLED;
}

// 数据验证
bool Like::is_valid() const {
    return validate().empty();
}

string Like::validate() const {
    if (user_id_ <= 0) {
        return "User ID must be positive";
    }
    if (announcement_id_ <= 0) {
        return "Announcement ID must be positive";
    }
    return "";
}

// 辅助功能
string Like::to_string() const {
    stringstream ss;
    ss << "Like{";
    ss << "id=" << id_ << ",";
    ss << "user_id=" << user_id_ << ",";
    ss << "announcement_id=" << announcement_id_ << ",";
    ss << "status=" << Like::to_string(status_) << ",";
    ss << "created_at=" << get_created_at_str() << ",";
    ss << "updated_at=" << get_updated_at_str() << "}";
    return ss.str();
}

// 枚举转换
LikeStatus Like::from_string(const string& str) {
    if (str == "ACTIVE" || str == "active") {
        return LikeStatus::ACTIVE;
    } else if (str == "CANCELLED" || str == "cancelled") {
        return LikeStatus::CANCELLED;
    } else {
        throw invalid_argument("Invalid LikeStatus string: " + str);
    }
}

string Like::to_string(LikeStatus status) {
    switch (status) {
        case LikeStatus::ACTIVE: return "ACTIVE";
        case LikeStatus::CANCELLED: return "CANCELLED";
        default: return "UNKNOWN";
    }
}

// LikeStatistics 结构体方法
LikeStatistics::LikeStatistics()
    : announcement_id_(0), like_count_(0), unique_users_(0), user_liked_(false) {}

LikeStatistics::LikeStatistics(int64_t announcement_id, int64_t like_count, 
                               int64_t unique_users, bool user_liked)
    : announcement_id_(announcement_id), like_count_(like_count), 
      unique_users_(unique_users), user_liked_(user_liked) {}

string LikeStatistics::to_string() const {
    stringstream ss;
    ss << "LikeStatistics{";
    ss << "announcement_id=" << announcement_id_ << ",";
    ss << "like_count=" << like_count_ << ",";
    ss << "unique_users=" << unique_users_ << ",";
    ss << "user_liked=" << (user_liked_ ? "true" : "false") << "}";
    return ss.str();
}

// LikeFilter 结构体方法
LikeFilter::LikeFilter() {}

string LikeFilter::to_query_condition() const {
    vector<string> conditions;
    
    if (user_id_.has_value()) {
        conditions.push_back("user_id = " + std::to_string(user_id_.value()));
    }
    
    if (announcement_id_.has_value()) {
        conditions.push_back("announcement_id = " + std::to_string(announcement_id_.value()));
    }
    
    if (status_.has_value()) {
        conditions.push_back("status = " + std::to_string(static_cast<int>(status_.value())));
    }
    
    if (conditions.empty()) {
        return "";
    }
    
    string result = "WHERE ";
    for (size_t i = 0; i < conditions.size(); ++i) {
        if (i > 0) {
            result += " AND ";
        }
        result += conditions[i];
    }
    
    return result;
}
