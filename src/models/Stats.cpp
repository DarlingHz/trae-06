#include "models/Stats.h"

namespace models {

UserStats::UserStats() 
    : total_bookmarks_(0), unread_count_(0), reading_count_(0), read_count_(0), favorite_count_(0) {}

UserStats::UserStats(int total, int unread, int reading, int read, int favorite)
    : total_bookmarks_(total), unread_count_(unread), reading_count_(reading), 
      read_count_(read), favorite_count_(favorite) {}

DomainStats::DomainStats(const std::string& domain, int click_count)
    : domain_(domain), click_count_(click_count) {}

DailyStats::DailyStats(const std::string& date, int count)
    : date_(date), count_(count) {}

TagStats::TagStats(const std::string& tag, int count)
    : tag_(tag), count_(count) {}

FolderStats::FolderStats(const std::string& folder, int count)
    : folder_(folder), count_(count) {}

} // namespace models
