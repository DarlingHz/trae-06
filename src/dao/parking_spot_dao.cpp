#include "parking/dao.h"
#include "parking/database.h"
#include "parking/models.h"
#include <stdexcept>
#include <vector>

// SQLiteParkingSpotDAO实现
int SQLiteParkingSpotDAO::create(const ParkingSpot& spot) {
    try {
        db::get().execute(
            "INSERT INTO parking_spots (owner_user_id, title, address, latitude, longitude, "
            "price_per_hour, daily_available_start, daily_available_end, status, created_at, updated_at) "
            "VALUES (" 
            + std::to_string(spot.owner_user_id) + ", "
            "'" + spot.title + "', "
            "'" + spot.address + "', "
            + std::to_string(spot.latitude) + ", "
            + std::to_string(spot.longitude) + ", "
            + std::to_string(spot.price_per_hour) + ", "
            "'" + spot.daily_available_start + "', "
            "'" + spot.daily_available_end + "', "
            "'" + to_string(spot.status) + "', "
            + std::to_string(spot.created_at) + ", "
            + std::to_string(spot.updated_at) + ")"
        );
        return static_cast<int>(db::get().last_insert_rowid());
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create parking spot: " + std::string(e.what()));
    }
}

std::optional<ParkingSpot> SQLiteParkingSpotDAO::find_by_id(int id) const {
    ParkingSpot spot;
    bool found = false;

    try {
        db::get().query(
            "SELECT id, owner_user_id, title, address, latitude, longitude, price_per_hour, "
            "daily_available_start, daily_available_end, status, created_at, updated_at "
            "FROM parking_spots WHERE id = " + std::to_string(id),
            [&](int argc, char** argv, char** azColName) -> int {
                if (argc >= 12) {
                    spot.id = std::stoi(argv[0]);
                    spot.owner_user_id = std::stoi(argv[1]);
                    spot.title = argv[2];
                    spot.address = argv[3];
                    spot.latitude = std::stod(argv[4]);
                    spot.longitude = std::stod(argv[5]);
                    spot.price_per_hour = std::stod(argv[6]);
                    spot.daily_available_start = argv[7];
                    spot.daily_available_end = argv[8];
                    spot.status = parking_spot_status_from_string(argv[9]);
                    spot.created_at = std::stoll(argv[10]);
                    spot.updated_at = std::stoll(argv[11]);
                    found = true;
                }
                return 0;
            }
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find parking spot by id: " + std::string(e.what()));
    }

    return found ? std::optional<ParkingSpot>(spot) : std::nullopt;
}

std::vector<ParkingSpot> SQLiteParkingSpotDAO::find_by_owner(int user_id) const {
    std::vector<ParkingSpot> spots;

    try {
        db::get().query(
            "SELECT id, owner_user_id, title, address, latitude, longitude, price_per_hour, "
            "daily_available_start, daily_available_end, status, created_at, updated_at "
            "FROM parking_spots WHERE owner_user_id = " + std::to_string(user_id),
            [&](int argc, char** argv, char** azColName) -> int {
                if (argc >= 12) {
                    ParkingSpot spot;
                    spot.id = std::stoi(argv[0]);
                    spot.owner_user_id = std::stoi(argv[1]);
                    spot.title = argv[2];
                    spot.address = argv[3];
                    spot.latitude = std::stod(argv[4]);
                    spot.longitude = std::stod(argv[5]);
                    spot.price_per_hour = std::stod(argv[6]);
                    spot.daily_available_start = argv[7];
                    spot.daily_available_end = argv[8];
                    spot.status = parking_spot_status_from_string(argv[9]);
                    spot.created_at = std::stoll(argv[10]);
                    spot.updated_at = std::stoll(argv[11]);
                    spots.push_back(spot);
                }
                return 0;
            }
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find parking spots by owner: " + std::string(e.what()));
    }

    return spots;
}

std::vector<ParkingSpot> SQLiteParkingSpotDAO::search_available(
    const std::string& city, std::time_t start_time, std::time_t end_time) const {
    std::vector<ParkingSpot> spots;

    std::string query = 
        "SELECT DISTINCT ps.id, ps.owner_user_id, ps.title, ps.address, ps.latitude, ps.longitude, "
        "ps.price_per_hour, ps.daily_available_start, ps.daily_available_end, ps.status, "
        "ps.created_at, ps.updated_at "
        "FROM parking_spots ps "
        "WHERE ps.status = 'active' "
        "AND NOT EXISTS (" 
        "    SELECT 1 FROM reservations r "
        "    WHERE r.spot_id = ps.id "
        "    AND r.status IN ('pending', 'confirmed') "
        "    AND r.start_time < " + std::to_string(end_time) + " "
        "    AND r.end_time > " + std::to_string(start_time) + " "
        ")";

    if (!city.empty()) {
        query += " AND ps.address LIKE '%" + city + "%'";
    }

    try {
        db::get().query(query,
            [&](int argc, char** argv, char** azColName) -> int {
                if (argc >= 12) {
                    ParkingSpot spot;
                    spot.id = std::stoi(argv[0]);
                    spot.owner_user_id = std::stoi(argv[1]);
                    spot.title = argv[2];
                    spot.address = argv[3];
                    spot.latitude = std::stod(argv[4]);
                    spot.longitude = std::stod(argv[5]);
                    spot.price_per_hour = std::stod(argv[6]);
                    spot.daily_available_start = argv[7];
                    spot.daily_available_end = argv[8];
                    spot.status = parking_spot_status_from_string(argv[9]);
                    spot.created_at = std::stoll(argv[10]);
                    spot.updated_at = std::stoll(argv[11]);
                    spots.push_back(spot);
                }
                return 0;
            }
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to search available parking spots: " + std::string(e.what()));
    }

    return spots;
}

bool SQLiteParkingSpotDAO::update(const ParkingSpot& spot) {
    try {
        db::get().execute(
            "UPDATE parking_spots SET "
            "title = '" + spot.title + "', "
            "address = '" + spot.address + "', "
            "latitude = " + std::to_string(spot.latitude) + ", "
            "longitude = " + std::to_string(spot.longitude) + ", "
            "price_per_hour = " + std::to_string(spot.price_per_hour) + ", "
            "daily_available_start = '" + spot.daily_available_start + "', "
            "daily_available_end = '" + spot.daily_available_end + "', "
            "status = '" + to_string(spot.status) + "', "
            "updated_at = " + std::to_string(spot.updated_at) + " "
            "WHERE id = " + std::to_string(spot.id)
        );
        return sqlite3_changes(db::get().get_native_handle()) > 0;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to update parking spot: " + std::string(e.what()));
    }
}

bool SQLiteParkingSpotDAO::is_available(int spot_id, std::time_t start_time, std::time_t end_time) const {
    bool available = true;

    try {
        db::get().query(
            "SELECT 1 FROM reservations "
            "WHERE spot_id = " + std::to_string(spot_id) + " "
            "AND status IN ('pending', 'confirmed') "
            "AND start_time < " + std::to_string(end_time) + " "
            "AND end_time > " + std::to_string(start_time),
            [&](int argc, char** argv, char** azColName) -> int {
                available = false;
                return 1; // 停止查询
            }
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to check parking spot availability: " + std::string(e.what()));
    }

    return available;
}
