#include <cpp-httplib/httplib.h>
#include <spdlog/spdlog.h>
#include <config/Config.h>
#include <repository/DatabaseRepository.h>
#include <service/EventService.h>
#include <service/UserService.h>
#include <service/RegistrationService.h>
#include <controller/EventController.h>
#include <controller/UserController.h>
#include <controller/HealthController.h>

int main(int argc, char* argv[]) {
    try {
        // 初始化配置
        event_signup_service::config::Config::initialize();
        const auto& config = event_signup_service::config::Config::get();

        // 初始化日志
        spdlog::set_level(spdlog::level::from_str(config.service.log_level));
        spdlog::info("服务启动，版本: 1.0.0");
        spdlog::info("配置加载成功 - 端口: {}, 数据库: {}", config.service.port, config.database.path);

        // 初始化数据库连接和仓库
        auto db_repo = std::make_shared<event_signup_service::repository::DatabaseRepository>(
            config.database.path
        );

        // 初始化服务层
        auto event_service = std::make_shared<event_signup_service::service::EventService>(db_repo);
        auto user_service = std::make_shared<event_signup_service::service::UserService>(db_repo);
        auto registration_service = std::make_shared<event_signup_service::service::RegistrationService>(db_repo);

        // 初始化控制器
        auto event_controller = std::make_shared<event_signup_service::controller::EventController>(
            event_service, user_service, registration_service
        );
        auto user_controller = std::make_shared<event_signup_service::controller::UserController>(
            event_service, user_service, registration_service
        );
        auto health_controller = std::make_shared<event_signup_service::controller::HealthController>(db_repo);

        // 创建HTTP服务器
        httplib::Server server;
        server.set_keep_alive_max_count(100);
        server.set_keep_alive_timeout(5);

        // 事件相关路由
        server.Post("/events", [event_controller](const auto& req, auto& res) {
            event_controller->create_event(req, res);
        });

        server.Put(R"(/events/(\d+))", [event_controller](const auto& req, auto& res) {
            event_controller->update_event(req, res);
        });

        server.Get(R"(/events/(\d+))", [event_controller](const auto& req, auto& res) {
            event_controller->get_event(req, res);
        });

        server.Get("/events", [event_controller](const auto& req, auto& res) {
            event_controller->get_events(req, res);
        });

        server.Get(R"(/events/(\d+)/stats)", [event_controller](const auto& req, auto& res) {
            event_controller->get_event_stats(req, res);
        });

        server.Get(R"(/events/(\d+)/registrations)", [event_controller](const auto& req, auto& res) {
            event_controller->get_event_registrations(req, res);
        });

        server.Post(R"(/events/(\d+)/register)", [event_controller](const auto& req, auto& res) {
            event_controller->register_for_event(req, res);
        });

        server.Post(R"(/events/(\d+)/cancel)", [event_controller](const auto& req, auto& res) {
            event_controller->cancel_registration(req, res);
        });

        server.Post(R"(/events/(\d+)/checkin)", [event_controller](const auto& req, auto& res) {
            event_controller->check_in(req, res);
        });

        // 用户相关路由
        server.Post("/users", [user_controller](const auto& req, auto& res) {
            user_controller->create_user(req, res);
        });

        server.Get(R"(/users/(\d+))", [user_controller](const auto& req, auto& res) {
            user_controller->get_user(req, res);
        });

        server.Get(R"(/users/(\d+)/registrations)", [user_controller](const auto& req, auto& res) {
            user_controller->get_user_registrations(req, res);
        });

        // 健康检查路由
        server.Get("/healthz", [health_controller](const auto& req, auto& res) {
            health_controller->check_health(req, res);
        });

        // 启动服务器
        spdlog::info("服务器启动，监听地址: {}:{}...", config.service.host, config.service.port);
        if (!server.listen(config.service.host, config.service.port)) {
            spdlog::error("服务器启动失败，地址: {}:{}", config.service.host, config.service.port);
            return 1;
        }

        return 0;
    } catch (const std::exception& e) {
        spdlog::error("服务启动失败: {}", e.what());
        return 1;
    }
}
