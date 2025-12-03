#include "parking/models.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <string>

// UserStatus转换
std::string to_string(UserStatus status) {
    switch (status) {
        case UserStatus::ACTIVE: return "active";
        case UserStatus::INACTIVE: return "inactive";
        default: throw std::invalid_argument("Invalid UserStatus");
    }
}

UserStatus user_status_from_string(const std::string& str) {
    if (str == "active") return UserStatus::ACTIVE;
    if (str == "inactive") return UserStatus::INACTIVE;
    throw std::invalid_argument("Invalid UserStatus string: " + str);
}

// ParkingSpotStatus转换
std::string to_string(ParkingSpotStatus status) {
    switch (status) {
        case ParkingSpotStatus::ACTIVE: return "active";
        case ParkingSpotStatus::INACTIVE: return "inactive";
        default: throw std::invalid_argument("Invalid ParkingSpotStatus");
    }
}

ParkingSpotStatus parking_spot_status_from_string(const std::string& str) {
    if (str == "active") return ParkingSpotStatus::ACTIVE;
    if (str == "inactive") return ParkingSpotStatus::INACTIVE;
    throw std::invalid_argument("Invalid ParkingSpotStatus string: " + str);
}

// ReservationStatus转换
std::string to_string(ReservationStatus status) {
    switch (status) {
        case ReservationStatus::PENDING: return "pending";
        case ReservationStatus::CONFIRMED: return "confirmed";
        case ReservationStatus::CANCELLED: return "cancelled";
        case ReservationStatus::FINISHED: return "finished";
        default: throw std::invalid_argument("Invalid ReservationStatus");
    }
}

ReservationStatus reservation_status_from_string(const std::string& str) {
    if (str == "pending") return ReservationStatus::PENDING;
    if (str == "confirmed") return ReservationStatus::CONFIRMED;
    if (str == "cancelled") return ReservationStatus::CANCELLED;
    if (str == "finished") return ReservationStatus::FINISHED;
    throw std::invalid_argument("Invalid ReservationStatus string: " + str);
}

// 时间格式化
std::string format_time(std::time_t timestamp, const std::string& format) {
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &timestamp);
#else
    localtime_r(&timestamp, &tm_buf);
#endif

    char buf[256];
    if (strftime(buf, sizeof(buf), format.c_str(), &tm_buf) == 0) {
        throw std::runtime_error("Failed to format time");
    }
    return std::string(buf);
}

// 时间解析
std::time_t parse_time(const std::string& time_str, const std::string& format) {
    std::tm tm_buf = {};
    std::istringstream ss(time_str);
    ss >> std::get_time(&tm_buf, format.c_str());

    if (ss.fail()) {
        throw std::invalid_argument("Invalid time format: " + time_str);
    }

    return mktime(&tm_buf);
}

// 解析HH:MM格式
std::time_t parse_time_hhmm(const std::string& time_str) {
    std::tm tm_buf = {};
    std::istringstream ss(time_str);
    int hours, minutes;
    char colon;

    if (!(ss >> hours >> colon >> minutes) || colon != ':' || 
        hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
        throw std::invalid_argument("Invalid time format (HH:MM expected): " + time_str);
    }

    // 设置为今天的指定时间
    std::time_t now = std::time(nullptr);
#ifdef _WIN32
    localtime_s(&tm_buf, &now);
#else
    localtime_r(&now, &tm_buf);
#endif

    tm_buf.tm_hour = hours;
    tm_buf.tm_min = minutes;
    tm_buf.tm_sec = 0;

    return mktime(&tm_buf);
}

// JSON序列化特化
namespace nlohmann {
    template<> struct adl_serializer<UserStatus> {
        static void to_json(json& j, UserStatus s) {
            j = to_string(s);
        }

        static void from_json(const json& j, UserStatus& s) {
            s = user_status_from_string(j.get<std::string>());
        }
    };

    template<> struct adl_serializer<ParkingSpotStatus> {
        static void to_json(json& j, ParkingSpotStatus s) {
            j = to_string(s);
        }

        static void from_json(const json& j, ParkingSpotStatus& s) {
            s = parking_spot_status_from_string(j.get<std::string>());
        }
    };

    template<> struct adl_serializer<ReservationStatus> {
        static void to_json(json& j, ReservationStatus s) {
            j = to_string(s);
        }

        static void from_json(const json& j, ReservationStatus& s) {
            s = reservation_status_from_string(j.get<std::string>());
        }
    };
}
