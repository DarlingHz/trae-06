#pragma once

#include <memory>
#include <string>
#include <optional>
#include <vector>

#include "models/announcement.h"
#include "repositories/announcement_repository.h"
#include "cache/cache_manager.h"

namespace services {

class AnnouncementService {
public:
    AnnouncementService(std::shared_ptr<repositories::AnnouncementRepository> announcement_repository);
    ~AnnouncementService();
    
    AnnouncementService(const AnnouncementService&) = delete;
    AnnouncementService& operator=(const AnnouncementService&) = delete;
    
    AnnouncementService(AnnouncementService&&) = default;
    AnnouncementService& operator=(AnnouncementService&&) = default;
    
    // 创建公告（仅管理员）
    std::optional<models::Announcement> create_announcement(const std::string& title, 
                                                           const std::string& content,
                                                           const std::string& category,
                                                           bool mandatory,
                                                           int publisher_id,
                                                           std::optional<std::time_t> expire_time = std::nullopt);
    
    // 按ID查找公告
    std::optional<models::Announcement> get_announcement_by_id(int announcement_id);
    
    // 按发布者ID查找公告
    std::vector<models::Announcement> get_announcements_by_publisher(int publisher_id);
    
    // 带筛选条件查找公告
    std::vector<models::Announcement> get_announcements_with_filter(const repositories::AnnouncementFilter& filter);
    
    // 获取所有公告
    std::vector<models::Announcement> get_all_announcements();
    
    // 更新公告（仅管理员）
    bool update_announcement(int announcement_id, 
                            const std::optional<std::string>& title = std::nullopt,
                            const std::optional<std::string>& content = std::nullopt,
                            const std::optional<std::string>& category = std::nullopt,
                            const std::optional<bool>& mandatory = std::nullopt,
                            const std::optional<std::time_t>& expire_time = std::nullopt,
                            const std::optional<models::Announcement::Status>& status = std::nullopt);
    
    // 删除公告（软删除，仅管理员）
    bool delete_announcement(int announcement_id);
    
    // 检查公告是否存在且有效
    bool is_announcement_valid(int announcement_id) const;
    
    // 检查公告是否必须阅读
    bool is_announcement_mandatory(int announcement_id) const;
    
    // 获取公告类别列表
    std::vector<std::string> get_available_categories() const;
    
    // 验证公告数据
    static bool validate_announcement(const std::string& title, const std::string& content,
                                      const std::string& category, int publisher_id);
    
private:
    std::shared_ptr<repositories::AnnouncementRepository> announcement_repository_;
    
    // 生成公告列表的缓存键
    std::string generate_cache_key(const repositories::AnnouncementFilter& filter) const;
    
    // 清理相关缓存
    void clear_related_cache(int announcement_id);
};

} // namespace services
