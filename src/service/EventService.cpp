#include <service/EventService.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace event_signup_service::service {

EventService::EventService(std::shared_ptr<repository::DatabaseRepository> repo) : repo_(std::move(repo)) {
    if (!repo_) {
        throw std::invalid_argument("repo不能为null");
    }
}

model::Event EventService::create_event(const model::Event& event) {
    spdlog::info("创建活动: title={}", event.title());

    model::Event new_event = event;
    int64_t event_id = repo_->create_event(new_event);
    new_event.set_id(event_id);

    spdlog::info("活动创建成功: id={}, title={}", event_id, event.title());
    return new_event;
}

std::optional<model::Event> EventService::update_event(int64_t event_id, const model::Event& event_data) {
    spdlog::info("更新活动: id={}", event_id);

    auto existing_event = repo_->get_event_by_id(event_id);
    if (!existing_event) {
        spdlog::warn("活动不存在: id={}", event_id);
        return std::nullopt;
    }

    // 校验: 活动已开始或已结束时，部分字段不可修改
    if (!existing_event->can_modify()) {
        spdlog::warn("活动已开始，不可修改: id={}", event_id);
        throw std::runtime_error("活动已开始，不可修改");
    }

    // 更新字段
    model::Event updated_event = *existing_event;
    updated_event.set_title(event_data.title());
    updated_event.set_description(event_data.description());
    updated_event.set_start_time(event_data.start_time());
    updated_event.set_end_time(event_data.end_time());
    updated_event.set_location(event_data.location());
    updated_event.set_capacity(event_data.capacity());
    updated_event.set_status(event_data.status());

    if (repo_->update_event(updated_event)) {
        spdlog::info("活动更新成功: id={}", event_id);
        return updated_event;
    }

    spdlog::error("活动更新失败: id={}", event_id);
    return std::nullopt;
}

std::optional<model::Event> EventService::get_event(int64_t event_id) {
    spdlog::info("获取活动详情: id={}", event_id);
    return repo_->get_event_by_id(event_id);
}

std::tuple<std::vector<model::Event>, int> EventService::get_events(const std::optional<std::string>& keyword,
                                                                      const std::optional<std::string>& status_str,
                                                                      const std::optional<std::chrono::system_clock::time_point>& from,
                                                                      const std::optional<std::chrono::system_clock::time_point>& to,
                                                                      int page, int page_size) {
    spdlog::info("获取活动列表: page={}, page_size={}, keyword={}, status={}", page, page_size, keyword.value_or(""), status_str.value_or(""));

    repository::EventListFilter filter;
    filter.keyword = keyword;

    if (status_str) {
        filter.status = model::Event::string_to_status(*status_str);
    }

    filter.from = from;
    filter.to = to;

    auto result = repo_->get_events(filter, page, page_size);
    return {result.events, result.total_count};
}

repository::EventStats EventService::get_event_stats(int64_t event_id) {
    spdlog::info("获取活动统计: id={}", event_id);

    // 验证活动是否存在
    if (!repo_->get_event_by_id(event_id)) {
        throw std::runtime_error("活动不存在");
    }

    return repo_->get_event_stats(event_id);
}

} // namespace event_signup_service::service
