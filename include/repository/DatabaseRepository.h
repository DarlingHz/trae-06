#ifndef DATABASE_REPOSITORY_H
#define DATABASE_REPOSITORY_H

#include <sqlite3.h>
#include <memory>
#include <model/Event.h>
#include <model/User.h>
#include <model/Registration.h>
#include <model/CheckInLog.h>
#include <vector>
#include <optional>
#include <tuple>

namespace event_signup_service::repository {

struct EventStats {
    int total_registrations{};
    int registered_count{};
    int waiting_count{};
    int canceled_count{};
    int checked_in_count{};
};

struct EventListFilter {
    std::optional<std::string> keyword;
    std::optional<model::EventStatus> status;
    std::optional<std::chrono::system_clock::time_point> from;
    std::optional<std::chrono::system_clock::time_point> to;
};

struct EventListResult {
    std::vector<model::Event> events;
    int total_count;
};

struct RegistrationListFilter {
    std::optional<model::RegistrationStatus> status;
};

struct RegistrationListResult {
    std::vector<model::Registration> registrations;
    int total_count;
};

class DatabaseRepository {
private:
    sqlite3* db_;

    // 事务相关
    bool in_transaction_ = false;

    // 辅助方法
    int64_t get_last_insert_id();
    std::string escape_string(const std::string& str) const;
    std::chrono::system_clock::time_point parse_datetime(const std::string& datetime_str) const;
    std::string format_datetime(const std::chrono::system_clock::time_point& time) const;

public:
    DatabaseRepository(const std::string& db_path);
    ~DatabaseRepository();

    // 禁止拷贝
    DatabaseRepository(const DatabaseRepository&) = delete;
    DatabaseRepository& operator=(const DatabaseRepository&) = delete;

    // 允许移动
    DatabaseRepository(DatabaseRepository&&) noexcept;
    DatabaseRepository& operator=(DatabaseRepository&&) noexcept;

    // 事务管理
    void begin_transaction();
    void commit_transaction();
    void rollback_transaction();

    // Event相关操作
    int64_t create_event(const model::Event& event);
    bool update_event(const model::Event& event);
    std::optional<model::Event> get_event_by_id(int64_t event_id);
    EventListResult get_events(const EventListFilter& filter, int page, int page_size);
    EventStats get_event_stats(int64_t event_id);

    // User相关操作
    int64_t create_user(const model::User& user);
    std::optional<model::User> get_user_by_id(int64_t user_id);
    std::optional<model::User> get_user_by_email(const std::string& email);

    // Registration相关操作
    int64_t create_registration(const model::Registration& registration);
    bool update_registration(const model::Registration& registration);
    std::optional<model::Registration> get_registration_by_id(int64_t registration_id);
    std::optional<model::Registration> get_registration_by_user_and_event(int64_t user_id, int64_t event_id);
    RegistrationListResult get_event_registrations(int64_t event_id, const RegistrationListFilter& filter, int page, int page_size);
    std::vector<model::Registration> get_event_waiting_list(int64_t event_id);
    int get_registered_count(int64_t event_id);
    std::vector<model::Registration> get_user_registrations(int64_t user_id);

    // CheckInLog相关操作
    int64_t create_check_in_log(const model::CheckInLog& log);
    std::optional<model::CheckInLog> get_check_in_log_by_registration(int64_t registration_id);

    // 健康检查
    bool is_connected() const;
};

} // namespace event_signup_service::repository

#endif // DATABASE_REPOSITORY_H
