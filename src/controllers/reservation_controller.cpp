#include "parking/controllers.h"
#include "parking/services.h"
#include "parking/models.h"
#include "parking/utils.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

// ReservationController实现
void ReservationController::create_reservation(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取当前用户
        const auto& user_id_str = req.get_header_value("X-Current-User-Id");
        if (user_id_str.empty()) {
            ResponseUtils::error(res, 401, "Unauthorized");
            return;
        }
        int user_id = std::stoi(user_id_str);
        auto user_opt = user_service_->get_user(user_id);
        if (!user_opt) {
            ResponseUtils::error(res, 401, "User not found");
            return;
        }

        // 解析请求
        auto data = json::parse(req.body);

        // 验证参数
        if (!data.contains("spot_id") || !data["spot_id"].is_number()) {
            ResponseUtils::error(res, 400, "Invalid spot_id");
            return;
        }
        if (!data.contains("start_time") || !data["start_time"].is_number()) {
            ResponseUtils::error(res, 400, "Invalid start_time");
            return;
        }
        if (!data.contains("end_time") || !data["end_time"].is_number()) {
            ResponseUtils::error(res, 400, "Invalid end_time");
            return;
        }
        if (!data.contains("vehicle_plate") || !data["vehicle_plate"].is_string() || data["vehicle_plate"].get<std::string>().empty()) {
            ResponseUtils::error(res, 400, "Invalid vehicle_plate");
            return;
        }

        int spot_id = data["spot_id"].get<int>();
        std::time_t start_time = data["start_time"].get<std::time_t>();
        std::time_t end_time = data["end_time"].get<std::time_t>();
        std::string vehicle_plate = data["vehicle_plate"].get<std::string>();

        // 调用服务
        Reservation reservation = reservation_service_->create_reservation(*user_opt, spot_id, start_time, end_time, vehicle_plate);

        // 构建响应
        json response_data;
        response_data["reservation"] = reservation;
        ResponseUtils::success(res, 201, "Reservation created successfully", response_data);
    } catch (const ServiceError& e) {
        Logger::error("Create reservation error: " + std::string(e.what()));
        if (e.type() == ServiceError::Type::RESERVATION_CONFLICT) {
            ResponseUtils::error(res, 409, e.what());
        } else if (e.type() == ServiceError::Type::PARKING_SPOT_NOT_FOUND) {
            ResponseUtils::error(res, 404, e.what());
        } else {
            ResponseUtils::error(res, 400, e.what());
        }
    } catch (const std::exception& e) {
        Logger::error("Create reservation exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
    }
}

void ReservationController::get_user_reservations(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取当前用户
        const auto& user_id_str = req.get_header_value("X-Current-User-Id");
        if (user_id_str.empty()) {
            ResponseUtils::error(res, 401, "Unauthorized");
            return;
        }
        int user_id = std::stoi(user_id_str);

        // 调用服务
        std::vector<Reservation> reservations = reservation_service_->get_user_reservations(user_id);

        // 构建响应
        json response_data;
        response_data["reservations"] = reservations;
        ResponseUtils::success(res, 200, "User reservations retrieved successfully", response_data);
    } catch (const std::exception& e) {
        Logger::error("Get user reservations exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
    }
}

void ReservationController::get_owner_reservations(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取当前用户
        const auto& user_id_str = req.get_header_value("X-Current-User-Id");
        if (user_id_str.empty()) {
            ResponseUtils::error(res, 401, "Unauthorized");
            return;
        }
        int user_id = std::stoi(user_id_str);

        // 调用服务
        std::vector<Reservation> reservations = reservation_service_->get_owner_reservations(user_id);

        // 构建响应
        json response_data;
        response_data["reservations"] = reservations;
        ResponseUtils::success(res, 200, "Owner reservations retrieved successfully", response_data);
    } catch (const std::exception& e) {
        Logger::error("Get owner reservations exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
    }
}

void ReservationController::cancel_reservation(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取当前用户
        const auto& user_id_str = req.get_header_value("X-Current-User-Id");
        if (user_id_str.empty()) {
            ResponseUtils::error(res, 401, "Unauthorized");
            return;
        }
        int user_id = std::stoi(user_id_str);
        auto user_opt = user_service_->get_user(user_id);
        if (!user_opt) {
            ResponseUtils::error(res, 401, "User not found");
            return;
        }

        // 获取Reservation ID
        int reservation_id = std::stoi(req.matches[1]);

        // 调用服务
        reservation_service_->cancel_reservation(*user_opt, reservation_id);

        ResponseUtils::success(res, 200, "Reservation cancelled successfully", {});
    } catch (const ServiceError& e) {
        Logger::error("Cancel reservation error: " + std::string(e.what()));
        if (e.type() == ServiceError::Type::PERMISSION_DENIED) {
            ResponseUtils::error(res, 403, e.what());
        } else if (e.type() == ServiceError::Type::RESERVATION_NOT_FOUND) {
            ResponseUtils::error(res, 404, e.what());
        } else {
            ResponseUtils::error(res, 400, e.what());
        }
    } catch (const std::exception& e) {
        Logger::error("Cancel reservation exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
    }
}

void ReservationController::finish_reservation(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取当前用户
        const auto& user_id_str = req.get_header_value("X-Current-User-Id");
        if (user_id_str.empty()) {
            ResponseUtils::error(res, 401, "Unauthorized");
            return;
        }
        int user_id = std::stoi(user_id_str);
        auto user_opt = user_service_->get_user(user_id);
        if (!user_opt) {
            ResponseUtils::error(res, 401, "User not found");
            return;
        }

        // 获取Reservation ID
        int reservation_id = std::stoi(req.matches[1]);

        // 调用服务
        reservation_service_->finish_reservation(*user_opt, reservation_id);

        ResponseUtils::success(res, 200, "Reservation finished successfully", {});
    } catch (const ServiceError& e) {
        Logger::error("Finish reservation error: " + std::string(e.what()));
        if (e.type() == ServiceError::Type::PERMISSION_DENIED) {
            ResponseUtils::error(res, 403, e.what());
        } else if (e.type() == ServiceError::Type::RESERVATION_NOT_FOUND) {
            ResponseUtils::error(res, 404, e.what());
        } else {
            ResponseUtils::error(res, 400, e.what());
        }
    } catch (const std::exception& e) {
        Logger::error("Finish reservation exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
    }
}
