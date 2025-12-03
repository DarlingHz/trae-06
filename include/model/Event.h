#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <optional>
#include <chrono>
#include <nlohmann/json.hpp>

namespace event_signup_service::model {

enum class EventStatus {
    DRAFT,      // 草稿
    PUBLISHED,  // 已发布
    CLOSED      // 已关闭
};

class Event {
private:
    int64_t id_{};
    std::string title_{};
    std::optional<std::string> description_{};
    std::chrono::system_clock::time_point start_time_{};
    std::chrono::system_clock::time_point end_time_{};
    std::string location_{};
    int capacity_{};
    EventStatus status_{EventStatus::DRAFT};
    std::chrono::system_clock::time_point created_at_{};
    std::chrono::system_clock::time_point updated_at_{};

public:
    // 构造函数
    Event() = default;
    Event(
        const std::string& title,
        const std::optional<std::string>& description,
        const std::chrono::system_clock::time_point& start_time,
        const std::chrono::system_clock::time_point& end_time,
        const std::string& location,
        int capacity,
        EventStatus status = EventStatus::DRAFT
    ) : title_(title),
        description_(description),
        start_time_(start_time),
        end_time_(end_time),
        location_(location),
        capacity_(capacity),
        status_(status),
        created_at_(std::chrono::system_clock::now()),
        updated_at_(std::chrono::system_clock::now()) {}

    // Getter方法
    int64_t id() const { return id_; }
    const std::string& title() const { return title_; }
    const std::optional<std::string>& description() const { return description_; }
    const std::chrono::system_clock::time_point& start_time() const { return start_time_; }
    const std::chrono::system_clock::time_point& end_time() const { return end_time_; }
    const std::string& location() const { return location_; }
    int capacity() const { return capacity_; }
    EventStatus status() const { return status_; }
    const std::chrono::system_clock::time_point& created_at() const { return created_at_; }
    const std::chrono::system_clock::time_point& updated_at() const { return updated_at_; }

    // Setter方法
    void set_id(int64_t id) { id_ = id; }
    void set_title(const std::string& title) { title_ = title; updated_at_ = std::chrono::system_clock::now(); }
    void set_description(const std::optional<std::string>& description) { description_ = description; updated_at_ = std::chrono::system_clock::now(); }
    void set_start_time(const std::chrono::system_clock::time_point& start_time) { start_time_ = start_time; updated_at_ = std::chrono::system_clock::now(); }
    void set_end_time(const std::chrono::system_clock::time_point& end_time) { end_time_ = end_time; updated_at_ = std::chrono::system_clock::now(); }
    void set_location(const std::string& location) { location_ = location; updated_at_ = std::chrono::system_clock::now(); }
    void set_capacity(int capacity) { capacity_ = capacity; updated_at_ = std::chrono::system_clock::now(); }
    void set_status(EventStatus status) { status_ = status; updated_at_ = std::chrono::system_clock::now(); }

    // 辅助方法
    std::string status_to_string() const;
    static EventStatus string_to_status(const std::string& status);
    bool can_modify() const;

    // JSON序列化
    friend void to_json(nlohmann::json& j, const Event& event);
    friend void from_json(const nlohmann::json& j, Event& event);
};

} // namespace event_signup_service::model

#endif // EVENT_H
