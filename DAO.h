#pragma once

#include "Models.h"
#include "SQLiteHelper.h"
#include <optional>
#include <vector>
#include <string>

class DAO {
private:
    DAO() = default;
    ~DAO() = default;

    DAO(const DAO&) = delete;
    DAO& operator=(const DAO&) = delete;
    DAO(DAO&&) = delete;
    DAO& operator=(DAO&&) = delete;

public:
    static DAO& getInstance();

    // User operations
    int createUser(const std::string& nickname);
    std::optional<User> getUserById(int userId);
    bool existsUser(int userId);

    // Station operations
    int createStation(const std::string& name, double latitude, double longitude, int capacity);
    bool updateStation(int stationId, const std::string& name = "", double latitude = 0, 
        double longitude = 0, int capacity = 0);
    std::optional<Station> getStationById(int stationId);
    StationQueryResult getStations(int page = 1, int pageSize = 10, 
        std::optional<int> minAvailableBikes = std::nullopt);
    bool updateStationAvailableBikes(int stationId, int delta);

    // Bike operations
    int createBike(int stationId, const std::string& status = "normal");
    bool updateBike(int bikeId, const std::optional<int>& stationId = std::nullopt, 
        const std::string& status = "");
    std::optional<Bike> getBikeById(int bikeId);
    std::optional<Bike> getAvailableBikeAtStation(int stationId);

    // Rental operations
    int startRental(int userId, int stationId, int bikeId);
    bool endRental(int rentalId, int endStationId);
    std::optional<Rental> getRentalById(int rentalId);
    std::optional<Rental> getActiveRentalByUserId(int userId);
    UserRentalResult getUserRentals(int userId);
    std::vector<StationStats> getTopStations(const std::string& startTime, 
        const std::string& endTime, int limit = 10);

    // Utility
    std::string getCurrentDateTime();
    int calculateFee(const std::string& startTime, const std::string& endTime);
};
