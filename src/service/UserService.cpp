#include <service/UserService.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace event_signup_service::service {

UserService::UserService(std::shared_ptr<repository::DatabaseRepository> repo) : repo_(std::move(repo)) {
    if (!repo_) {
        throw std::invalid_argument("repo不能为null");
    }
}

model::User UserService::create_user(const model::User& user) {
    spdlog::info("创建用户: name={}, email={}", user.name(), user.email());

    // 检查邮箱是否已存在
    if (repo_->get_user_by_email(user.email())) {
        spdlog::warn("邮箱已存在: email={}", user.email());
        throw std::runtime_error("邮箱已存在");
    }

    model::User new_user = user;
    int64_t user_id = repo_->create_user(new_user);
    new_user.set_id(user_id);

    spdlog::info("用户创建成功: id={}, name={}, email={}", user_id, user.name(), user.email());
    return new_user;
}

std::optional<model::User> UserService::get_user(int64_t user_id) {
    spdlog::info("获取用户详情: id={}", user_id);
    return repo_->get_user_by_id(user_id);
}

std::optional<model::User> UserService::get_user_by_email(const std::string& email) {
    spdlog::info("根据邮箱获取用户: email={}", email);
    return repo_->get_user_by_email(email);
}

std::vector<model::Registration> UserService::get_user_registrations(int64_t user_id) {
    spdlog::info("获取用户报名记录: id={}", user_id);

    // 验证用户是否存在
    if (!repo_->get_user_by_id(user_id)) {
        throw std::runtime_error("用户不存在");
    }

    return repo_->get_user_registrations(user_id);
}

} // namespace event_signup_service::service
