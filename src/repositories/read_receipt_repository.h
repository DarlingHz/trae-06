#pragma once

#include "models/read_receipt.h"
#include <memory>
#include <optional>
#include <vector>
#include <string>
#include <ctime>

namespace repositories {

struct ReadReceiptFilter {
    std::optional<int> announcement_id;
    std::optional<int> user_id;
    std::optional<std::time_t> start_time;
    std::optional<std::time_t> end_time;
};

struct AnnouncementStats {
    int total_users;
    int read_count;
    int unread_count;
};

struct UserReadingStats {
    int total_announcements;
    int read_count;
    int unread_count;
    int mandatory_total;
    int mandatory_read;
    double mandatory_completion_rate;
};

class ReadReceiptRepository {
public:
    ReadReceiptRepository() = default;
    virtual ~ReadReceiptRepository() = default;

    virtual std::optional<models::ReadReceipt> find_by_id(int id) const;
    virtual std::optional<models::ReadReceipt> find_by_announcement_and_user(int announcement_id, int user_id) const;
    virtual std::vector<models::ReadReceipt> find_with_filter(const ReadReceiptFilter& filter) const;
    virtual std::vector<models::ReadReceipt> find_read_users_by_announcement(int announcement_id) const;
    virtual int create(const models::ReadReceipt& receipt);
    virtual bool update(const models::ReadReceipt& receipt);
    virtual bool delete_by_id(int id);
    virtual AnnouncementStats get_announcement_stats(int announcement_id) const;
    virtual UserReadingStats get_user_reading_stats(int user_id, const std::optional<std::time_t>& start_time = std::nullopt,
                                                    const std::optional<std::time_t>& end_time = std::nullopt) const;
    virtual bool has_read(int user_id, int announcement_id) const;
    virtual std::vector<int> find_read_announcement_ids(int user_id) const;
};

std::unique_ptr<ReadReceiptRepository> create_read_receipt_repository();

} // namespace repositories
