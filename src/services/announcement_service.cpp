#include "services/announcement_service.h"
#include <cpprest/json.h>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <iterator>

namespace services {

using namespace web;

AnnouncementService::AnnouncementService(std::shared_ptr<repositories::AnnouncementRepository> announcement_repository)
    : announcement_repository_(std::move(announcement_repository)) {
    if (!announcement_repository_) {
        throw std::invalid_argument("announcement_repository cannot be null");
    }
}

AnnouncementService::~AnnouncementService() {
}

std::optional<models::Announcement> AnnouncementService::create_announcement(const std::string& title, 
                                                                             const std::string& content,
                                                                             const std::string& category,
                                                                             bool mandatory,
                                                                             int publisher_id,
                                                                             std::optional<std::time_t> expire_time) {
    if (!validate_announcement(title, content, category, publisher_id)) {
        return std::nullopt;
    }
    
    std::time_t now = std::time(nullptr);
    
    models::Announcement announcement(0, title, content, category, mandatory, 
                                     publisher_id, now, expire_time, now, now,
                                     models::Announcement::Status::NORMAL);
    
    int announcement_id = announcement_repository_->create(announcement);
    if (announcement_id <= 0) {
        return std::nullopt;
    }
    
    // 清理缓存
    cache::CacheManager::instance().clear_announcements();
    
    return announcement_repository_->find_by_id(announcement_id);
}

std::optional<models::Announcement> AnnouncementService::get_announcement_by_id(int announcement_id) {
    if (announcement_id <= 0) {
        throw std::invalid_argument("announcement_id must be positive");
    }
    
    // 尝试从缓存获取
    auto cache = cache::CacheManager::instance();
    if (cache.is_enabled()) {
        auto json_opt = cache.get_announcement(announcement_id);
        if (json_opt) {
            try {
                json::value json_val = json::value::parse(*json_opt);
                
                int id = json_val[U("id")].as_integer();
                std::string title = utility::conversions::to_utf8string(json_val[U("title")].as_string());
                std::string content = utility::conversions::to_utf8string(json_val[U("content")].as_string());
                std::string category = utility::conversions::to_utf8string(json_val[U("category")].as_string());
                bool mandatory = json_val[U("mandatory")].as_bool();
                int publisher_id = json_val[U("publisher_id")].as_integer();
                std::time_t publish_time = json_val[U("publish_time")].as_integer();
                
                std::optional<std::time_t> expire_time;
                if (json_val.has_field(U("expire_time")) && !json_val[U("expire_time")].is_null()) {
                    expire_time = json_val[U("expire_time")].as_integer();
                }
                
                std::time_t created_at = json_val[U("created_at")].as_integer();
                std::time_t updated_at = json_val[U("updated_at")].as_integer();
                
                std::string status_str = utility::conversions::to_utf8string(json_val[U("status")].as_string());
                models::Announcement::Status status = models::Announcement::string_to_status(status_str);
                
                models::Announcement announcement(id, title, content, category, mandatory, 
                                                 publisher_id, publish_time, expire_time, 
                                                 created_at, updated_at, status);
                
                return announcement;
            } catch (const std::exception&) {
                // 缓存解析失败，从数据库获取
            }
        }
    }
    
    // 从数据库获取
    auto announcement = announcement_repository_->find_by_id(announcement_id);
    
    // 缓存结果
    if (announcement && cache.is_enabled()) {
        json::value json_val;
        json_val[U("id")] = json::value::number(announcement->get_id());
        json_val[U("title")] = json::value::string(utility::conversions::to_string_t(announcement->get_title()));
        json_val[U("content")] = json::value::string(utility::conversions::to_string_t(announcement->get_content()));
        json_val[U("category")] = json::value::string(utility::conversions::to_string_t(announcement->get_category()));
        json_val[U("mandatory")] = json::value::boolean(announcement->is_mandatory());
        json_val[U("publisher_id")] = json::value::number(announcement->get_publisher_id());
        json_val[U("publish_time")] = json::value::number(static_cast<int64_t>(announcement->get_publish_time()));
        
        if (announcement->get_expire_time()) {
            json_val[U("expire_time")] = json::value::number(static_cast<int64_t>(*announcement->get_expire_time()));
        } else {
            json_val[U("expire_time")] = json::value::null();
        }
        
        json_val[U("created_at")] = json::value::number(static_cast<int64_t>(announcement->get_created_at()));
        json_val[U("updated_at")] = json::value::number(static_cast<int64_t>(announcement->get_updated_at()));
        json_val[U("status")] = json::value::string(utility::conversions::to_string_t(models::Announcement::status_to_string(announcement->get_status())));
        
        cache.put_announcement(announcement_id, json_val.serialize());
    }
    
    return announcement;
}

std::vector<models::Announcement> AnnouncementService::get_announcements_by_publisher(int publisher_id) {
    if (publisher_id <= 0) {
        throw std::invalid_argument("publisher_id must be positive");
    }
    
    return announcement_repository_->find_by_publisher_id(publisher_id);
}

std::vector<models::Announcement> AnnouncementService::get_announcements_with_filter(const repositories::AnnouncementFilter& filter) {
    // 尝试从缓存获取
    auto cache = cache::CacheManager::instance();
    std::string cache_key = generate_cache_key(filter);
    
    if (cache.is_enabled()) {
        auto json_opt = cache.get_announcement_list(cache_key);
        if (json_opt) {
            try {
                json::value json_val = json::value::parse(*json_opt);
                auto json_array = json_val.as_array();
                
                std::vector<models::Announcement> announcements;
                announcements.reserve(json_array.size());
                
                for (const auto& item : json_array) {
                    int id = item[U("id")].as_integer();
                    std::string title = utility::conversions::to_utf8string(item[U("title")].as_string());
                    std::string content = utility::conversions::to_utf8string(item[U("content")].as_string());
                    std::string category = utility::conversions::to_utf8string(item[U("category")].as_string());
                    bool mandatory = item[U("mandatory")].as_bool();
                    int publisher_id = item[U("publisher_id")].as_integer();
                    std::time_t publish_time = item[U("publish_time")].as_integer();
                    
                    std::optional<std::time_t> expire_time;
                    if (item.has_field(U("expire_time")) && !item[U("expire_time")].is_null()) {
                        expire_time = item[U("expire_time")].as_integer();
                    }
                    
                    std::time_t created_at = item[U("created_at")].as_integer();
                    std::time_t updated_at = item[U("updated_at")].as_integer();
                    
                    std::string status_str = utility::conversions::to_utf8string(item[U("status")].as_string());
                    models::Announcement::Status status = models::Announcement::string_to_status(status_str);
                    
                    announcements.emplace_back(id, title, content, category, mandatory, 
                                             publisher_id, publish_time, expire_time, 
                                             created_at, updated_at, status);
                }
                
                return announcements;
            } catch (const std::exception&) {
                // 缓存解析失败，从数据库获取
            }
        }
    }
    
    // 从数据库获取
    auto announcements = announcement_repository_->find_with_filter(filter);
    
    // 缓存结果
    if (cache.is_enabled()) {
        json::value json_array;
        json_array[U("announcements")] = json::value::array();
        
        for (size_t i = 0; i < announcements.size(); ++i) {
            const auto& ann = announcements[i];
            json::value json_val;
            json_val[U("id")] = json::value::number(ann.get_id());
            json_val[U("title")] = json::value::string(utility::conversions::to_string_t(ann.get_title()));
            json_val[U("content")] = json::value::string(utility::conversions::to_string_t(ann.get_content()));
            json_val[U("category")] = json::value::string(utility::conversions::to_string_t(ann.get_category()));
            json_val[U("mandatory")] = json::value::boolean(ann.is_mandatory());
            json_val[U("publisher_id")] = json::value::number(ann.get_publisher_id());
            json_val[U("publish_time")] = json::value::number(static_cast<int64_t>(ann.get_publish_time()));
            
            if (ann.get_expire_time()) {
                json_val[U("expire_time")] = json::value::number(static_cast<int64_t>(*ann.get_expire_time()));
            } else {
                json_val[U("expire_time")] = json::value::null();
            }
            
            json_val[U("created_at")] = json::value::number(static_cast<int64_t>(ann.get_created_at()));
            json_val[U("updated_at")] = json::value::number(static_cast<int64_t>(ann.get_updated_at()));
            json_val[U("status")] = json::value::string(utility::conversions::to_string_t(models::Announcement::status_to_string(ann.get_status())));
            
            json_array[U("announcements")].as_array()[i] = json_val;
        }
        
        cache.put_announcement_list(cache_key, json_array.serialize());
    }
    
    return announcements;
}

std::vector<models::Announcement> AnnouncementService::get_all_announcements() {
    repositories::AnnouncementFilter filter;
    filter.page_size = std::numeric_limits<int>::max(); // 不限制数量
    return get_announcements_with_filter(filter);
}

bool AnnouncementService::update_announcement(int announcement_id, 
                                            const std::optional<std::string>& title,
                                            const std::optional<std::string>& content,
                                            const std::optional<std::string>& category,
                                            const std::optional<bool>& mandatory,
                                            const std::optional<std::time_t>& expire_time,
                                            const std::optional<models::Announcement::Status>& status) {
    if (announcement_id <= 0) {
        throw std::invalid_argument("announcement_id must be positive");
    }
    
    auto existing_announcement = announcement_repository_->find_by_id(announcement_id);
    if (!existing_announcement) {
        return false;
    }
    
    // 验证数据（如果提供了新值）
    if (title && content && category) {
        if (!validate_announcement(*title, *content, *category, existing_announcement->get_publisher_id())) {
            return false;
        }
    }
    
    models::Announcement updated_announcement = *existing_announcement;
    
    if (title) updated_announcement.set_title(*title);
    if (content) updated_announcement.set_content(*content);
    if (category) updated_announcement.set_category(*category);
    if (mandatory) updated_announcement.set_mandatory(*mandatory);
    if (expire_time) updated_announcement.set_expire_time(*expire_time);
    if (status) updated_announcement.set_status(*status);
    
    updated_announcement.set_updated_at(std::time(nullptr));
    
    bool result = announcement_repository_->update(updated_announcement);
    
    // 如果更新成功，清理相关缓存
    if (result) {
        clear_related_cache(announcement_id);
    }
    
    return result;
}

bool AnnouncementService::delete_announcement(int announcement_id) {
    if (announcement_id <= 0) {
        throw std::invalid_argument("announcement_id must be positive");
    }
    
    bool result = announcement_repository_->delete_by_id(announcement_id);
    
    // 如果删除成功，清理相关缓存
    if (result) {
        clear_related_cache(announcement_id);
    }
    
    return result;
}

bool AnnouncementService::is_announcement_valid(int announcement_id) const {
    if (announcement_id <= 0) {
        return false;
    }
    
    auto announcement = announcement_repository_->find_by_id(announcement_id);
    if (!announcement) {
        return false;
    }
    
    // 检查状态是否为正常
    if (announcement->get_status() != models::Announcement::Status::NORMAL) {
        return false;
    }
    
    // 检查是否过期
    if (announcement->get_expire_time() && *announcement->get_expire_time() < std::time(nullptr)) {
        return false;
    }
    
    return true;
}

bool AnnouncementService::is_announcement_mandatory(int announcement_id) const {
    if (announcement_id <= 0) {
        return false;
    }
    
    auto announcement = announcement_repository_->find_by_id(announcement_id);
    if (!announcement) {
        return false;
    }
    
    return announcement->is_mandatory();
}

std::vector<std::string> AnnouncementService::get_available_categories() const {
    static const std::vector<std::string> categories = {
        "制度", "通知", "公告", "提醒", "紧急", "其他"
    };
    return categories;
}

bool AnnouncementService::validate_announcement(const std::string& title, const std::string& content,
                                                const std::string& category, int publisher_id) {
    if (title.empty() || title.size() > 255) {
        throw std::invalid_argument("title must be between 1 and 255 characters");
    }
    
    if (content.empty()) {
        throw std::invalid_argument("content cannot be empty");
    }
    
    if (category.empty() || category.size() > 50) {
        throw std::invalid_argument("category must be between 1 and 50 characters");
    }
    
    if (publisher_id <= 0) {
        throw std::invalid_argument("publisher_id must be positive");
    }
    
    return true;
}

std::string AnnouncementService::generate_cache_key(const repositories::AnnouncementFilter& filter) const {
    std::ostringstream oss;
    oss << "announcements:";
    
    if (filter.category) oss << "category:" << *filter.category << ";";
    if (filter.status) oss << "status:" << *filter.status << ";";
    if (filter.mandatory) oss << "mandatory:" << std::boolalpha << *filter.mandatory << ";";
    if (filter.publisher_id) oss << "publisher:" << *filter.publisher_id << ";";
    if (filter.start_time) oss << "start:" << *filter.start_time << ";";
    if (filter.end_time) oss << "end:" << *filter.end_time << ";";
    if (filter.page) oss << "page:" << *filter.page << ";";
    if (filter.page_size) oss << "size:" << *filter.page_size << ";";
    
    return oss.str();
}

void AnnouncementService::clear_related_cache(int announcement_id) {
    auto& cache_manager = cache::CacheManager::instance();
    
    // 清理该公告的详情缓存
    cache_manager.remove_announcement(announcement_id);
    
    // 清理所有公告列表缓存（因为列表可能包含该公告）
    // 注意：这比较粗暴，实际应用中可以使用更精细的缓存失效策略
    cache::CacheStats stats = cache_manager.get_stats();
    if (stats.announcement_list_cache_size > 0) {
        // 重新生成缓存会自动清除旧数据，或者我们可以实现更精确的失效
        // 这里我们选择不清除所有列表缓存，而是让它们自然过期
    }
}

} // namespace services
