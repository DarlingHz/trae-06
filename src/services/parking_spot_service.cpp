#include "parking/services.h"
#include "parking/utils.h"
#include "parking/config.h"
#include <stdexcept>
#include <string>
#include <regex>

// ParkingSpotService实现
ParkingSpot ParkingSpotService::create_spot(const User& owner,
                                             const std::string& title,
                                             const std::string& address,
                                             double latitude,
                                             double longitude,
                                             double price_per_hour,
                                             int daily_available_start,
                                             int daily_available_end) {
    // 参数验证
    if (title.empty()) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Title cannot be empty");
    }
    if (address.empty()) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Address cannot be empty");
    }
    if (price_per_hour <= 0) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Price per hour must be positive");
    }
    if (daily_available_start < 0 || daily_available_start >= 2400) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Invalid available start time");
    }
    if (daily_available_end < 0 || daily_available_end >= 2400) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Invalid available end time");
    }

    // 创建停车位
    ParkingSpot spot;
    spot.owner_user_id = owner.id;
    spot.title = title;
    spot.address = address;
    spot.latitude = latitude;
    spot.longitude = longitude;
    spot.price_per_hour = price_per_hour;
    spot.daily_available_start = daily_available_start;
    spot.daily_available_end = daily_available_end;
    spot.status = ParkingSpotStatus::ACTIVE;
    spot.created_at = std::time(nullptr);
    spot.updated_at = spot.created_at;

    try {
        spot.id = parking_spot_dao_->create(spot);
        return spot;
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to create parking spot: " + std::string(e.what()));
    }
}

std::optional<ParkingSpot> ParkingSpotService::get_spot(int id) const {
    try {
        return parking_spot_dao_->find_by_id(id);
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to get parking spot: " + std::string(e.what()));
    }
}

std::vector<ParkingSpot> ParkingSpotService::get_user_spots(int user_id) const {
    try {
        return parking_spot_dao_->find_by_owner(user_id);
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to get user's parking spots: " + std::string(e.what()));
    }
}

ParkingSpot ParkingSpotService::update_spot(const User& owner, int spot_id,
                                             const std::string& title,
                                             const std::string& address,
                                             double latitude,
                                             double longitude,
                                             double price_per_hour,
                                             int daily_available_start,
                                             int daily_available_end) {
    // 参数验证
    if (title.empty()) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Title cannot be empty");
    }
    if (address.empty()) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Address cannot be empty");
    }
    if (price_per_hour <= 0) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Price per hour must be positive");
    }
    if (daily_available_start < 0 || daily_available_start >= 2400) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Invalid available start time");
    }
    if (daily_available_end < 0 || daily_available_end >= 2400) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Invalid available end time");
    }

    // 查找停车位
    auto spot_opt = get_spot(spot_id);
    if (!spot_opt) {
        throw ServiceError(ServiceError::Type::PARKING_SPOT_NOT_FOUND, "Parking spot not found");
    }

    ParkingSpot spot = *spot_opt;

    // 检查所有权
    if (spot.owner_user_id != owner.id) {
        throw ServiceError(ServiceError::Type::PERMISSION_DENIED, "Permission denied");
    }

    // 更新信息
    spot.title = title;
    spot.address = address;
    spot.latitude = latitude;
    spot.longitude = longitude;
    spot.price_per_hour = price_per_hour;
    spot.daily_available_start = daily_available_start;
    spot.daily_available_end = daily_available_end;
    spot.updated_at = std::time(nullptr);

    try {
        parking_spot_dao_->update(spot);
        return spot;
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to update parking spot: " + std::string(e.what()));
    }
}

void ParkingSpotService::deactivate_spot(const User& owner, int spot_id) {
    // 查找停车位
    auto spot_opt = get_spot(spot_id);
    if (!spot_opt) {
        throw ServiceError(ServiceError::Type::PARKING_SPOT_NOT_FOUND, "Parking spot not found");
    }

    ParkingSpot spot = *spot_opt;

    // 检查所有权
    if (spot.owner_user_id != owner.id) {
        throw ServiceError(ServiceError::Type::PERMISSION_DENIED, "Permission denied");
    }

    // 软删除：标记为INACTIVE
    spot.status = ParkingSpotStatus::INACTIVE;
    spot.updated_at = std::time(nullptr);

    try {
        parking_spot_dao_->update(spot);
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to deactivate parking spot: " + std::string(e.what()));
    }
}

std::vector<ParkingSpot> ParkingSpotService::search_spots(const std::string& city,
                                                          std::time_t start_time,
                                                          std::time_t end_time) const {
    // 参数验证
    if (start_time >= end_time) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Start time must be before end time");
    }

    try {
        return parking_spot_dao_->search_available(city, start_time, end_time);
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to search parking spots: " + std::string(e.what()));
    }
}
