#include "services/read_receipt_service.h"
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <map>

namespace services {

using namespace models;
using namespace repositories;

ReadReceiptService::ReadReceiptService(
    std::shared_ptr<ReadReceiptRepository> read_receipt_repository,
    std::shared_ptr<AnnouncementRepository> announcement_repository,
    std::shared_ptr<UserRepository> user_repository
) : read_receipt_repository_(std::move(read_receipt_repository)),
    announcement_repository_(std::move(announcement_repository)),
    user_repository_(std::move(user_repository)) {
    
    if (!read_receipt_repository_) {
        throw std::invalid_argument("read_receipt_repository cannot be null");
    }
    if (!announcement_repository_) {
        throw std::invalid_argument("announcement_repository cannot be null");
    }
    if (!user_repository_) {
        throw std::invalid_argument("user_repository cannot be null");
    }
}

ReadReceiptService::~ReadReceiptService() {
}

std::optional<ReadReceipt> ReadReceiptService::create_read_receipt(int announcement_id, int user_id,
                                                                  const std::optional<std::string>& client_ip,
                                                                  const std::optional<std::string>& user_agent,
                                                                  const std::optional<std::string>& extra_metadata) {
    if (!validate_read_receipt(announcement_id, user_id)) {
        return std::nullopt;
    }
    
    // 检查是否已经存在阅读记录（幂等性）
    auto existing_receipt = get_read_receipt_by_announcement_and_user(announcement_id, user_id);
    if (existing_receipt) {
        return existing_receipt;
    }
    
    // 创建新的阅读记录
    ReadReceipt receipt = build_read_receipt(announcement_id, user_id, client_ip, user_agent, extra_metadata);
    
    int receipt_id = read_receipt_repository_->create(receipt);
    if (receipt_id <= 0) {
        return std::nullopt;
    }
    
    // 返回创建的阅读记录
    return read_receipt_repository_->find_by_id(receipt_id);
}

std::optional<ReadReceipt> ReadReceiptService::get_read_receipt_by_id(int receipt_id) const {
    if (receipt_id <= 0) {
        throw std::invalid_argument("receipt_id must be positive");
    }
    
    return read_receipt_repository_->find_by_id(receipt_id);
}

std::optional<ReadReceipt> ReadReceiptService::get_read_receipt_by_announcement_and_user(int announcement_id, int user_id) const {
    if (announcement_id <= 0 || user_id <= 0) {
        throw std::invalid_argument("announcement_id and user_id must be positive");
    }
    
    return read_receipt_repository_->find_by_announcement_and_user(announcement_id, user_id);
}

std::vector<ReadReceipt> ReadReceiptService::get_read_receipts_by_announcement(int announcement_id) const {
    if (announcement_id <= 0) {
        throw std::invalid_argument("announcement_id must be positive");
    }
    
    ReadReceiptFilter filter;
    filter.announcement_id = announcement_id;
    
    return read_receipt_repository_->find_with_filter(filter);
}

std::vector<ReadReceipt> ReadReceiptService::get_read_receipts_by_user(int user_id) const {
    if (user_id <= 0) {
        throw std::invalid_argument("user_id must be positive");
    }
    
    ReadReceiptFilter filter;
    filter.user_id = user_id;
    filter.page_size = std::numeric_limits<int>::max(); // 不限制数量
    
    return read_receipt_repository_->find_with_filter(filter);
}

std::vector<Announcement> ReadReceiptService::get_unread_announcements(int user_id) const {
    if (user_id <= 0) {
        throw std::invalid_argument("user_id must be positive");
    }
    
    // 获取所有有效的公告
    auto valid_announcements = get_valid_announcements(user_id);
    
    // 获取用户已读的公告ID
    auto read_receipts = get_read_receipts_by_user(user_id);
    std::vector<int> read_announcement_ids;
    read_announcement_ids.reserve(read_receipts.size());
    
    for (const auto& receipt : read_receipts) {
        read_announcement_ids.push_back(receipt.get_announcement_id());
    }
    
    // 过滤出未读公告
    std::vector<Announcement> unread_announcements;
    std::copy_if(valid_announcements.begin(), valid_announcements.end(),
                 std::back_inserter(unread_announcements),
                 [&read_announcement_ids](const Announcement& ann) {
                     return std::find(read_announcement_ids.begin(), read_announcement_ids.end(),
                                     ann.get_id()) == read_announcement_ids.end();
                 });
    
    // 按发布时间倒序排序
    std::sort(unread_announcements.begin(), unread_announcements.end(),
              [](const Announcement& a, const Announcement& b) {
                  return a.get_publish_time() > b.get_publish_time();
              });
    
    return unread_announcements;
}

std::vector<Announcement> ReadReceiptService::get_read_announcements(int user_id) const {
    if (user_id <= 0) {
        throw std::invalid_argument("user_id must be positive");
    }
    
    // 获取用户已读的公告ID
    auto read_receipts = get_read_receipts_by_user(user_id);
    
    // 获取对应的公告信息
    std::vector<Announcement> read_announcements;
    read_announcements.reserve(read_receipts.size());
    
    for (const auto& receipt : read_receipts) {
        auto announcement = announcement_repository_->find_by_id(receipt.get_announcement_id());
        if (announcement) {
            read_announcements.push_back(*announcement);
        }
    }
    
    // 按阅读时间倒序排序
    std::sort(read_announcements.begin(), read_announcements.end(),
              [&read_receipts](const Announcement& a, const Announcement& b) {
                  // 找到对应的阅读时间
                  auto find_read_time = [&read_receipts](int announcement_id) {
                      for (const auto& receipt : read_receipts) {
                          if (receipt.get_announcement_id() == announcement_id) {
                              return receipt.get_read_at();
                          }
                      }
                      return std::time(nullptr);
                  };
                  
                  return find_read_time(a.get_id()) > find_read_time(b.get_id());
              });
    
    return read_announcements;
}

std::vector<Announcement> ReadReceiptService::get_mandatory_unread_announcements(int user_id) const {
    if (user_id <= 0) {
        throw std::invalid_argument("user_id must be positive");
    }
    
    // 获取所有有效的必须阅读公告
    AnnouncementFilter ann_filter;
    ann_filter.mandatory = true;
    ann_filter.status = Announcement::Status::NORMAL;
    
    auto mandatory_announcements = announcement_repository_->find_with_filter(ann_filter);
    
    // 过滤出未过期的公告
    std::time_t now = std::time(nullptr);
    std::vector<Announcement> valid_mandatory;
    std::copy_if(mandatory_announcements.begin(), mandatory_announcements.end(),
                 std::back_inserter(valid_mandatory),
                 [now](const Announcement& ann) {
                     return !ann.get_expire_time() || *ann.get_expire_time() >= now;
                 });
    
    // 获取用户已读的公告ID
    auto read_receipts = get_read_receipts_by_user(user_id);
    std::vector<int> read_announcement_ids;
    read_announcement_ids.reserve(read_receipts.size());
    
    for (const auto& receipt : read_receipts) {
        read_announcement_ids.push_back(receipt.get_announcement_id());
    }
    
    // 过滤出未读的必须阅读公告
    std::vector<Announcement> unread_mandatory;
    std::copy_if(valid_mandatory.begin(), valid_mandatory.end(),
                 std::back_inserter(unread_mandatory),
                 [&read_announcement_ids](const Announcement& ann) {
                     return std::find(read_announcement_ids.begin(), read_announcement_ids.end(),
                                     ann.get_id()) == read_announcement_ids.end();
                 });
    
    // 按发布时间倒序排序
    std::sort(unread_mandatory.begin(), unread_mandatory.end(),
              [](const Announcement& a, const Announcement& b) {
                  return a.get_publish_time() > b.get_publish_time();
              });
    
    return unread_mandatory;
}

ReadReceiptService::MandatoryStats ReadReceiptService::get_mandatory_reading_stats(int user_id) const {
    if (user_id <= 0) {
        throw std::invalid_argument("user_id must be positive");
    }
    
    // 获取所有有效的必须阅读公告
    AnnouncementFilter ann_filter;
    ann_filter.mandatory = true;
    ann_filter.status = Announcement::Status::NORMAL;
    
    auto mandatory_announcements = announcement_repository_->find_with_filter(ann_filter);
    
    // 过滤出未过期的公告
    std::time_t now = std::time(nullptr);
    std::vector<Announcement> valid_mandatory;
    std::copy_if(mandatory_announcements.begin(), mandatory_announcements.end(),
                 std::back_inserter(valid_mandatory),
                 [now](const Announcement& ann) {
                     return !ann.get_expire_time() || *ann.get_expire_time() >= now;
                 });
    
    int total_mandatory = valid_mandatory.size();
    
    // 计算已读的必须阅读公告数
    auto read_receipts = get_read_receipts_by_user(user_id);
    std::vector<int> read_announcement_ids;
    read_announcement_ids.reserve(read_receipts.size());
    
    for (const auto& receipt : read_receipts) {
        read_announcement_ids.push_back(receipt.get_announcement_id());
    }
    
    int read_mandatory = 0;
    for (const auto& ann : valid_mandatory) {
        if (std::find(read_announcement_ids.begin(), read_announcement_ids.end(),
                     ann.get_id()) != read_announcement_ids.end()) {
            ++read_mandatory;
        }
    }
    
    MandatoryStats stats;
    stats.total_mandatory = total_mandatory;
    stats.read_mandatory = read_mandatory;
    stats.completion_rate = total_mandatory > 0 ? static_cast<double>(read_mandatory) / total_mandatory : 1.0;
    
    return stats;
}

AnnouncementStats ReadReceiptService::get_announcement_read_stats(int announcement_id) const {
    if (announcement_id <= 0) {
        throw std::invalid_argument("announcement_id must be positive");
    }
    
    return read_receipt_repository_->get_announcement_stats(announcement_id);
}

UserReadingStats ReadReceiptService::get_user_reading_stats(int user_id,
                                                           std::optional<std::time_t> start_time,
                                                           std::optional<std::time_t> end_time) const {
    if (user_id <= 0) {
        throw std::invalid_argument("user_id must be positive");
    }
    
    return read_receipt_repository_->get_user_reading_stats(user_id, start_time, end_time);
}

std::vector<ReadReceipt> ReadReceiptService::batch_mark_as_read(int user_id,
                                                               const std::vector<int>& announcement_ids,
                                                               const std::optional<std::string>& client_ip,
                                                               const std::optional<std::string>& user_agent) const {
    if (user_id <= 0) {
        throw std::invalid_argument("user_id must be positive");
    }
    
    std::vector<ReadReceipt> results;
    
    for (int announcement_id : announcement_ids) {
        if (announcement_id <= 0) {
            continue;
        }
        
        // 检查公告是否存在且有效
        auto announcement = announcement_repository_->find_by_id(announcement_id);
        if (!announcement) {
            continue;
        }
        
        // 检查用户是否已阅读
        auto existing_receipt = get_read_receipt_by_announcement_and_user(announcement_id, user_id);
        if (existing_receipt) {
            results.push_back(*existing_receipt);
            continue;
        }
        
        // 创建阅读记录
        ReadReceipt receipt = build_read_receipt(announcement_id, user_id, client_ip, user_agent, std::nullopt);
        
        int receipt_id = read_receipt_repository_->create(receipt);
        if (receipt_id > 0) {
            auto new_receipt = read_receipt_repository_->find_by_id(receipt_id);
            if (new_receipt) {
                results.push_back(*new_receipt);
            }
        }
    }
    
    return results;
}

bool ReadReceiptService::has_read_announcement(int announcement_id, int user_id) const {
    if (announcement_id <= 0 || user_id <= 0) {
        return false;
    }
    
    auto receipt = get_read_receipt_by_announcement_and_user(announcement_id, user_id);
    return receipt.has_value();
}

bool ReadReceiptService::has_read_all_mandatory_announcements(int user_id) const {
    if (user_id <= 0) {
        return false;
    }
    
    auto stats = get_mandatory_reading_stats(user_id);
    return stats.completion_rate == 1.0 && stats.total_mandatory > 0;
}

ReadReceiptService::ReadingProgress ReadReceiptService::get_user_reading_progress(int user_id,
                                                                                  std::time_t start_time,
                                                                                  std::time_t end_time) const {
    if (user_id <= 0) {
        throw std::invalid_argument("user_id must be positive");
    }
    
    if (start_time >= end_time) {
        throw std::invalid_argument("start_time must be before end_time");
    }
    
    // 获取时间范围内的所有公告
    AnnouncementFilter ann_filter;
    ann_filter.start_time = start_time;
    ann_filter.end_time = end_time;
    ann_filter.status = Announcement::Status::NORMAL;
    ann_filter.page_size = std::numeric_limits<int>::max();
    
    auto announcements = announcement_repository_->find_with_filter(ann_filter);
    
    int total_announcements = announcements.size();
    
    // 获取用户在该时间范围内的阅读记录
    ReadReceiptFilter receipt_filter;
    receipt_filter.user_id = user_id;
    receipt_filter.start_time = start_time;
    receipt_filter.end_time = end_time;
    receipt_filter.page_size = std::numeric_limits<int>::max();
    
    auto read_receipts = read_receipt_repository_->find_with_filter(receipt_filter);
    
    int read_announcements = read_receipts.size();
    int unread_announcements = total_announcements - read_announcements;
    double read_rate = total_announcements > 0 ? static_cast<double>(read_announcements) / total_announcements : 0.0;
    
    ReadingProgress progress;
    progress.total_announcements = total_announcements;
    progress.read_announcements = read_announcements;
    progress.unread_announcements = unread_announcements;
    progress.read_rate = read_rate;
    progress.date_range_start = start_time;
    progress.date_range_end = end_time;
    
    return progress;
}

ReadReceiptService::ServiceStats ReadReceiptService::get_service_stats() const {
    ServiceStats stats;
    stats.total_receipts = 0;
    stats.active_users = 0;
    stats.popular_announcement_id = 0;
    stats.popular_announcement_read_count = 0;
    stats.last_receipt_time = 0;
    
    // 获取所有阅读记录以统计
    ReadReceiptFilter filter;
    filter.page_size = std::numeric_limits<int>::max();
    
    auto receipts = read_receipt_repository_->find_with_filter(filter);
    stats.total_receipts = receipts.size();
    
    if (receipts.empty()) {
        return stats;
    }
    
    // 统计活跃用户数
    std::set<int> unique_users;
    for (const auto& receipt : receipts) {
        unique_users.insert(receipt.get_user_id());
    }
    stats.active_users = unique_users.size();
    
    // 统计最受欢迎的公告
    std::map<int, int> announcement_read_counts;
    for (const auto& receipt : receipts) {
        announcement_read_counts[receipt.get_announcement_id()]++;
    }
    
    if (!announcement_read_counts.empty()) {
        auto popular = std::max_element(announcement_read_counts.begin(), announcement_read_counts.end(),
            [](const auto& a, const auto& b) {
                return a.second < b.second;
            });
        
        stats.popular_announcement_id = popular->first;
        stats.popular_announcement_read_count = popular->second;
    }
    
    // 获取最后一条阅读记录时间
    auto last_receipt = std::max_element(receipts.begin(), receipts.end(),
        [](const auto& a, const auto& b) {
            return a.get_read_at() < b.get_read_at();
        });
    
    stats.last_receipt_time = last_receipt->get_read_at();
    
    return stats;
}

int ReadReceiptService::cleanup_expired_receipts(std::time_t cutoff_time) const {
    ReadReceiptFilter filter;
    filter.end_time = cutoff_time;
    filter.page_size = std::numeric_limits<int>::max();
    
    auto receipts_to_delete = read_receipt_repository_->find_with_filter(filter);
    
    int deleted_count = 0;
    for (const auto& receipt : receipts_to_delete) {
        if (read_receipt_repository_->delete_by_id(receipt.get_id())) {
            deleted_count++;
        }
    }
    
    return deleted_count;
}

bool ReadReceiptService::validate_read_receipt(int announcement_id, int user_id) const {
    if (announcement_id <= 0 || user_id <= 0) {
        return false;
    }
    
    // 检查公告是否存在且有效
    auto announcement = announcement_repository_->find_by_id(announcement_id);
    if (!announcement) {
        return false;
    }
    
    // 检查公告状态是否正常
    if (announcement->get_status() != Announcement::Status::NORMAL) {
        return false;
    }
    
    // 检查公告是否过期
    if (announcement->get_expire_time() && *announcement->get_expire_time() < std::time(nullptr)) {
        return false;
    }
    
    // 检查用户是否存在且活跃
    auto user = user_repository_->find_by_id(user_id);
    if (!user || user->get_status() != User::Status::ACTIVE) {
        return false;
    }
    
    return true;
}

ReadReceipt ReadReceiptService::build_read_receipt(int announcement_id, int user_id,
                                                   const std::optional<std::string>& client_ip,
                                                   const std::optional<std::string>& user_agent,
                                                   const std::optional<std::string>& extra_metadata) const {
    std::time_t now = std::time(nullptr);
    
    ReadReceipt receipt(0, announcement_id, user_id, now,
                       client_ip, user_agent, extra_metadata.value_or("{}"));
    
    return receipt;
}

std::vector<Announcement> ReadReceiptService::get_valid_announcements(int user_id) const {
    (void)user_id; // 暂时不需要用户信息，后续可能需要基于权限过滤
    
    AnnouncementFilter filter;
    filter.status = Announcement::Status::NORMAL;
    filter.page_size = std::numeric_limits<int>::max();
    
    auto announcements = announcement_repository_->find_with_filter(filter);
    
    // 过滤出未过期的公告
    std::time_t now = std::time(nullptr);
    std::vector<Announcement> valid_announcements;
    std::copy_if(announcements.begin(), announcements.end(),
                 std::back_inserter(valid_announcements),
                 [now](const Announcement& ann) {
                     return !ann.get_expire_time() || *ann.get_expire_time() >= now;
                 });
    
    return valid_announcements;
}

} // namespace services
