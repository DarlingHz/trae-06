#ifndef PARKING_SERVICES_H
#define PARKING_SERVICES_H

#include "models.h"
#include "dao.h"
#include <vector>
#include <optional>
#include <string>
#include <stdexcept>

// 错误定义
class ServiceError : public std::runtime_error {
public:
    enum class Type {
        USER_NOT_FOUND,
        USER_ALREADY_EXISTS,
        INVALID_CREDENTIALS,
        INVALID_TOKEN,
        TOKEN_EXPIRED,
        SPOT_NOT_FOUND,
        SPOT_NOT_OWNED,
        SPOT_NOT_AVAILABLE,
        RESERVATION_NOT_FOUND,
        RESERVATION_NOT_AUTHORIZED,
        RESERVATION_STATUS_INVALID,
        TIME_CONFLICT,
        INVALID_TIME_RANGE,
        VALIDATION_ERROR,
        DATABASE_ERROR
    };

    ServiceError(Type type, const std::string& message)
        : std::runtime_error(message), type_(type) {}

    Type type() const { return type_; }

private:
    Type type_;
};

// 用户服务
class UserService {
private:
    std::unique_ptr<UserDAO> user_dao_;
    std::unique_ptr<SessionDAO> session_dao_;

public:
    UserService(std::unique_ptr<UserDAO> user_dao, std::unique_ptr<SessionDAO> session_dao)
        : user_dao_(std::move(user_dao)), session_dao_(std::move(session_dao)) {}

    // 用户注册
    User register_user(const std::string& name, const std::string& email, const std::string& password);

    // 用户登录
    std::pair<User, std::string> login(const std::string& email, const std::string& password);

    // 验证Token
    std::optional<User> validate_token(const std::string& token);

    // 登出
    void logout(const std::string& token);

    // 根据ID获取用户
    std::optional<User> get_user(int id) const;
};

// 停车位服务
class ParkingSpotService {
private:
    std::unique_ptr<ParkingSpotDAO> spot_dao_;

public:
    ParkingSpotService(std::unique_ptr<ParkingSpotDAO> spot_dao)
        : spot_dao_(std::move(spot_dao)) {}

    // 创建停车位
    ParkingSpot create_spot(const ParkingSpot& spot);

    // 获取用户的所有停车位
    std::vector<ParkingSpot> get_user_spots(int user_id) const;

    // 获取停车位详情
    std::optional<ParkingSpot> get_spot(int id) const;

    // 更新停车位
    bool update_spot(const ParkingSpot& spot, int owner_id);

    // 禁用停车位（软删除）
    bool deactivate_spot(int id, int owner_id);

    // 搜索可用停车位
    std::vector<ParkingSpot> search_available_spots(
        const std::string& city, const std::string& start_time_str, const std::string& end_time_str) const;
};

// 预约服务
class ReservationService {
private:
    std::unique_ptr<ReservationDAO> reservation_dao_;
    std::unique_ptr<ParkingSpotDAO> spot_dao_;

    // 计算价格
    double calculate_price(const ParkingSpot& spot, std::time_t start_time, std::time_t end_time) const;

public:
    ReservationService(std::unique_ptr<ReservationDAO> reservation_dao, std::unique_ptr<ParkingSpotDAO> spot_dao)
        : reservation_dao_(std::move(reservation_dao)), spot_dao_(std::move(spot_dao)) {}

    // 创建预约
    Reservation create_reservation(int renter_id, int spot_id,
        const std::string& start_time_str, const std::string& end_time_str,
        const std::string& vehicle_plate);

    // 获取用户的所有预约（作为租客）
    std::vector<Reservation> get_user_reservations(int renter_id) const;

    // 获取用户收到的所有预约（作为车主）
    std::vector<Reservation> get_owner_reservations(int owner_id) const;

    // 获取预约详情
    std::optional<Reservation> get_reservation(int id) const;

    // 取消预约
    bool cancel_reservation(int id, int user_id, bool is_owner);

    // 完成预约
    bool finish_reservation(int id, int owner_id);
};

#endif // PARKING_SERVICES_H
