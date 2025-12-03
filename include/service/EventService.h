#ifndef EVENT_SERVICE_H
#define EVENT_SERVICE_H

#include <model/Event.h>
#include <repository/DatabaseRepository.h>
#include <memory>
#include <vector>
#include <optional>
#include <tuple>

namespace event_signup_service::service {

class EventService {
private:
    std::shared_ptr<repository::DatabaseRepository> repo_;

public:
    EventService(std::shared_ptr<repository::DatabaseRepository> repo);
    ~EventService() = default;

    // 创建活动
    model::Event create_event(const model::Event& event);

    // 更新活动
    std::optional<model::Event> update_event(int64_t event_id, const model::Event& event_data);

    // 获取活动详情
    std::optional<model::Event> get_event(int64_t event_id);

    // 获取活动列表
    std::tuple<std::vector<model::Event>, int> get_events(const std::optional<std::string>& keyword,
                                                          const std::optional<std::string>& status,
                                                          const std::optional<std::chrono::system_clock::time_point>& from,
                                                          const std::optional<std::chrono::system_clock::time_point>& to,
                                                          int page, int page_size);

    // 获取活动统计信息
    repository::EventStats get_event_stats(int64_t event_id);
};

} // namespace event_signup_service::service

#endif // EVENT_SERVICE_H
