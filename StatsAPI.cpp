#include "StatsAPI.h"
#include <sstream>
#include <iomanip>

std::string StatsAPI::stationStatsToJson(const StationStats& stats) {
    std::stringstream ss;
    ss << "{";
    ss << ToJson("station_id", stats.stationId) << ",";
    ss << ToJson("name", stats.name) << ",";
    ss << ToJson("rental_count", stats.rentalCount);
    ss << "}";
    return ss.str();
}

HttpResponse StatsAPI::getTopStations(const HttpRequest& request) {
    try {
        std::string startTime;
        std::string endTime;
        int limit = 10;

        if (request.queryParams.count("start_time")) {
            startTime = request.queryParams.at("start_time");
        }
        if (request.queryParams.count("end_time")) {
            endTime = request.queryParams.at("end_time");
        }
        if (request.queryParams.count("limit")) {
            limit = std::stoi(request.queryParams.at("limit"));
        }

        if (startTime.empty() || endTime.empty()) {
            return {400, CreateErrorResponse(400, "start_time and end_time are required")};
        }

        std::stringstream cacheKeyStream;
        cacheKeyStream << "top_stations_" << startTime << "_" << endTime << "_" << limit;
        std::string cacheKey = cacheKeyStream.str();

        auto cached = Cache::getInstance().get(cacheKey);
        if (cached) {
            return {200, cached.value()};
        }

        std::vector<StationStats> result = DAO::getInstance().getTopStations(startTime, endTime, limit);

        std::vector<std::string> stationJsons;
        for (const auto& stats : result) {
            stationJsons.push_back(stationStatsToJson(stats));
        }

        std::string json = ToArrayJson(stationJsons);
        Cache::getInstance().set(cacheKey, json, 60000);
        return {200, json};
    } catch (...) {
        return {500, CreateErrorResponse(500, "Internal server error")};
    }
}

HttpResponse StatsAPI::getDashboardStats(const HttpRequest& request) {
    try {
        std::string dateFilter = request.queryParams.count("date") ? request.queryParams.at("date") : "";        
        std::string cacheKey = "dashboard_stats_" + (dateFilter.empty() ? "all" : dateFilter);

        auto cached = Cache::getInstance().get(cacheKey);
        if (cached) {
            return {200, cached.value()};
        }

        sqlite3* db = SQLiteHelper::getInstance().getDB();
        if (!db) {
            return {500, CreateErrorResponse(500, "Database connection failed")};
        }

        int totalStations = 0;
        int totalBikes = 0;
        int availableBikes = 0;
        int activeRentals = 0;
        double totalRevenue = 0;

        std::string stationSql = "SELECT COUNT(*) FROM stations;";
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, stationSql.c_str(), -1, &stmt, nullptr);
        if (rc == SQLITE_OK) {
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                totalStations = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }

        std::string bikeSql = "SELECT COUNT(*), SUM(CASE WHEN status = 'normal' THEN 1 ELSE 0 END) FROM bikes;";
        rc = sqlite3_prepare_v2(db, bikeSql.c_str(), -1, &stmt, nullptr);
        if (rc == SQLITE_OK) {
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                totalBikes = sqlite3_column_int(stmt, 0);
                availableBikes = sqlite3_column_int(stmt, 1);
            }
            sqlite3_finalize(stmt);
        }

        std::string rentalSql = "SELECT COUNT(*) FROM rentals WHERE end_time IS NULL;";
        rc = sqlite3_prepare_v2(db, rentalSql.c_str(), -1, &stmt, nullptr);
        if (rc == SQLITE_OK) {
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                activeRentals = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }

        std::string revenueSql = "SELECT IFNULL(SUM(fee), 0) FROM rentals WHERE end_time IS NOT NULL;";
        if (!dateFilter.empty()) {
            revenueSql = "SELECT IFNULL(SUM(fee), 0) FROM rentals WHERE end_time IS NOT NULL AND DATE(start_time) = ?;";
        }

        rc = sqlite3_prepare_v2(db, revenueSql.c_str(), -1, &stmt, nullptr);
        if (rc == SQLITE_OK) {
            if (!dateFilter.empty()) {
                sqlite3_bind_text(stmt, 1, dateFilter.c_str(), -1, SQLITE_STATIC);
            }
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                totalRevenue = sqlite3_column_double(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }

        std::stringstream ss;
        ss << "{";
        ss << ToJson("total_stations", totalStations) << ",";
        ss << ToJson("total_bikes", totalBikes) << ",";
        ss << ToJson("available_bikes", availableBikes) << ",";
        ss << ToJson("active_rentals", activeRentals) << ",";
        ss << ToJson("total_revenue", totalRevenue);
        ss << "}";

        std::string json = ss.str();
        Cache::getInstance().set(cacheKey, json, 30000);
        return {200, json};
    } catch (...) {
        return {500, CreateErrorResponse(500, "Internal server error")};
    }
}
