#ifndef USER_H
#define USER_H

#include <string>
#include <optional>
#include <chrono>
#include <nlohmann/json.hpp>

namespace event_signup_service::model {

class User {
private:
    int64_t id_{};
    std::string name_{};
    std::string email_{};
    std::optional<std::string> phone_{};
    std::chrono::system_clock::time_point created_at_{};

public:
    // 构造函数
    User() = default;
    User(
        const std::string& name,
        const std::string& email,
        const std::optional<std::string>& phone = std::nullopt
    ) : name_(name),
        email_(email),
        phone_(phone),
        created_at_(std::chrono::system_clock::now()) {}

    // Getter方法
    int64_t id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::string& email() const { return email_; }
    const std::optional<std::string>& phone() const { return phone_; }
    const std::chrono::system_clock::time_point& created_at() const { return created_at_; }

    // Setter方法
    void set_id(int64_t id) { id_ = id; }
    void set_name(const std::string& name) { name_ = name; }
    void set_email(const std::string& email) { email_ = email; }
    void set_phone(const std::optional<std::string>& phone) { phone_ = phone; }

    // JSON序列化
    friend void to_json(nlohmann::json& j, const User& user);
    friend void from_json(const nlohmann::json& j, User& user);
};

} // namespace event_signup_service::model

#endif // USER_H
