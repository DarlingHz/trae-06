#include "parking/services.h"
#include "parking/utils.h"
#include "parking/config.h"
#include <stdexcept>
#include <string>
#include <cmath>

// ReservationService实现
Reservation ReservationService::create_reservation(const User& renter,
                                                   int spot_id,
                                                   std::time_t start_time,
                                                   std::time_t end_time,
                                                   const std::string& vehicle_plate) {
    // 参数验证
    if (start_time >= end_time) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Start time must be before end time");
    }
    if (vehicle_plate.empty()) {
        throw ServiceError(ServiceError::Type::VALIDATION_ERROR, "Vehicle plate cannot be empty");
    }

    // 查找停车位
    auto spot_opt = parking_spot_dao_->find_by_id(spot_id);
    if (!spot_opt) {
        throw ServiceError(ServiceError::Type::PARKING_SPOT_NOT_FOUND, "Parking spot not found");
    }

    ParkingSpot spot = *spot_opt;

    // 检查停车位状态
    if (spot.status != ParkingSpotStatus::ACTIVE) {
        throw ServiceError(ServiceError::Type::PARKING_SPOT_INACTIVE, "Parking spot is inactive");
    }

    // 检查是否预约自己的车位
    if (spot.owner_user_id == renter.id) {
        throw ServiceError(ServiceError::Type::CANNOT_RESERVE_OWN_SPOT, "Cannot reserve your own parking spot");
    }

    // 检查预约时间是否在车位可用时间段内
    std::tm tm_start;
    localtime_r(&start_time, &tm_start);
    int start_time_minutes = tm_start.tm_hour * 60 + tm_start.tm_min;
    std::tm tm_end;
    localtime_r(&end_time, &tm_end);
    int end_time_minutes = tm_end.tm_hour * 60 + tm_end.tm_min;

    if (start_time_minutes < spot.daily_available_start || end_time_minutes > spot.daily_available_end) {
        throw ServiceError(ServiceError::Type::OUT_OF_AVAILABLE_HOURS, "Reservation time is outside of available hours");
    }

    // 检查预约时长
    const int min_duration_hours = Config::instance().min_reservation_duration_hours();
    const int max_duration_hours = Config::instance().max_reservation_duration_hours();
    std::time_t duration_seconds = end_time - start_time;
    double duration_hours = static_cast<double>(duration_seconds) / 3600.0;

    if (duration_hours < min_duration_hours) {
        throw ServiceError(ServiceError::Type::RESERVATION_TOO_SHORT,
                          "Reservation duration must be at least " + std::to_string(min_duration_hours) + " hour(s)");
    }
    if (duration_hours > max_duration_hours) {
        throw ServiceError(ServiceError::Type::RESERVATION_TOO_LONG,
                          "Reservation duration cannot exceed " + std::to_string(max_duration_hours) + " hour(s)");
    }

    // 创建预约
    Reservation reservation;
    reservation.spot_id = spot_id;
    reservation.renter_user_id = renter.id;
    reservation.owner_user_id = spot.owner_user_id;
    reservation.vehicle_plate = vehicle_plate;
    reservation.start_time = start_time;
    reservation.end_time = end_time;
    reservation.total_price = std::ceil(duration_hours) * spot.price_per_hour;
    reservation.status = ReservationStatus::PENDING;
    reservation.created_at = std::time(nullptr);
    reservation.updated_at = reservation.created_at;

    try {
        reservation.id = reservation_dao_->create(reservation);
        return reservation;
    } catch (const std::runtime_error& e) {
        if (std::string(e.what()).find("conflict") != std::string::npos) {
            throw ServiceError(ServiceError::Type::RESERVATION_CONFLICT, "Reservation time conflict");
        }
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to create reservation: " + std::string(e.what()));
    }
}

std::optional<Reservation> ReservationService::get_reservation(int id) const {
    try {
        return reservation_dao_->find_by_id(id);
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to get reservation: " + std::string(e.what()));
    }
}

std::vector<Reservation> ReservationService::get_user_reservations(int user_id) const {
    try {
        return reservation_dao_->find_by_renter(user_id);
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to get user reservations: " + std::string(e.what()));
    }
}

std::vector<Reservation> ReservationService::get_owner_reservations(int user_id) const {
    try {
        return reservation_dao_->find_by_owner(user_id);
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to get owner reservations: " + std::string(e.what()));
    }
}

void ReservationService::cancel_reservation(const User& current_user, int reservation_id) {
    // 查找预约
    auto reservation_opt = get_reservation(reservation_id);
    if (!reservation_opt) {
        throw ServiceError(ServiceError::Type::RESERVATION_NOT_FOUND, "Reservation not found");
    }

    Reservation reservation = *reservation_opt;

    // 检查状态
    if (reservation.status == ReservationStatus::CANCELLED) {
        throw ServiceError(ServiceError::Type::RESERVATION_ALREADY_CANCELLED, "Reservation is already cancelled");
    }
    if (reservation.status == ReservationStatus::FINISHED) {
        throw ServiceError(ServiceError::Type::RESERVATION_ALREADY_FINISHED, "Reservation is already finished");
    }

    // 检查权限（只能取消自己的预约，或拥有者取消）
    if (reservation.renter_user_id != current_user.id && reservation.owner_user_id != current_user.id) {
        throw ServiceError(ServiceError::Type::PERMISSION_DENIED, "Permission denied");
    }

    // 更新状态
    reservation.status = ReservationStatus::CANCELLED;
    reservation.updated_at = std::time(nullptr);

    try {
        reservation_dao_->update_status(reservation.id, reservation.status);
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to cancel reservation: " + std::string(e.what()));
    }
}

void ReservationService::finish_reservation(const User& owner, int reservation_id) {
    // 查找预约
    auto reservation_opt = get_reservation(reservation_id);
    if (!reservation_opt) {
        throw ServiceError(ServiceError::Type::RESERVATION_NOT_FOUND, "Reservation not found");
    }

    Reservation reservation = *reservation_opt;

    // 检查状态
    if (reservation.status == ReservationStatus::CANCELLED) {
        throw ServiceError(ServiceError::Type::RESERVATION_ALREADY_CANCELLED, "Reservation is already cancelled");
    }
    if (reservation.status == ReservationStatus::FINISHED) {
        throw ServiceError(ServiceError::Type::RESERVATION_ALREADY_FINISHED, "Reservation is already finished");
    }

    // 检查权限（只能由车位拥有者完成）
    if (reservation.owner_user_id != owner.id) {
        throw ServiceError(ServiceError::Type::PERMISSION_DENIED, "Permission denied");
    }

    // 更新状态
    reservation.status = ReservationStatus::FINISHED;
    reservation.updated_at = std::time(nullptr);

    try {
        reservation_dao_->update_status(reservation.id, reservation.status);
    } catch (const std::exception& e) {
        throw ServiceError(ServiceError::Type::DATABASE_ERROR, "Failed to finish reservation: " + std::string(e.what()));
    }
}
