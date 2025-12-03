#include <service/RegistrationService.h>
#include <model/Event.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace event_signup_service::service {

RegistrationService::RegistrationService(std::shared_ptr<repository::DatabaseRepository> repo) : repo_(std::move(repo)) {
    if (!repo_) {
        throw std::invalid_argument("repo不能为null");
    }
}

RegisterResult RegistrationService::register_for_event(int64_t user_id, int64_t event_id) {
    spdlog::info("用户报名活动: user_id={}, event_id={}", user_id, event_id);

    repo_->begin_transaction();

    try {
        // 1. 验证用户和活动是否存在
        auto user = repo_->get_user_by_id(user_id);
        if (!user) {
            throw std::runtime_error("用户不存在");
        }

        auto event = repo_->get_event_by_id(event_id);
        if (!event) {
            throw std::runtime_error("活动不存在");
        }

        // 2. 检查活动状态
        if (event->status() != model::EventStatus::PUBLISHED) {
            throw std::runtime_error("活动未发布或已关闭");
        }

        // 3. 检查是否已存在有效报名
        auto existing_registration = repo_->get_registration_by_user_and_event(user_id, event_id);
        if (existing_registration && existing_registration->is_active()) {
            spdlog::info("用户已报名该活动: user_id={}, event_id={}", user_id, event_id);
            repo_->commit_transaction();
            return {*existing_registration, existing_registration->status() == model::RegistrationStatus::WAITING};
        }

        // 4. 检查当前报名人数
        int registered_count = repo_->get_registered_count(event_id);
        model::RegistrationStatus status = model::RegistrationStatus::REGISTERED;
        bool was_added_to_waiting_list = false;

        if (registered_count >= event->capacity()) {
            status = model::RegistrationStatus::WAITING;
            was_added_to_waiting_list = true;
            spdlog::info("活动已达报名上限，加入等候名单: event_id={}, capacity={}, current={}", event_id, event->capacity(), registered_count);
        }

        // 5. 创建报名记录
        model::Registration registration(user_id, event_id, status);
        int64_t registration_id = repo_->create_registration(registration);
        registration.set_id(registration_id);

        repo_->commit_transaction();

        if (was_added_to_waiting_list) {
            spdlog::info("用户加入等候名单: user_id={}, event_id={}, registration_id={}", user_id, event_id, registration_id);
        } else {
            spdlog::info("用户报名成功: user_id={}, event_id={}, registration_id={}", user_id, event_id, registration_id);
        }

        return {registration, was_added_to_waiting_list};

    } catch (const std::exception& e) {
        repo_->rollback_transaction();
        spdlog::error("报名失败: user_id={}, event_id={}, error={}", user_id, event_id, e.what());
        throw;
    }
}

std::optional<model::Registration> RegistrationService::cancel_registration(int64_t user_id, int64_t event_id) {
    spdlog::info("用户取消报名: user_id={}, event_id={}", user_id, event_id);

    repo_->begin_transaction();

    try {
        // 1. 获取用户的报名记录
        auto registration = repo_->get_registration_by_user_and_event(user_id, event_id);
        if (!registration) {
            spdlog::warn("报名记录不存在: user_id={}, event_id={}", user_id, event_id);
            repo_->commit_transaction();
            return std::nullopt;
        }

        // 2. 检查报名状态
        if (registration->status() == model::RegistrationStatus::CANCELED) {
            spdlog::info("报名已取消: user_id={}, event_id={}", user_id, event_id);
            repo_->commit_transaction();
            return *registration;
        }

        if (registration->status() == model::RegistrationStatus::CHECKED_IN) {
            throw std::runtime_error("已签到，无法取消");
        }

        // 3. 将报名状态改为取消
        registration->set_status(model::RegistrationStatus::CANCELED);
        repo_->update_registration(*registration);

        // 4. 如果是REGISTERED状态，从等候名单中提升一位
        if (registration->status() == model::RegistrationStatus::REGISTERED) {
            auto waiting_list = repo_->get_event_waiting_list(event_id);
            if (!waiting_list.empty()) {
                auto first_waiting = waiting_list[0];
                first_waiting.set_status(model::RegistrationStatus::REGISTERED);
                repo_->update_registration(first_waiting);
                spdlog::info("从等候名单提升用户: event_id={}, user_id={}, registration_id={}", event_id, first_waiting.user_id(), first_waiting.id());
            }
        }

        repo_->commit_transaction();

        spdlog::info("取消报名成功: user_id={}, event_id={}, registration_id={}", user_id, event_id, registration->id());
        return *registration;

    } catch (const std::exception& e) {
        repo_->rollback_transaction();
        spdlog::error("取消报名失败: user_id={}, event_id={}, error={}", user_id, event_id, e.what());
        throw;
    }
}

model::CheckInLog RegistrationService::check_in(int64_t user_id, int64_t event_id, const std::string& channel_str) {
    spdlog::info("用户签到: user_id={}, event_id={}, channel={}", user_id, event_id, channel_str);

    repo_->begin_transaction();

    try {
        // 1. 获取用户的报名记录
        auto registration = repo_->get_registration_by_user_and_event(user_id, event_id);
        if (!registration) {
            throw std::runtime_error("未找到报名记录");
        }

        // 2. 检查报名状态
        if (registration->status() == model::RegistrationStatus::CHECKED_IN) {
            throw std::runtime_error("已签到，无法重复签到");
        }

        if (registration->status() != model::RegistrationStatus::REGISTERED) {
            throw std::runtime_error("报名状态无效，无法签到");
        }

        // 3. 转换签到渠道
        model::CheckInChannel channel = model::CheckInLog::string_to_channel(channel_str);

        // 4. 更新报名状态为已签到
        registration->set_status(model::RegistrationStatus::CHECKED_IN);
        repo_->update_registration(*registration);

        // 5. 创建签到日志
        model::CheckInLog check_in_log(registration->id(), channel);
        int64_t log_id = repo_->create_check_in_log(check_in_log);
        check_in_log.set_id(log_id);

        repo_->commit_transaction();

        spdlog::info("签到成功: user_id={}, event_id={}, registration_id={}, log_id={}", user_id, event_id, registration->id(), log_id);
        return check_in_log;

    } catch (const std::exception& e) {
        repo_->rollback_transaction();
        spdlog::error("签到失败: user_id={}, event_id={}, error={}", user_id, event_id, e.what());
        throw;
    }
}

std::tuple<std::vector<model::Registration>, int> RegistrationService::get_event_registrations(
    int64_t event_id,
    const std::optional<std::string>& status_str,
    int page,
    int page_size
) {
    spdlog::info("获取活动报名列表: event_id={}, status={}, page={}, page_size={}", event_id, status_str.value_or(""), page, page_size);

    // 验证活动是否存在
    if (!repo_->get_event_by_id(event_id)) {
        throw std::runtime_error("活动不存在");
    }

    repository::RegistrationListFilter filter;
    if (status_str) {
        filter.status = model::Registration::string_to_status(*status_str);
    }

    auto result = repo_->get_event_registrations(event_id, filter, page, page_size);
    return {result.registrations, result.total_count};
}

} // namespace event_signup_service::service
