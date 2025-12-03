#include <controller/HealthController.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace event_signup_service::controller {

using json = nlohmann::json;

void HealthController::check_health(const httplib::Request& req, httplib::Response& res) {
    try {
        bool db_ok = db_repo_->check_health();

        json response;
        response["status"] = "ok";
        response["version"] = "1.0.0";
        response["database"] = db_ok ? "connected" : "disconnected";

        res.status = db_ok ? 200 : 503;
        res.set_content(response.dump(), "application/json");
    } catch (const std::exception& e) {
        spdlog::error("健康检查失败: {}", e.what());
        res.status = 503;
        res.set_content(json({
            {"status", "error"},
            {"version", "1.0.0"},
            {"database", "error"},
            {"message", e.what()}
        }).dump(), "application/json");
    }
}

} // namespace event_signup_service::controller
