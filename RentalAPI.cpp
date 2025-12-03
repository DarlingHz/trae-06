#include "RentalAPI.h"
#include <sstream>
#include <regex>

std::string RentalAPI::rentalToJson(const Rental& rental) {
    std::stringstream ss;
    ss << "{";
    ss << ToJson("rental_id", rental.id) << ",";
    ss << ToJson("user_id", rental.userId) << ",";
    ss << ToJson("bike_id", rental.bikeId) << ",";
    ss << ToJson("start_station_id", rental.startStationId) << ",";
    ss << ToJson("start_time", rental.startTime);
    
    if (rental.endStationId) {
        ss << "," << ToJson("end_station_id", rental.endStationId.value());
    }
    if (rental.endTime) {
        ss << "," << ToJson("end_time", rental.endTime.value());
    }
    ss << "," << ToJson("fee", rental.fee);
    ss << "," << ToJson("created_at", rental.createdAt);
    ss << "}";
    return ss.str();
}

HttpResponse RentalAPI::startRental(const HttpRequest& request) {
    try {
        int userId = -1;
        int stationId = -1;

        std::regex userIdRx(R"("user_id":\s*(\d+))");
        std::smatch match;
        if (std::regex_search(request.body, match, userIdRx)) {
            userId = std::stoi(match[1]);
        }

        std::regex stationIdRx(R"("station_id":\s*(\d+))");
        if (std::regex_search(request.body, match, stationIdRx)) {
            stationId = std::stoi(match[1]);
        }

        if (userId <= 0 || stationId <= 0) {
            return {400, CreateErrorResponse(400, "Invalid parameters: user_id and station_id are required")};
        }

        if (!DAO::getInstance().existsUser(userId)) {
            return {404, CreateErrorResponse(404, "User not found")};
        }

        std::optional<Station> station = DAO::getInstance().getStationById(stationId);
        if (!station) {
            return {404, CreateErrorResponse(404, "Station not found")};
        }

        std::optional<Bike> bike = DAO::getInstance().getAvailableBikeAtStation(stationId);
        if (!bike) {
            return {400, CreateErrorResponse(400, "No available bikes at this station")};
        }

        std::optional<Rental> existingRental = DAO::getInstance().getActiveRentalByUserId(userId);
        if (existingRental) {
            return {400, CreateErrorResponse(400, "User already has an active rental")};
        }

        int rentalId = DAO::getInstance().startRental(userId, stationId, bike.value().id);
        if (rentalId < 0) {
            return {500, CreateErrorResponse(500, "Failed to start rental")};
        }

        bool bikeUpdated = DAO::getInstance().updateBike(bike.value().id, std::nullopt, "rented");
        bool stationUpdated = DAO::getInstance().updateStationAvailableBikes(stationId, -1);

        if (!bikeUpdated || !stationUpdated) {
            return {500, CreateErrorResponse(500, "Failed to update bike or station status")};
        }

        std::optional<Rental> rental = DAO::getInstance().getRentalById(rentalId);
        if (!rental) {
            return {500, CreateErrorResponse(500, "Rental not found after creation")};
        }

        std::stringstream cacheKey;
        cacheKey << "user_rentals_" << userId;
        Cache::getInstance().invalidate(cacheKey.str());
        Cache::getInstance().invalidate("stations_" + std::to_string(stationId));

        return {201, rentalToJson(rental.value())};
    } catch (...) {
        return {500, CreateErrorResponse(500, "Internal server error")};
    }
}

HttpResponse RentalAPI::endRental(const HttpRequest& request) {
    try {
        int userId = -1;
        int stationId = -1;

        std::regex userIdRx(R"("user_id":\s*(\d+))");
        std::smatch match;
        if (std::regex_search(request.body, match, userIdRx)) {
            userId = std::stoi(match[1]);
        }

        std::regex stationIdRx(R"("station_id":\s*(\d+))");
        if (std::regex_search(request.body, match, stationIdRx)) {
            stationId = std::stoi(match[1]);
        }

        if (userId <= 0 || stationId <= 0) {
            return {400, CreateErrorResponse(400, "Invalid parameters: user_id and station_id are required")};
        }

        std::optional<Station> station = DAO::getInstance().getStationById(stationId);
        if (!station) {
            return {404, CreateErrorResponse(404, "Station not found")};
        }

        std::optional<Rental> activeRental = DAO::getInstance().getActiveRentalByUserId(userId);
        if (!activeRental) {
            return {400, CreateErrorResponse(400, "No active rental found for user")};
        }

        bool success = DAO::getInstance().endRental(activeRental.value().id, stationId);
        if (!success) {
            return {500, CreateErrorResponse(500, "Failed to end rental")};
        }

        bool bikeUpdated = DAO::getInstance().updateBike(activeRental.value().bikeId, stationId, "normal");
        bool stationUpdated = DAO::getInstance().updateStationAvailableBikes(stationId, 1);

        if (!bikeUpdated || !stationUpdated) {
            return {500, CreateErrorResponse(500, "Failed to update bike or station status")};
        }

        std::optional<Rental> rental = DAO::getInstance().getRentalById(activeRental.value().id);
        if (!rental) {
            return {500, CreateErrorResponse(500, "Rental not found after update")};
        }

        std::stringstream cacheKey;
        cacheKey << "user_rentals_" << userId;
        Cache::getInstance().invalidate(cacheKey.str());
        Cache::getInstance().invalidate("stations_" + std::to_string(stationId));

        return {200, rentalToJson(rental.value())};
    } catch (...) {
        return {500, CreateErrorResponse(500, "Internal server error")};
    }
}

HttpResponse RentalAPI::getUserRentals(const HttpRequest& request) {
    try {
        std::string path = request.path;
        std::regex idRx(R"(/users/(\d+)/rentals)");
        std::smatch match;

        if (!std::regex_match(path, match, idRx)) {
            return {400, CreateErrorResponse(400, "Invalid user ID format")};
        }

        int userId = std::stoi(match[1]);

        if (!DAO::getInstance().existsUser(userId)) {
            return {404, CreateErrorResponse(404, "User not found")};
        }

        std::string cacheKey = "user_rentals_" + std::to_string(userId);
        auto cached = Cache::getInstance().get(cacheKey);
        if (cached) {
            return {200, cached.value()};
        }

        UserRentalResult result = DAO::getInstance().getUserRentals(userId);

        std::vector<std::string> rentalJsons;
        for (const auto& rental : result.rentals) {
            rentalJsons.push_back(rentalToJson(rental));
        }

        std::stringstream ss;
        ss << "{";
        ss << ToJson("total_rides", result.stats.totalRides) << ",";
        ss << ToJson("total_fee", result.stats.totalFee) << ",";
        ss << "\"rentals\":" << ToArrayJson(rentalJsons);
        ss << "}";

        std::string json = ss.str();
        Cache::getInstance().set(cacheKey, json, 30000);
        return {200, json};
    } catch (...) {
        return {500, CreateErrorResponse(500, "Internal server error")};
    }
}
