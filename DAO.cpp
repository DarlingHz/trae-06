#include "DAO.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <chrono>

DAO::DAO() {
}

DAO& DAO::getInstance() {
    static DAO instance;
    return instance;
}

int DAO::createUser(const std::string& nickname) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    if (!db) {
        return -1;
    }

    std::string sql = "INSERT INTO users (nickname) VALUES (?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    sqlite3_bind_text(stmt, 1, nickname.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }

    int userId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    return userId;
}

std::optional<User> DAO::getUserById(int userId) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    if (!db) {
        return std::nullopt;
    }

    std::string sql = "SELECT user_id, nickname, created_at FROM users WHERE user_id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, userId);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* nickname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        sqlite3_finalize(stmt);
        return User(id, nickname ? nickname : "", createdAt ? createdAt : "");
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

bool DAO::existsUser(int userId) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    if (!db) {
        return false;
    }

    std::string sql = "SELECT 1 FROM users WHERE user_id = ? LIMIT 1;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, userId);
    rc = sqlite3_step(stmt);
    bool exists = (rc == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

int DAO::createStation(const std::string& name, double latitude, double longitude, int capacity) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    if (!db) {
        return -1;
    }

    std::string sql = "INSERT INTO stations (name, latitude, longitude, capacity, available_bikes) VALUES (?, ?, ?, ?, 0);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, latitude);
    sqlite3_bind_double(stmt, 3, longitude);
    sqlite3_bind_int(stmt, 4, capacity);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }

    int stationId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    return stationId;
}

bool DAO::updateStation(int stationId, const std::string& name, double latitude, 
    double longitude, int capacity) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    if (!db) {
        return false;
    }

    std::stringstream ss;
    ss << "UPDATE stations SET";
    bool hasUpdate = false;

    if (!name.empty()) {
        if (hasUpdate) ss << ",";
        ss << " name = ?";
        hasUpdate = true;
    }
    if (latitude != 0 || longitude != 0) {
        if (hasUpdate) ss << ",";
        ss << " latitude = ?, longitude = ?";
        hasUpdate = true;
    }
    if (capacity != 0) {
        if (hasUpdate) ss << ",";
        ss << " capacity = ?";
        hasUpdate = true;
    }

    if (!hasUpdate) {
        return true;
    }

    ss << " WHERE station_id = ?;";
    std::string sql = ss.str();

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    int paramIndex = 1;
    if (!name.empty()) {
        sqlite3_bind_text(stmt, paramIndex++, name.c_str(), -1, SQLITE_STATIC);
    }
    if (latitude != 0 || longitude != 0) {
        sqlite3_bind_double(stmt, paramIndex++, latitude);
        sqlite3_bind_double(stmt, paramIndex++, longitude);
    }
    if (capacity != 0) {
        sqlite3_bind_int(stmt, paramIndex++, capacity);
    }
    sqlite3_bind_int(stmt, paramIndex++, stationId);

    rc = sqlite3_step(stmt);
    bool success = (rc == SQLITE_DONE);

    if (!success) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}

std::optional<Station> DAO::getStationById(int stationId) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    if (!db) {
        return std::nullopt;
    }

    std::string sql = "SELECT station_id, name, latitude, longitude, capacity, available_bikes, created_at FROM stations WHERE station_id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, stationId);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        double lat = sqlite3_column_double(stmt, 2);
        double lon = sqlite3_column_double(stmt, 3);
        int cap = sqlite3_column_int(stmt, 4);
        int avail = sqlite3_column_int(stmt, 5);
        const char* createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        sqlite3_finalize(stmt);
        return Station(id, name ? name : "", lat, lon, cap, avail, createdAt ? createdAt : "");
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

StationQueryResult DAO::getStations(int page, int pageSize, std::optional<int> minAvailableBikes) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    StationQueryResult result;

    if (!db) {
        return result;
    }

    std::stringstream countSs;
    countSs << "SELECT COUNT(*) FROM stations WHERE 1=1";
    if (minAvailableBikes) {
        countSs << " AND available_bikes >= " << minAvailableBikes.value();
    }

    sqlite3_stmt* countStmt;
    int rc = sqlite3_prepare_v2(db, countSs.str().c_str(), -1, &countStmt, nullptr);
    if (rc == SQLITE_OK) {
        rc = sqlite3_step(countStmt);
        if (rc == SQLITE_ROW) {
            result.pagination.totalItems = sqlite3_column_int(countStmt, 0);
        }
        sqlite3_finalize(countStmt);
    }

    result.pagination.page = page;
    result.pagination.pageSize = pageSize;
    result.pagination.totalPages = (result.pagination.totalItems + pageSize - 1) / pageSize;

    std::stringstream ss;
    ss << "SELECT station_id, name, latitude, longitude, capacity, available_bikes, created_at FROM stations WHERE 1=1";
    if (minAvailableBikes) {
        ss << " AND available_bikes >= " << minAvailableBikes.value();
    }
    ss << " ORDER BY created_at DESC LIMIT ? OFFSET ?;";

    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, ss.str().c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return result;
    }

    sqlite3_bind_int(stmt, 1, pageSize);
    sqlite3_bind_int(stmt, 2, (page - 1) * pageSize);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        double lat = sqlite3_column_double(stmt, 2);
        double lon = sqlite3_column_double(stmt, 3);
        int cap = sqlite3_column_int(stmt, 4);
        int avail = sqlite3_column_int(stmt, 5);
        const char* createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        result.stations.emplace_back(id, name ? name : "", lat, lon, cap, avail, createdAt ? createdAt : "");
    }

    sqlite3_finalize(stmt);
    return result;
}

bool DAO::updateStationAvailableBikes(int stationId, int delta) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    if (!db) {
        return false;
    }

    std::string sql = "UPDATE stations SET available_bikes = available_bikes + ? WHERE station_id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, delta);
    sqlite3_bind_int(stmt, 2, stationId);

    rc = sqlite3_step(stmt);
    bool success = (rc == SQLITE_DONE);

    if (!success) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}

int DAO::createBike(int stationId, const std::string& status) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    if (!db) {
        return -1;
    }

    std::string sql = "INSERT INTO bikes (current_station_id, status) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    sqlite3_bind_int(stmt, 1, stationId);
    sqlite3_bind_text(stmt, 2, status.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }

    int bikeId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    return bikeId;
}

bool DAO::updateBike(int bikeId, const std::optional<int>& stationId, const std::string& status) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    if (!db) {
        return false;
    }

    std::stringstream ss;
    ss << "UPDATE bikes SET";
    bool hasUpdate = false;

    if (stationId) {
        if (hasUpdate) ss << ",";
        ss << " current_station_id = ?";
        hasUpdate = true;
    } else {
        if (hasUpdate) ss << ",";
        ss << " current_station_id = NULL";
        hasUpdate = true;
    }

    if (!status.empty()) {
        if (hasUpdate) ss << ",";
        ss << " status = ?";
        hasUpdate = true;
    }

    if (!hasUpdate) {
        return true;
    }

    ss << " WHERE bike_id = ?;";
    std::string sql = ss.str();

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    int paramIndex = 1;
    if (stationId) {
        sqlite3_bind_int(stmt, paramIndex++, stationId.value());
    }
    if (!status.empty()) {
        sqlite3_bind_text(stmt, paramIndex++, status.c_str(), -1, SQLITE_STATIC);
    }
    sqlite3_bind_int(stmt, paramIndex++, bikeId);

    rc = sqlite3_step(stmt);
    bool success = (rc == SQLITE_DONE);

    if (!success) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}

std::optional<Bike> DAO::getBikeById(int bikeId) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    if (!db) {
        return std::nullopt;
    }

    std::string sql = "SELECT bike_id, current_station_id, status, created_at FROM bikes WHERE bike_id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, bikeId);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::optional<int> stationId;
        if (sqlite3_column_type(stmt, 1) != SQLITE_NULL) {
            stationId = sqlite3_column_int(stmt, 1);
        }
        const char* status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        sqlite3_finalize(stmt);
        return Bike(id, stationId, status ? status : "normal", createdAt ? createdAt : "");
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<Bike> DAO::getAvailableBikeAtStation(int stationId) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    if (!db) {
        return std::nullopt;
    }

    std::string sql = "SELECT bike_id, current_station_id, status, created_at FROM bikes WHERE current_station_id = ? AND status = 'normal' LIMIT 1;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, stationId);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::optional<int> sid = sqlite3_column_int(stmt, 1);
        const char* status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        sqlite3_finalize(stmt);
        return Bike(id, sid, status ? status : "normal", createdAt ? createdAt : "");
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

int DAO::startRental(int userId, int stationId, int bikeId) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    if (!db) {
        return -1;
    }

    std::string sql = "INSERT INTO rentals (user_id, bike_id, start_station_id) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, bikeId);
    sqlite3_bind_int(stmt, 3, stationId);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }

    int rentalId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    return rentalId;
}

bool DAO::endRental(int rentalId, int endStationId) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    if (!db) {
        return false;
    }

    std::string getSql = "SELECT start_time FROM rentals WHERE rental_id = ? AND end_time IS NULL;";
    sqlite3_stmt* getStmt;
    int rc = sqlite3_prepare_v2(db, getSql.c_str(), -1, &getStmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(getStmt, 1, rentalId);
    rc = sqlite3_step(getStmt);
    std::string startTime;

    if (rc == SQLITE_ROW) {
        const char* time = reinterpret_cast<const char*>(sqlite3_column_text(getStmt, 0));
        if (time) startTime = time;
    } else {
        sqlite3_finalize(getStmt);
        return false;
    }
    sqlite3_finalize(getStmt);

    std::string currentTime = getCurrentDateTime();
    int fee = calculateFee(startTime, currentTime);

    std::string updateSql = "UPDATE rentals SET end_station_id = ?, end_time = ?, fee = ? WHERE rental_id = ?;";
    sqlite3_stmt* updateStmt;
    rc = sqlite3_prepare_v2(db, updateSql.c_str(), -1, &updateStmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(updateStmt, 1, endStationId);
    sqlite3_bind_text(updateStmt, 2, currentTime.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(updateStmt, 3, fee);
    sqlite3_bind_int(updateStmt, 4, rentalId);

    rc = sqlite3_step(updateStmt);
    bool success = (rc == SQLITE_DONE);

    if (!success) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(updateStmt);
    return success;
}

std::optional<Rental> DAO::getRentalById(int rentalId) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    if (!db) {
        return std::nullopt;
    }

    std::string sql = "SELECT rental_id, user_id, bike_id, start_station_id, end_station_id, start_time, end_time, fee, created_at FROM rentals WHERE rental_id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, rentalId);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        int userId = sqlite3_column_int(stmt, 1);
        int bikeId = sqlite3_column_int(stmt, 2);
        int startStationId = sqlite3_column_int(stmt, 3);
        std::optional<int> endStationId;
        if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
            endStationId = sqlite3_column_int(stmt, 4);
        }
        const char* startTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        std::optional<std::string> endTime;
        if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
            const char* et = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            if (et) endTime = et;
        }
        double fee = sqlite3_column_double(stmt, 7);
        const char* createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        sqlite3_finalize(stmt);
        return Rental(id, userId, bikeId, startStationId, endStationId, startTime ? startTime : "", 
            endTime, fee, createdAt ? createdAt : "");
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<Rental> DAO::getActiveRentalByUserId(int userId) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    if (!db) {
        return std::nullopt;
    }

    std::string sql = "SELECT rental_id, user_id, bike_id, start_station_id, end_station_id, start_time, end_time, fee, created_at FROM rentals WHERE user_id = ? AND end_time IS NULL LIMIT 1;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, userId);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        int uid = sqlite3_column_int(stmt, 1);
        int bikeId = sqlite3_column_int(stmt, 2);
        int startStationId = sqlite3_column_int(stmt, 3);
        std::optional<int> endStationId;
        if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
            endStationId = sqlite3_column_int(stmt, 4);
        }
        const char* startTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        std::optional<std::string> endTime;
        if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
            const char* et = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            if (et) endTime = et;
        }
        double fee = sqlite3_column_double(stmt, 7);
        const char* createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        sqlite3_finalize(stmt);
        return Rental(id, uid, bikeId, startStationId, endStationId, startTime ? startTime : "", 
            endTime, fee, createdAt ? createdAt : "");
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

UserRentalResult DAO::getUserRentals(int userId) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    UserRentalResult result;

    if (!db) {
        return result;
    }

    std::string statsSql = "SELECT COUNT(*), SUM(fee) FROM rentals WHERE user_id = ?;";
    sqlite3_stmt* statsStmt;
    int rc = sqlite3_prepare_v2(db, statsSql.c_str(), -1, &statsStmt, nullptr);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(statsStmt, 1, userId);
        rc = sqlite3_step(statsStmt);
        if (rc == SQLITE_ROW) {
            result.stats.totalRides = sqlite3_column_int(statsStmt, 0);
            result.stats.totalFee = sqlite3_column_double(statsStmt, 1);
        }
        sqlite3_finalize(statsStmt);
    }

    std::string rentalsSql = "SELECT rental_id, user_id, bike_id, start_station_id, end_station_id, start_time, end_time, fee, created_at FROM rentals WHERE user_id = ? ORDER BY start_time DESC;";
    sqlite3_stmt* rentalsStmt;
    rc = sqlite3_prepare_v2(db, rentalsSql.c_str(), -1, &rentalsStmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return result;
    }

    sqlite3_bind_int(rentalsStmt, 1, userId);

    while ((rc = sqlite3_step(rentalsStmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(rentalsStmt, 0);
        int uid = sqlite3_column_int(rentalsStmt, 1);
        int bikeId = sqlite3_column_int(rentalsStmt, 2);
        int startStationId = sqlite3_column_int(rentalsStmt, 3);
        std::optional<int> endStationId;
        if (sqlite3_column_type(rentalsStmt, 4) != SQLITE_NULL) {
            endStationId = sqlite3_column_int(rentalsStmt, 4);
        }
        const char* startTime = reinterpret_cast<const char*>(sqlite3_column_text(rentalsStmt, 5));
        std::optional<std::string> endTime;
        if (sqlite3_column_type(rentalsStmt, 6) != SQLITE_NULL) {
            const char* et = reinterpret_cast<const char*>(sqlite3_column_text(rentalsStmt, 6));
            if (et) endTime = et;
        }
        double fee = sqlite3_column_double(rentalsStmt, 7);
        const char* createdAt = reinterpret_cast<const char*>(sqlite3_column_text(rentalsStmt, 8));
        result.rentals.emplace_back(id, uid, bikeId, startStationId, endStationId, startTime ? startTime : "", 
            endTime, fee, createdAt ? createdAt : "");
    }

    sqlite3_finalize(rentalsStmt);
    return result;
}

std::vector<StationStats> DAO::getTopStations(const std::string& startTime, 
    const std::string& endTime, int limit) {
    sqlite3* db = SQLiteHelper::getInstance().getDB();
    std::vector<StationStats> result;

    if (!db) {
        return result;
    }

    std::stringstream ss;
    ss << "SELECT s.station_id, s.name, COUNT(r.rental_id) as rental_count ";
    ss << "FROM stations s JOIN rentals r ON s.station_id = r.start_station_id ";
    ss << "WHERE r.start_time >= ? AND r.start_time <= ? ";
    ss << "GROUP BY s.station_id ORDER BY rental_count DESC LIMIT ?;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, ss.str().c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return result;
    }

    sqlite3_bind_text(stmt, 1, startTime.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, endTime.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, limit);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int count = sqlite3_column_int(stmt, 2);
        result.emplace_back(id, name ? name : "", count);
    }

    sqlite3_finalize(stmt);
    return result;
}

std::string DAO::getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm localTime;
#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif

    std::stringstream ss;
    ss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "." << std::setw(3) << std::setfill('0') << ms.count();
    return ss.str();
}

int DAO::calculateFee(const std::string& startTime, const std::string& endTime) {
    try {
        std::tm startTm{};
        std::tm endTm{};
        std::istringstream startStream(startTime);
        std::istringstream endStream(endTime);

        startStream >> std::get_time(&startTm, "%Y-%m-%d %H:%M:%S");
        endStream >> std::get_time(&endTm, "%Y-%m-%d %H:%M:%S");

        if (!startStream || !endStream) {
            return 0;
        }

        auto start = std::chrono::system_clock::from_time_t(std::mktime(&startTm));
        auto end = std::chrono::system_clock::from_time_t(std::mktime(&endTm));
        auto diff = std::chrono::duration_cast<std::chrono::minutes>(end - start);
        long long minutes = diff.count();

        if (minutes <= 30) {
            return 0;
        }

        long long extraMinutes = minutes - 30;
        long long fee = (extraMinutes / 15) * 1 + (extraMinutes % 15 > 0 ? 1 : 0);
        return static_cast<int>(fee);
    } catch (...) {
        return 0;
    }
}
