#ifndef PARKING_MODELS_H
#define PARKING_MODELS_H

#include <string>
#include <ctime>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 用户状态
enum class UserStatus {
    ACTIVE,
    INACTIVE
};

// 停车位状态
enum class ParkingSpotStatus {
    ACTIVE,
    INACTIVE
};

// 预约状态
enum class ReservationStatus {
    PENDING,
    CONFIRMED,
    CANCELLED,
    FINISHED
};

// 用户模型
struct User {
    int id;
    std::string name;
    std::string email;
    std::string password_hash;
    UserStatus status;
    std::time_t created_at;
    std::time_t updated_at;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        User, id, name, email, password_hash, status, created_at, updated_at
    );
};

// 停车位模型
struct ParkingSpot {
    int id;
    int owner_user_id;
    std::string title;
    std::string address;
    double latitude;
    double longitude;
    double price_per_hour;
    std::string daily_available_start;  // "HH:MM"格式
    std::string daily_available_end;    // "HH:MM"格式
    ParkingSpotStatus status;
    std::time_t created_at;
    std::time_t updated_at;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        ParkingSpot, id, owner_user_id, title, address, latitude, longitude,
        price_per_hour, daily_available_start, daily_available_end, status,
        created_at, updated_at
    );
};

// 预约模型
struct Reservation {
    int id;
    int spot_id;
    int renter_user_id;
    std::string vehicle_plate;
    std::time_t start_time;
    std::time_t end_time;
    double total_price;
    ReservationStatus status;
    std::time_t created_at;
    std::time_t updated_at;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        Reservation, id, spot_id, renter_user_id, vehicle_plate, start_time,
        end_time, total_price, status, created_at, updated_at
    );
};

// Token会话模型
struct Session {
    std::string token;
    int user_id;
    std::time_t expires_at;
    std::time_t created_at;
};

// 辅助函数：枚举类型与字符串转换
std::string to_string(UserStatus status);
UserStatus user_status_from_string(const std::string& str);

std::string to_string(ParkingSpotStatus status);
ParkingSpotStatus parking_spot_status_from_string(const std::string& str);

std::string to_string(ReservationStatus status);
ReservationStatus reservation_status_from_string(const std::string& str);

// 辅助函数：时间戳格式化
std::string format_time(std::time_t timestamp, const std::string& format = "%Y-%m-%d %H:%M:%S");
std::time_t parse_time(const std::string& time_str, const std::string& format = "%Y-%m-%d %H:%M:%S");
std::time_t parse_time_hhmm(const std::string& time_str);  // 解析"HH:MM"格式

#endif // PARKING_MODELS_H
