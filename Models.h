#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>

struct User {
    int userId;
    std::string nickname;
    std::string createdAt;

    User() : userId(-1), nickname(""), createdAt("") {}
    User(int id, const std::string& n, const std::string& c) : 
        userId(id), nickname(n), createdAt(c) {}
};

struct Station {
    int stationId;
    std::string name;
    double latitude;
    double longitude;
    int capacity;
    int availableBikes;
    std::string createdAt;

    Station() : stationId(-1), name(""), latitude(0), longitude(0), capacity(0), availableBikes(0), createdAt("") {}
    Station(int id, const std::string& n, double lat, double lon, int cap, int avail, const std::string& c) : 
        stationId(id), name(n), latitude(lat), longitude(lon), capacity(cap), availableBikes(avail), createdAt(c) {}
};

struct Bike {
    int bikeId;
    std::optional<int> currentStationId;
    std::string status; // normal, broken, maintenance
    std::string createdAt;

    Bike() : bikeId(-1), currentStationId(std::nullopt), status("normal"), createdAt("") {}
    Bike(int id, const std::optional<int>& sid, const std::string& s, const std::string& c) : 
        bikeId(id), currentStationId(sid), status(s), createdAt(c) {}
};

struct Rental {
    int rentalId;
    int userId;
    int bikeId;
    int startStationId;
    std::optional<int> endStationId;
    std::string startTime;
    std::optional<std::string> endTime;
    double fee;
    std::string createdAt;

    Rental() : rentalId(-1), userId(-1), bikeId(-1), startStationId(-1), 
        endStationId(std::nullopt), startTime(""), endTime(std::nullopt), fee(0.0), createdAt("") {}
    Rental(int id, int uid, int bid, int s, const std::optional<int>& e, const std::string& st, 
        const std::optional<std::string>& et, double f, const std::string& c) : 
        rentalId(id), userId(uid), bikeId(bid), startStationId(s), endStationId(e), 
        startTime(st), endTime(et), fee(f), createdAt(c) {}
};

struct PaginationInfo {
    int page;
    int pageSize;
    int totalItems;
    int totalPages;

    PaginationInfo() : page(1), pageSize(10), totalItems(0), totalPages(0) {}
    PaginationInfo(int p, int ps, int ti, int tp) : page(p), pageSize(ps), totalItems(ti), totalPages(tp) {}
};

struct StationQueryResult {
    std::vector<Station> stations;
    PaginationInfo pagination;
};

struct UserRentalStats {
    int totalRides;
    int totalMinutes;
    double totalFee;

    UserRentalStats() : totalRides(0), totalMinutes(0), totalFee(0.0) {}
    UserRentalStats(int tr, int tm, double tf) : totalRides(tr), totalMinutes(tm), totalFee(tf) {}
};

struct UserRentalResult {
    UserRentalStats stats;
    std::vector<Rental> rentals;
};

struct StationStats {
    int stationId;
    std::string stationName;
    int rentalCount;

    StationStats() : stationId(-1), stationName(""), rentalCount(0) {}
    StationStats(int id, const std::string& n, int c) : stationId(id), stationName(n), rentalCount(c) {}
};

struct Metrics {
    std::string startTime;
    int totalRequests;
    int requestsLast5Minutes;
    int dbConnectionsActive;
    int dbConnectionsTotal;
    int cacheSize;

    Metrics() : totalRequests(0), requestsLast5Minutes(0), 
        dbConnectionsActive(0), dbConnectionsTotal(0), cacheSize(0) {}
};

struct StartRentalRequest {
    int userId;
    int stationId;
};

struct EndRentalRequest {
    int rentalId;
    int endStationId;
};
