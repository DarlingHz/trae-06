#include <controller/EventController.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace event_signup_service::controller {

using json = nlohmann::json;

void EventController::create_event(const httplib::Request& req, httplib::Response& res) {
    try {
        auto event_json = json::parse(req.body);

        // 解析请求参数
        model::Event event;
        if (event_json.contains("title")) {
            event.setTitle(event_json["title"].get<std::string>());
        }
        if (event_json.contains("description")) {
            event.setDescription(event_json["description"].get<std::string>());
        }
        if (event_json.contains("start_time")) {
            event.setStartTime(std::chrono::system_clock::from_time_t(event_json["start_time"].get<time_t>()));
        }
        if (event_json.contains("end_time")) {
            event.setEndTime(std::chrono::system_clock::from_time_t(event_json["end_time"].get<time_t>()));
        }
        if (event_json.contains("location")) {
            event.setLocation(event_json["location"].get<std::string>());
        }
        if (event_json.contains("capacity")) {
            event.setCapacity(event_json["capacity"].get<int>());
        }
        if (event_json.contains("status")) {
            event.setStatus(model::Event::string_to_status(event_json["status"].get<std::string>()));
        }

        auto created_event = event_service_->create_event(event);
        res.status = 201;
        res.set_content(json(created_event).dump(), "application/json");
    } catch (const std::exception& e) {
        spdlog::error("创建活动失败: {}", e.what());
        res.status = 400;
        res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    }
}

void EventController::update_event(const httplib::Request& req, httplib::Response& res) {
    try {
        int event_id = std::stoi(req.matches[1]);
        auto event_json = json::parse(req.body);

        // 获取现有活动
        auto event = event_service_->get_event(event_id);
        if (!event) {
            res.status = 404;
            res.set_content(json({{"error", "活动不存在"}}).dump(), "application/json");
            return;
        }

        // 解析请求参数
        if (event_json.contains("title")) {
            event->setTitle(event_json["title"].get<std::string>());
        }
        if (event_json.contains("description")) {
            event->setDescription(event_json["description"].get<std::string>());
        }
        if (event_json.contains("start_time")) {
            event->setStartTime(std::chrono::system_clock::from_time_t(event_json["start_time"].get<time_t>()));
        }
        if (event_json.contains("end_time")) {
            event->setEndTime(std::chrono::system_clock::from_time_t(event_json["end_time"].get<time_t>()));
        }
        if (event_json.contains("location")) {
            event->setLocation(event_json["location"].get<std::string>());
        }
        if (event_json.contains("capacity")) {
            event->setCapacity(event_json["capacity"].get<int>());
        }
        if (event_json.contains("status")) {
            event->setStatus(model::Event::string_to_status(event_json["status"].get<std::string>()));
        }

        auto updated_event = event_service_->update_event(*event);
        res.status = 200;
        res.set_content(json(updated_event).dump(), "application/json");
    } catch (const std::invalid_argument& e) {
        res.status = 400;
        res.set_content(json({{"error", "无效的活动ID"}}).dump(), "application/json");
    } catch (const std::exception& e) {
        spdlog::error("更新活动失败: {}", e.what());
        res.status = 400;
        res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    }
}

void EventController::get_event(const httplib::Request& req, httplib::Response& res) {
    try {
        int event_id = std::stoi(req.matches[1]);
        auto event = event_service_->get_event(event_id);

        if (!event) {
            res.status = 404;
            res.set_content(json({{"error", "活动不存在"}}).dump(), "application/json");
            return;
        }

        // 获取统计信息
        auto stats = event_service_->get_event_stats(event_id);

        json response = *event;
        response["registered_count"] = stats.registered_count;
        response["waiting_count"] = stats.waiting_count;
        response["checked_in_count"] = stats.checked_in_count;

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    } catch (const std::invalid_argument& e) {
        res.status = 400;
        res.set_content(json({{"error", "无效的活动ID"}}).dump(), "application/json");
    } catch (const std::exception& e) {
        spdlog::error("获取活动详情失败: {}", e.what());
        res.status = 500;
        res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    }
}

void EventController::get_events(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析查询参数
        int page = 1;
        int page_size = 20;
        std::string keyword;
        std::string status_str;
        time_t from_time = 0;
        time_t to_time = std::numeric_limits<time_t>::max();

        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
        }
        if (req.has_param("page_size")) {
            page_size = std::stoi(req.get_param_value("page_size"));
        }
        if (req.has_param("keyword")) {
            keyword = req.get_param_value("keyword");
        }
        if (req.has_param("status")) {
            status_str = req.get_param_value("status");
        }
        if (req.has_param("from")) {
            from_time = std::stoll(req.get_param_value("from"));
        }
        if (req.has_param("to")) {
            to_time = std::stoll(req.get_param_value("to"));
        }

        auto from = std::chrono::system_clock::from_time_t(from_time);
        auto to = std::chrono::system_clock::from_time_t(to_time);

        auto events = event_service_->get_events(page, page_size, keyword, status_str, from, to);

        json response;
        response["events"] = events;
        response["total"] = events.size();
        response["page"] = page;
        response["page_size"] = page_size;

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    } catch (const std::exception& e) {
        spdlog::error("获取活动列表失败: {}", e.what());
        res.status = 500;
        res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    }
}

void EventController::get_event_stats(const httplib::Request& req, httplib::Response& res) {
    try {
        int event_id = std::stoi(req.matches[1]);
        auto stats = event_service_->get_event_stats(event_id);

        json response;
        response["total_registrations"] = stats.registered_count + stats.waiting_count + stats.canceled_count;
        response["registered_count"] = stats.registered_count;
        response["waiting_count"] = stats.waiting_count;
        response["canceled_count"] = stats.canceled_count;
        response["checked_in_count"] = stats.checked_in_count;

        if (stats.registered_count > 0) {
            response["checkin_rate"] = static_cast<double>(stats.checked_in_count) / stats.registered_count;
        } else {
            response["checkin_rate"] = 0.0;
        }

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    } catch (const std::invalid_argument& e) {
        res.status = 400;
        res.set_content(json({{"error", "无效的活动ID"}}).dump(), "application/json");
    } catch (const std::exception& e) {
        spdlog::error("获取活动统计失败: {}", e.what());
        res.status = 500;
        res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    }
}

void EventController::get_event_registrations(const httplib::Request& req, httplib::Response& res) {
    try {
        int event_id = std::stoi(req.matches[1]);

        // 解析查询参数
        int page = 1;
        int page_size = 20;
        std::string status_str;

        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
        }
        if (req.has_param("page_size")) {
            page_size = std::stoi(req.get_param_value("page_size"));
        }
        if (req.has_param("status")) {
            status_str = req.get_param_value("status");
        }

        auto registrations = registration_service_->get_event_registrations(event_id, page, page_size, status_str);

        json response;
        response["registrations"] = registrations;
        response["total"] = registrations.size();
        response["page"] = page;
        response["page_size"] = page_size;

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    } catch (const std::invalid_argument& e) {
        res.status = 400;
        res.set_content(json({{"error", "无效的活动ID"}}).dump(), "application/json");
    } catch (const std::exception& e) {
        spdlog::error("获取活动报名列表失败: {}", e.what());
        res.status = 500;
        res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    }
}

void EventController::register_for_event(const httplib::Request& req, httplib::Response& res) {
    try {
        int event_id = std::stoi(req.matches[1]);
        auto body_json = json::parse(req.body);

        int user_id;
        if (body_json.contains("user_id")) {
            user_id = body_json["user_id"].get<int>();
        } else if (body_json.contains("email")) {
            auto user = user_service_->get_user_by_email(body_json["email"].get<std::string>());
            if (!user) {
                res.status = 404;
                res.set_content(json({{"error", "用户不存在"}}).dump(), "application/json");
                return;
            }
            user_id = user->getId();
        } else {
            res.status = 400;
            res.set_content(json({{"error", "缺少user_id或email参数"}}).dump(), "application/json");
            return;
        }

        auto result = registration_service_->register_for_event(user_id, event_id);

        json response = *result.registration;
        response["message"] = "报名成功";
        if (result.registration->getStatus() == model::Registration::RegistrationStatus::WAITING) {
            response["message"] = "报名成功，进入等候名单";
            response["position"] = result.waiting_position;
        }

        res.status = 201;
        res.set_content(response.dump(), "application/json");
    } catch (const std::invalid_argument& e) {
        res.status = 400;
        res.set_content(json({{"error", "无效的活动ID"}}).dump(), "application/json");
    } catch (const std::exception& e) {
        spdlog::error("活动报名失败: {}", e.what());
        res.status = 400;
        res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    }
}

void EventController::cancel_registration(const httplib::Request& req, httplib::Response& res) {
    try {
        int event_id = std::stoi(req.matches[1]);
        auto body_json = json::parse(req.body);

        int user_id;
        if (body_json.contains("user_id")) {
            user_id = body_json["user_id"].get<int>();
        } else if (body_json.contains("email")) {
            auto user = user_service_->get_user_by_email(body_json["email"].get<std::string>());
            if (!user) {
                res.status = 404;
                res.set_content(json({{"error", "用户不存在"}}).dump(), "application/json");
                return;
            }
            user_id = user->getId();
        } else {
            res.status = 400;
            res.set_content(json({{"error", "缺少user_id或email参数"}}).dump(), "application/json");
            return;
        }

        auto canceled_reg = registration_service_->cancel_registration(user_id, event_id);

        json response = *canceled_reg;
        response["message"] = "取消报名成功";

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    } catch (const std::invalid_argument& e) {
        res.status = 400;
        res.set_content(json({{"error", "无效的活动ID"}}).dump(), "application/json");
    } catch (const std::exception& e) {
        spdlog::error("取消报名失败: {}", e.what());
        res.status = 400;
        res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    }
}

void EventController::check_in(const httplib::Request& req, httplib::Response& res) {
    try {
        int event_id = std::stoi(req.matches[1]);
        auto body_json = json::parse(req.body);

        int registration_id = 0;

        if (body_json.contains("registration_id")) {
            registration_id = body_json["registration_id"].get<int>();
        } else if (body_json.contains("user_id")) {
            // 通过user_id和event_id查找报名记录
            auto registrations = registration_service_->get_event_registrations(
                event_id, 1, 1, "REGISTERED"
            );
            for (const auto& reg : registrations) {
                if (reg.getUserId() == body_json["user_id"].get<int>()) {
                    registration_id = reg.getId();
                    break;
                }
            }
            if (registration_id == 0) {
                res.status = 404;
                res.set_content(json({{"error", "该用户没有有效的报名记录"}}).dump(), "application/json");
                return;
            }
        } else if (body_json.contains("email")) {
            // 通过email和event_id查找报名记录
            auto user = user_service_->get_user_by_email(body_json["email"].get<std::string>());
            if (!user) {
                res.status = 404;
                res.set_content(json({{"error", "用户不存在"}}).dump(), "application/json");
                return;
            }

            auto registrations = registration_service_->get_event_registrations(
                event_id, 1, 1, "REGISTERED"
            );
            for (const auto& reg : registrations) {
                if (reg.getUserId() == user->getId()) {
                    registration_id = reg.getId();
                    break;
                }
            }
            if (registration_id == 0) {
                res.status = 404;
                res.set_content(json({{"error", "该用户没有有效的报名记录"}}).dump(), "application/json");
                return;
            }
        } else {
            res.status = 400;
            res.set_content(json({{"error", "缺少registration_id、user_id或email参数"}}).dump(), "application/json");
            return;
        }

        std::string channel_str = "MANUAL";
        if (body_json.contains("channel")) {
            channel_str = body_json["channel"].get<std::string>();
        }

        auto channel = model::CheckInLog::string_to_channel(channel_str);
        auto checked_in_reg = registration_service_->check_in(registration_id, channel);

        json response = *checked_in_reg;
        response["message"] = "签到成功";

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    } catch (const std::invalid_argument& e) {
        res.status = 400;
        res.set_content(json({{"error", "无效的活动ID"}}).dump(), "application/json");
    } catch (const std::exception& e) {
        spdlog::error("签到失败: {}", e.what());
        res.status = 400;
        res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    }
}

} // namespace event_signup_service::controller
