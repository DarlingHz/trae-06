#include "parking/controllers.h"
#include "parking/services.h"
#include "parking/utils.h"
#include <httplib.h>
#include <string>

// AuthMiddleware实现
AuthMiddleware::AuthMiddleware(UserService* user_service) : user_service_(user_service) {}

bool AuthMiddleware::operator()(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取Token
        std::string token = req.get_header_value("X-Auth-Token");

        if (token.empty()) {
            ResponseUtils::error(res, 401, "Unauthorized: Missing X-Auth-Token header");
            return false;
        }

        // 验证Token
        auto user_opt = user_service_->validate_token(token);

        if (!user_opt) {
            ResponseUtils::error(res, 401, "Unauthorized: Invalid or expired token");
            return false;
        }

        // 将用户ID存储在请求上下文中
        req.headers.insert({"X-Current-User-Id", std::to_string(user_opt->id)});
        return true;
    } catch (const std::exception& e) {
        Logger::error("Auth middleware exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
        return false;
    }
}

// Server setup
void setup_routes(httplib::Server& server,
                  UserController* user_controller,
                  ParkingSpotController* parking_spot_controller,
                  ReservationController* reservation_controller,
                  AuthMiddleware* auth_middleware) {
    // 版本端点
    server.Get("/api/version", [](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json data;
        data["version"] = "1.0.0";
        ResponseUtils::success(res, 200, "Service version", data);
    });

    // 健康检查
    server.Get("/health", [](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json data;
        data["status"] = "healthy";
        ResponseUtils::success(res, 200, "Service status", data);
    });

    // 用户端点
    server.Post("/api/users/register", std::bind(&UserController::register_user, user_controller, std::placeholders::_1, std::placeholders::_2));
    server.Post("/api/users/login", std::bind(&UserController::login, user_controller, std::placeholders::_1, std::placeholders::_2));
    server.Get("/api/users/me", *auth_middleware, std::bind(&UserController::get_current_user, user_controller, std::placeholders::_1, std::placeholders::_2));
    server.Post("/api/users/logout", *auth_middleware, std::bind(&UserController::logout, user_controller, std::placeholders::_1, std::placeholders::_2));

    // 停车位端点
    server.Post("/api/parking-spots", *auth_middleware, std::bind(&ParkingSpotController::create_spot, parking_spot_controller, std::placeholders::_1, std::placeholders::_2));
    server.Get("/api/parking-spots/my", *auth_middleware, std::bind(&ParkingSpotController::get_user_spots, parking_spot_controller, std::placeholders::_1, std::placeholders::_2));
    server.Put("/api/parking-spots/(\\d+)" , *auth_middleware, std::bind(&ParkingSpotController::update_spot, parking_spot_controller, std::placeholders::_1, std::placeholders::_2));
    server.Delete("/api/parking-spots/(\\d+)" , *auth_middleware, std::bind(&ParkingSpotController::deactivate_spot, parking_spot_controller, std::placeholders::_1, std::placeholders::_2));
    server.Get("/api/parking-spots/search", std::bind(&ParkingSpotController::search_spots, parking_spot_controller, std::placeholders::_1, std::placeholders::_2));
    server.Get("/api/parking-spots/(\\d+)" , std::bind(&ParkingSpotController::get_spot, parking_spot_controller, std::placeholders::_1, std::placeholders::_2));

    // 预约端点
    server.Post("/api/reservations", *auth_middleware, std::bind(&ReservationController::create_reservation, reservation_controller, std::placeholders::_1, std::placeholders::_2));
    server.Get("/api/reservations/my", *auth_middleware, std::bind(&ReservationController::get_user_reservations, reservation_controller, std::placeholders::_1, std::placeholders::_2));
    server.Get("/api/reservations/for-my-spots", *auth_middleware, std::bind(&ReservationController::get_owner_reservations, reservation_controller, std::placeholders::_1, std::placeholders::_2));
    server.Post("/api/reservations/(\\d+)/cancel", *auth_middleware, std::bind(&ReservationController::cancel_reservation, reservation_controller, std::placeholders::_1, std::placeholders::_2));
    server.Post("/api/reservations/(\\d+)/finish", *auth_middleware, std::bind(&ReservationController::finish_reservation, reservation_controller, std::placeholders::_1, std::placeholders::_2));
}
