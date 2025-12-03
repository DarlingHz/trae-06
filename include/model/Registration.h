#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

namespace event_signup_service::model {

enum class RegistrationStatus {
    REGISTERED,  // 已报名
    WAITING,     // 等候
    CANCELED,    // 已取消
    CHECKED_IN   // 已签到
};

class Registration {
private:
    int64_t id_{};
    int64_t user_id_{};
    int64_t event_id_{};
    RegistrationStatus status_{RegistrationStatus::REGISTERED};
    std::chrono::system_clock::time_point created_at_{};
    std::chrono::system_clock::time_point updated_at_{};

public:
    // 构造函数
    Registration() = default;
    Registration(
        int64_t user_id,
        int64_t event_id,
        RegistrationStatus status = RegistrationStatus::REGISTERED
    ) : user_id_(user_id),
        event_id_(event_id),
        status_(status),
        created_at_(std::chrono::system_clock::now()),
        updated_at_(std::chrono::system_clock::now()) {}

    // Getter方法
    int64_t id() const { return id_; }
    int64_t user_id() const { return user_id_; }
    int64_t event_id() const { return event_id_; }
    RegistrationStatus status() const { return status_; }
    const std::chrono::system_clock::time_point& created_at() const { return created_at_; }
    const std::chrono::system_clock::time_point& updated_at() const { return updated_at_; }

    // Setter方法
    void set_id(int64_t id) { id_ = id; }
    void set_status(RegistrationStatus status) { status_ = status; updated_at_ = std::chrono::system_clock::now(); }

    // 辅助方法
    std::string status_to_string() const;
    static RegistrationStatus string_to_status(const std::string& status);
    bool is_active() const { return status_ == RegistrationStatus::REGISTERED || status_ == RegistrationStatus::WAITING; }

    // JSON序列化
    friend void to_json(nlohmann::json& j, const Registration& registration);
    friend void from_json(const nlohmann::json& j, Registration& registration);
};

} // namespace event_signup_service::model

#endif // REGISTRATION_H
