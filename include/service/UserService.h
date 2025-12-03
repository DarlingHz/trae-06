#ifndef USER_SERVICE_H
#define USER_SERVICE_H

#include <model/User.h>
#include <repository/DatabaseRepository.h>
#include <memory>
#include <optional>

namespace event_signup_service::service {

class UserService {
private:
    std::shared_ptr<repository::DatabaseRepository> repo_;

public:
    UserService(std::shared_ptr<repository::DatabaseRepository> repo);
    ~UserService() = default;

    // 创建用户
    model::User create_user(const model::User& user);

    // 获取用户详情
    std::optional<model::User> get_user(int64_t user_id);

    // 根据邮箱获取用户
    std::optional<model::User> get_user_by_email(const std::string& email);

    // 获取用户报名记录
    std::vector<model::Registration> get_user_registrations(int64_t user_id);
};

} // namespace event_signup_service::service

#endif // USER_SERVICE_H
