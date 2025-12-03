#ifndef READ_RECEIPT_SERVICE_H
#define READ_RECEIPT_SERVICE_H

#include <memory>
#include <vector>
#include <optional>
#include <string>
#include <ctime>

#include "models/user.h"
#include "models/announcement.h"
#include "models/read_receipt.h"
#include "repositories/read_receipt_repository.h"
#include "repositories/announcement_repository.h"
#include "repositories/user_repository.h"

namespace services {

class ReadReceiptService {
public:
    ReadReceiptService(
        std::shared_ptr<repositories::ReadReceiptRepository> read_receipt_repository,
        std::shared_ptr<repositories::AnnouncementRepository> announcement_repository,
        std::shared_ptr<repositories::UserRepository> user_repository
    );
    
    ~ReadReceiptService();
    
    // 构造函数和析构函数的默认实现
    ReadReceiptService(const ReadReceiptService&) = default;
    ReadReceiptService& operator=(const ReadReceiptService&) = default;
    ReadReceiptService(ReadReceiptService&&) = default;
    ReadReceiptService& operator=(ReadReceiptService&&) = default;
    
    // 创建阅读记录（幂等操作）
    std::optional<models::ReadReceipt> create_read_receipt(int announcement_id, int user_id,
                                                          const std::optional<std::string>& client_ip = std::nullopt,
                                                          const std::optional<std::string>& user_agent = std::nullopt,
                                                          const std::optional<std::string>& extra_metadata = std::nullopt);
    
    // 按ID获取阅读记录
    std::optional<models::ReadReceipt> get_read_receipt_by_id(int receipt_id) const;
    
    // 按公告ID和用户ID获取阅读记录（用于幂等检查）
    std::optional<models::ReadReceipt> get_read_receipt_by_announcement_and_user(int announcement_id, int user_id) const;
    
    // 按公告ID获取所有阅读记录
    std::vector<models::ReadReceipt> get_read_receipts_by_announcement(int announcement_id) const;
    
    // 按用户ID获取所有阅读记录
    std::vector<models::ReadReceipt> get_read_receipts_by_user(int user_id) const;
    
    // 获取用户的未读公告列表
    std::vector<models::Announcement> get_unread_announcements(int user_id) const;
    
    // 获取用户的已读公告列表
    std::vector<models::Announcement> get_read_announcements(int user_id) const;
    
    // 获取用户的必须阅读且未读的公告列表
    std::vector<models::Announcement> get_mandatory_unread_announcements(int user_id) const;
    
    // 获取用户的必须阅读公告完成情况统计
    struct MandatoryStats {
        int total_mandatory;   // 总必须阅读公告数
        int read_mandatory;    // 已读必须阅读公告数
        double completion_rate; // 完成率
    };
    MandatoryStats get_mandatory_reading_stats(int user_id) const;
    
    // 获取公告的阅读统计
    repositories::AnnouncementStats get_announcement_read_stats(int announcement_id) const;
    
    // 获取用户的阅读统计概览
    repositories::UserReadingStats get_user_reading_stats(int user_id,
                                                         std::optional<std::time_t> start_time = std::nullopt,
                                                         std::optional<std::time_t> end_time = std::nullopt) const;
    
    // 批量标记公告为已读
    std::vector<models::ReadReceipt> batch_mark_as_read(int user_id,
                                                       const std::vector<int>& announcement_ids,
                                                       const std::optional<std::string>& client_ip = std::nullopt,
                                                       const std::optional<std::string>& user_agent = std::nullopt) const;
    
    // 检查用户是否已阅读指定公告
    bool has_read_announcement(int announcement_id, int user_id) const;
    
    // 检查用户是否已阅读所有必须阅读的公告
    bool has_read_all_mandatory_announcements(int user_id) const;
    
    // 获取用户在特定时间范围内的阅读进度
    struct ReadingProgress {
        int total_announcements;  // 时间范围内的总公告数
        int read_announcements;   // 已读公告数
        int unread_announcements; // 未读公告数
        double read_rate;         // 阅读率
        std::time_t date_range_start;
        std::time_t date_range_end;
    };
    ReadingProgress get_user_reading_progress(int user_id,
                                             std::time_t start_time,
                                             std::time_t end_time) const;
    
    // 获取阅读记录的统计信息
    struct ServiceStats {
        int total_receipts;        // 总阅读记录数
        int active_users;          // 有阅读记录的活跃用户数
        int popular_announcement_id; // 阅读人数最多的公告ID
        int popular_announcement_read_count; // 最受欢迎公告的阅读人数
        std::time_t last_receipt_time; // 最后一条阅读记录的时间
    };
    ServiceStats get_service_stats() const;
    
    // 清理过期的阅读记录（可选功能）
    int cleanup_expired_receipts(std::time_t cutoff_time) const;
    
private:
    std::shared_ptr<repositories::ReadReceiptRepository> read_receipt_repository_;
    std::shared_ptr<repositories::AnnouncementRepository> announcement_repository_;
    std::shared_ptr<repositories::UserRepository> user_repository_;
    
    // 验证阅读记录数据
    bool validate_read_receipt(int announcement_id, int user_id) const;
    
    // 构建阅读记录对象
    models::ReadReceipt build_read_receipt(int announcement_id, int user_id,
                                           const std::optional<std::string>& client_ip,
                                           const std::optional<std::string>& user_agent,
                                           const std::optional<std::string>& extra_metadata) const;
    
    // 获取所有有效的公告（状态正常且未过期）
    std::vector<models::Announcement> get_valid_announcements(int user_id) const;
};

} // namespace services

#endif // READ_RECEIPT_SERVICE_H
