#include <controller/UserController.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace event_signup_service::controller {

using json = nlohmann::json;

void UserController::create_user(const httplib::Request& req, httplib::Response& res) {
    try {
        auto user_json = json::parse(req.body);

        // 解析请求参数
        std::string name;
        std::string email;
        std::string phone;

        if (!user_json.contains("name")) {
            res.status = 400;
            res.set_content(json({{"error", "缺少name参数"}}).dump(), "application/json");
            return;
        }
        name = user_json["name"].get<std::string>();

        if (!user_json.contains("email")) {
            res.status = 400;
            res.set_content(json({{"error", "缺少email参数"}}).dump(), "application/json");
            return;
        }
        email = user_json["email"].get<std::string>();

        if (user_json.contains("phone")) {
            phone = user_json["phone"].get<std::string>();
        }

        model::User user{0, name, email, phone, std::chrono::system_clock::now()};
        auto created_user = user_service_->create_user(user);

        res.status = 201;
        res.set_content(json(created_user).dump(), "application/json");
    } catch (const std::invalid_argument& e) {
        res.status = 400;
        res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    } catch (const std::runtime_error& e) {
        if (std::string(e.what()) == "邮箱已存在") {
            res.status = 409;
        } else {
            res.status = 500;
        }
        res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    } catch (const std::exception& e) {
        spdlog::error("创建用户失败: {}", e.what());
        res.status = 500;
        res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    }
}

void UserController::get_user(const httplib::Request& req, httplib::Response& res) {
    try {
        int user_id = std::stoi(req.matches[1]);
        auto user = user_service_->get_user(user_id);

        if (!user) {
            res.status = 404;
            res.set_content(json({{"error", "用户不存在"}}).dump(), "application/json");
            return;
        }

        res.status = 200;
        res.set_content(json(*user).dump(), "application/json");
    } catch (const std::invalid_argument& e) {
        res.status = 400;
        res.set_content(json({{"error", "无效的用户ID"}}).dump(), "application/json");
    } catch (const std::exception& e) {
        spdlog::error("获取用户详情失败: {}", e.what());
        res.status = 500;
        res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    }
}

void UserController::get_user_registrations(const httplib::Request& req, httplib::Response& res) {
    try {
        int user_id = std::stoi(req.matches[1]);

        // 解析查询参数
        int page = 1;
        int page_size = 20;

        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
        }
        if (req.has_param("page_size")) {
            page_size = std::stoi(req.get_param_value("page_size"));
        }

        auto registrations = user_service_->get_user_registrations(user_id, page, page_size);

        json response;
        response["registrations"] = registrations;
        response["total"] = registrations.size();
        response["page"] = page;
        response["page_size"] = page_size;

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    } catch (const std::invalid_argument& e) {
        res.status = 400;
        res.set_content(json({{"error", "无效的用户ID"}}).dump(), "application/json");
    } catch (const std::exception& e) {
        spdlog::error("获取用户报名记录失败: {}", e.what());
        res.status = 500;
        res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    }
}

} // namespace event_signup_service::controller
