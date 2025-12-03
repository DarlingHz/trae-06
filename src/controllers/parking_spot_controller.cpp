#include "parking/controllers.h"
#include "parking/services.h"
#include "parking/models.h"
#include "parking/utils.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <string>
#include <regex>

using json = nlohmann::json;

// ParkingSpotController实现
void ParkingSpotController::create_spot(const httplib::Request& req, httplib::Response& res) {
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
        if (!data.contains("title") || !data["title"].is_string() || data["title"].get<std::string>().empty()) {
            ResponseUtils::error(res, 400, "Invalid title");
            return;
        }
        if (!data.contains("address") || !data["address"].is_string() || data["address"].get<std::string>().empty()) {
            ResponseUtils::error(res, 400, "Invalid address");
            return;
        }
        if (!data.contains("price_per_hour") || !data["price_per_hour"].is_number()) {
            ResponseUtils::error(res, 400, "Invalid price_per_hour");
            return;
        }
        if (!data.contains("daily_available_start") || !data["daily_available_start"].is_string()) {
            ResponseUtils::error(res, 400, "Invalid daily_available_start");
            return;
        }
        if (!data.contains("daily_available_end") || !data["daily_available_end"].is_string()) {
            ResponseUtils::error(res, 400, "Invalid daily_available_end");
            return;
        }

        std::string title = data["title"].get<std::string>();
        std::string address = data["address"].get<std::string>();
        double price_per_hour = data["price_per_hour"].get<double>();
        int daily_available_start = parse_time_hhmm(data["daily_available_start"].get<std::string>());
        int daily_available_end = parse_time_hhmm(data["daily_available_end"].get<std::string>());

        double latitude = 0.0;
        double longitude = 0.0;
        if (data.contains("latitude") && data["latitude"].is_number()) {
            latitude = data["latitude"].get<double>();
        }
        if (data.contains("longitude") && data["longitude"].is_number()) {
            longitude = data["longitude"].get<double>();
        }

        // 调用服务
        ParkingSpot spot = parking_spot_service_->create_spot(*user_opt, title, address, latitude, longitude,
                                                            price_per_hour, daily_available_start, daily_available_end);

        // 构建响应
        json response_data;
        response_data["spot"] = spot;
        ResponseUtils::success(res, 201, "Parking spot created successfully", response_data);
    } catch (const ServiceError& e) {
        Logger::error("Create parking spot error: " + std::string(e.what()));
        if (e.type() == ServiceError::Type::PERMISSION_DENIED) {
            ResponseUtils::error(res, 403, e.what());
        } else {
            ResponseUtils::error(res, 400, e.what());
        }
    } catch (const std::exception& e) {
        Logger::error("Create parking spot exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
    }
}

void ParkingSpotController::get_user_spots(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取当前用户
        const auto& user_id_str = req.get_header_value("X-Current-User-Id");
        if (user_id_str.empty()) {
            ResponseUtils::error(res, 401, "Unauthorized");
            return;
        }
        int user_id = std::stoi(user_id_str);

        // 调用服务
        std::vector<ParkingSpot> spots = parking_spot_service_->get_user_spots(user_id);

        // 构建响应
        json response_data;
        response_data["spots"] = spots;
        ResponseUtils::success(res, 200, "User parking spots retrieved successfully", response_data);
    } catch (const std::exception& e) {
        Logger::error("Get user spots exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
    }
}

void ParkingSpotController::update_spot(const httplib::Request& req, httplib::Response& res) {
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

        // 获取Spot ID
        int spot_id = std::stoi(req.matches[1]);

        // 解析请求
        auto data = json::parse(req.body);

        // 验证参数
        if (!data.contains("title") || !data["title"].is_string() || data["title"].get<std::string>().empty()) {
            ResponseUtils::error(res, 400, "Invalid title");
            return;
        }
        if (!data.contains("address") || !data["address"].is_string() || data["address"].get<std::string>().empty()) {
            ResponseUtils::error(res, 400, "Invalid address");
            return;
        }
        if (!data.contains("price_per_hour") || !data["price_per_hour"].is_number()) {
            ResponseUtils::error(res, 400, "Invalid price_per_hour");
            return;
        }
        if (!data.contains("daily_available_start") || !data["daily_available_start"].is_string()) {
            ResponseUtils::error(res, 400, "Invalid daily_available_start");
            return;
        }
        if (!data.contains("daily_available_end") || !data["daily_available_end"].is_string()) {
            ResponseUtils::error(res, 400, "Invalid daily_available_end");
            return;
        }

        std::string title = data["title"].get<std::string>();
        std::string address = data["address"].get<std::string>();
        double price_per_hour = data["price_per_hour"].get<double>();
        int daily_available_start = parse_time_hhmm(data["daily_available_start"].get<std::string>());
        int daily_available_end = parse_time_hhmm(data["daily_available_end"].get<std::string>());

        double latitude = 0.0;
        double longitude = 0.0;
        if (data.contains("latitude") && data["latitude"].is_number()) {
            latitude = data["latitude"].get<double>();
        }
        if (data.contains("longitude") && data["longitude"].is_number()) {
            longitude = data["longitude"].get<double>();
        }

        // 调用服务
        ParkingSpot spot = parking_spot_service_->update_spot(*user_opt, spot_id, title, address, latitude, longitude,
                                                           price_per_hour, daily_available_start, daily_available_end);

        // 构建响应
        json response_data;
        response_data["spot"] = spot;
        ResponseUtils::success(res, 200, "Parking spot updated successfully", response_data);
    } catch (const ServiceError& e) {
        Logger::error("Update parking spot error: " + std::string(e.what()));
        if (e.type() == ServiceError::Type::PERMISSION_DENIED) {
            ResponseUtils::error(res, 403, e.what());
        } else if (e.type() == ServiceError::Type::PARKING_SPOT_NOT_FOUND) {
            ResponseUtils::error(res, 404, e.what());
        } else {
            ResponseUtils::error(res, 400, e.what());
        }
    } catch (const std::exception& e) {
        Logger::error("Update parking spot exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
    }
}

void ParkingSpotController::deactivate_spot(const httplib::Request& req, httplib::Response& res) {
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

        // 获取Spot ID
        int spot_id = std::stoi(req.matches[1]);

        // 调用服务
        parking_spot_service_->deactivate_spot(*user_opt, spot_id);

        ResponseUtils::success(res, 200, "Parking spot deactivated successfully", {});
    } catch (const ServiceError& e) {
        Logger::error("Deactivate parking spot error: " + std::string(e.what()));
        if (e.type() == ServiceError::Type::PERMISSION_DENIED) {
            ResponseUtils::error(res, 403, e.what());
        } else if (e.type() == ServiceError::Type::PARKING_SPOT_NOT_FOUND) {
            ResponseUtils::error(res, 404, e.what());
        } else {
            ResponseUtils::error(res, 400, e.what());
        }
    } catch (const std::exception& e) {
        Logger::error("Deactivate parking spot exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
    }
}

void ParkingSpotController::search_spots(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取查询参数
        std::string city = req.get_param_value("city");
        auto start_time_str = req.get_param_value("start_time");
        auto end_time_str = req.get_param_value("end_time");

        if (start_time_str.empty()) {
            ResponseUtils::error(res, 400, "Missing start_time parameter");
            return;
        }
        if (end_time_str.empty()) {
            ResponseUtils::error(res, 400, "Missing end_time parameter");
            return;
        }

        // 解析时间
        std::time_t start_time = std::stoll(start_time_str);
        std::time_t end_time = std::stoll(end_time_str);

        // 调用服务
        std::vector<ParkingSpot> spots = parking_spot_service_->search_spots(city, start_time, end_time);

        // 构建响应
        json response_data;
        response_data["spots"] = spots;
        ResponseUtils::success(res, 200, "Parking spots retrieved successfully", response_data);
    } catch (const ServiceError& e) {
        Logger::error("Search parking spots error: " + std::string(e.what()));
        ResponseUtils::error(res, 400, e.what());
    } catch (const std::exception& e) {
        Logger::error("Search parking spots exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
    }
}

void ParkingSpotController::get_spot(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取Spot ID
        int spot_id = std::stoi(req.matches[1]);

        // 调用服务
        auto spot_opt = parking_spot_service_->get_spot(spot_id);
        if (!spot_opt) {
            ResponseUtils::error(res, 404, "Parking spot not found");
            return;
        }

        // 构建响应
        json response_data;
        response_data["spot"] = *spot_opt;
        ResponseUtils::success(res, 200, "Parking spot retrieved successfully", response_data);
    } catch (const std::exception& e) {
        Logger::error("Get parking spot exception: " + std::string(e.what()));
        ResponseUtils::error(res, 500, "Internal server error");
    }
}
