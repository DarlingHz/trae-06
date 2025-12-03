#ifndef REGISTRATION_SERVICE_H
#define REGISTRATION_SERVICE_H

#include <model/Registration.h>
#include <model/CheckInLog.h>
#include <repository/DatabaseRepository.h>
#include <memory>
#include <optional>
#include <vector>

namespace event_signup_service::service {

struct RegisterResult {
    model::Registration registration;
    bool was_added_to_waiting_list = false;
};

class RegistrationService {
private:
    std::shared_ptr<repository::DatabaseRepository> repo_;

public:
    RegistrationService(std::shared_ptr<repository::DatabaseRepository> repo);
    ~RegistrationService() = default;

    // 用户报名活动
    RegisterResult register_for_event(int64_t user_id, int64_t event_id);

    // 取消报名
    std::optional<model::Registration> cancel_registration(int64_t user_id, int64_t event_id);

    // 活动签到
    model::CheckInLog check_in(int64_t user_id, int64_t event_id, const std::string& channel);

    // 获取活动报名列表
    std::tuple<std::vector<model::Registration>, int> get_event_registrations(
        int64_t event_id,
        const std::optional<std::string>& status,
        int page,
        int page_size
    );
};

} // namespace event_signup_service::service

#endif // REGISTRATION_SERVICE_H
