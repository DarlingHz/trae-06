#pragma once

#include <string>
#include <vector>
#include <utility>

namespace models {

class UserStats {
private:
    int total_bookmarks_;
    int unread_count_;
    int reading_count_;
    int read_count_;
    int favorite_count_;

public:
    UserStats();
    UserStats(int total, int unread, int reading, int read, int favorite);

    int total_bookmarks() const { return total_bookmarks_; }
    int unread_count() const { return unread_count_; }
    int reading_count() const { return reading_count_; }
    int read_count() const { return read_count_; }
    int favorite_count() const { return favorite_count_; }
};

class DomainStats {
private:
    std::string domain_;
    int click_count_;

public:
    DomainStats(const std::string& domain, int click_count);

    const std::string& domain() const { return domain_; }
    int click_count() const { return click_count_; }
};

class DailyStats {
private:
    std::string date_;
    int count_;

public:
    DailyStats(const std::string& date, int count);

    const std::string& date() const { return date_; }
    int count() const { return count_; }
};

class TagStats {
private:
    std::string tag_;
    int count_;

public:
    TagStats(const std::string& tag, int count);

    const std::string& tag() const { return tag_; }
    int count() const { return count_; }
};

class FolderStats {
private:
    std::string folder_;
    int count_;

public:
    FolderStats(const std::string& folder, int count);

    const std::string& folder() const { return folder_; }
    int count() const { return count_; }
};

} // namespace models
