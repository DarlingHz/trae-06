#pragma once

#include "models/announcement.h"
#include <memory>
#include <optional>
#include <vector>
#include <string>
#include <ctime>

namespace repositories {

struct AnnouncementFilter {
    std::optional<std::string> category;
    std::optional<bool> mandatory;
    std::optional<models::Announcement::Status> status;
    std::optional<std::time_t> start_time;
    std::optional<std::time_t> end_time;
};

class AnnouncementRepository {
public:
    AnnouncementRepository() = default;
    virtual ~AnnouncementRepository() = default;

    virtual std::optional<models::Announcement> find_by_id(int id) const;
    virtual std::vector<models::Announcement> find_by_publisher_id(int publisher_id) const;
    virtual std::vector<models::Announcement> find_with_filter(const AnnouncementFilter& filter, 
                                                               int page = 1, int page_size = 20,
                                                               bool order_by_publish_time_desc = true) const;
    virtual int count_with_filter(const AnnouncementFilter& filter) const;
    virtual int create(const models::Announcement& announcement);
    virtual bool update(const models::Announcement& announcement);
    virtual bool delete_by_id(int id);
    virtual std::vector<models::Announcement> find_unread_by_user_id(int user_id, int page = 1, int page_size = 20) const;
    virtual std::vector<models::Announcement> find_mandatory_by_user_id(int user_id, int page = 1, int page_size = 20) const;
};

std::unique_ptr<AnnouncementRepository> create_announcement_repository();

} // namespace repositories
