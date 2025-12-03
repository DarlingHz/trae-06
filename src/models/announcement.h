#pragma once

#include <string>
#include <ctime>
#include <optional>

namespace models {

class Announcement {
public:
    enum class Status {
        NORMAL,
        WITHDRAWN,
        DELETED
    };

    Announcement() = default;
    Announcement(int id, const std::string& title, const std::string& content, 
                 const std::string& category, bool mandatory, int publisher_id,
                 std::time_t publish_time, const std::optional<std::time_t>& expire_time,
                 std::time_t created_at, std::time_t updated_at, Status status);

    int get_id() const { return id_; }
    void set_id(int id) { id_ = id; }

    const std::string& get_title() const { return title_; }
    void set_title(const std::string& title) { title_ = title; }

    const std::string& get_content() const { return content_; }
    void set_content(const std::string& content) { content_ = content; }

    const std::string& get_category() const { return category_; }
    void set_category(const std::string& category) { category_ = category; }

    bool is_mandatory() const { return mandatory_; }
    void set_mandatory(bool mandatory) { mandatory_ = mandatory; }

    int get_publisher_id() const { return publisher_id_; }
    void set_publisher_id(int publisher_id) { publisher_id_ = publisher_id; }

    std::time_t get_publish_time() const { return publish_time_; }
    void set_publish_time(std::time_t publish_time) { publish_time_ = publish_time; }

    const std::optional<std::time_t>& get_expire_time() const { return expire_time_; }
    void set_expire_time(const std::optional<std::time_t>& expire_time) { expire_time_ = expire_time; }

    std::time_t get_created_at() const { return created_at_; }
    void set_created_at(std::time_t created_at) { created_at_ = created_at; }

    std::time_t get_updated_at() const { return updated_at_; }
    void set_updated_at(std::time_t updated_at) { updated_at_ = updated_at; }

    Status get_status() const { return status_; }
    void set_status(Status status) { status_ = status; }
    static Status status_from_string(const std::string& status_str);
    static std::string status_to_string(Status status);

private:
    int id_ = 0;
    std::string title_;
    std::string content_;
    std::string category_;
    bool mandatory_ = false;
    int publisher_id_ = 0;
    std::time_t publish_time_ = 0;
    std::optional<std::time_t> expire_time_;
    std::time_t created_at_ = 0;
    std::time_t updated_at_ = 0;
    Status status_ = Status::NORMAL;
};

} // namespace models
