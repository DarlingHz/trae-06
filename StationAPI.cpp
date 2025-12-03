#include "StationAPI.h"
#include <sstream>
#include <regex>

std::string StationAPI::stationToJson(const Station& station) {
    std::stringstream ss;
    ss << "{";
    ss << ToJson("station_id", station.id) << ",";
    ss << ToJson("name", station.name) << ",";
    ss << ToJson("latitude", station.latitude) << ",";
    ss << ToJson("longitude", station.longitude) << ",";
    ss << ToJson("capacity", station.capacity) << ",";
    ss << ToJson("available_bikes", station.availableBikes) << ",";
    ss << ToJson("created_at", station.createdAt);
    ss << "}";
    return ss.str();
}

HttpResponse StationAPI::createStation(const HttpRequest& request) {
    try {
        std::string name;
        double latitude = 0, longitude = 0;
        int capacity = 0;

        std::regex nameRx(R"("name":\s*"([^"]+)")");
        std::smatch match;
        if (std::regex_search(request.body, match, nameRx)) {
            name = match[1];
        }

        std::regex latRx(R"("latitude":\s*([-+]?[0-9]*\.?[0-9]+))");
        if (std::regex_search(request.body, match, latRx)) {
            latitude = std::stod(match[1]);
        }

        std::regex lonRx(R"("longitude":\s*([-+]?[0-9]*\.?[0-9]+))");
        if (std::regex_search(request.body, match, lonRx)) {
            longitude = std::stod(match[1]);
        }

        std::regex capRx(R"("capacity":\s*(\d+))");
        if (std::regex_search(request.body, match, capRx)) {
            capacity = std::stoi(match[1]);
        }

        if (name.empty() || capacity <= 0) {
            return {400, CreateErrorResponse(400, "Invalid parameters: name and capacity are required")};
        }

        int stationId = DAO::getInstance().createStation(name, latitude, longitude, capacity);
        if (stationId < 0) {
            return {500, CreateErrorResponse(500, "Failed to create station")};
        }

        std::optional<Station> station = DAO::getInstance().getStationById(stationId);
        if (!station) {
            return {500, CreateErrorResponse(500, "Station not found after creation")};
        }

        return {201, stationToJson(station.value())};
    } catch (...) {
        return {500, CreateErrorResponse(500, "Internal server error")};
    }
}

HttpResponse StationAPI::updateStation(const HttpRequest& request) {
    try {
        std::string path = request.path;
        std::regex idRx(R"(/stations/(\d+))");
        std::smatch match;

        if (!std::regex_match(path, match, idRx)) {
            return {400, CreateErrorResponse(400, "Invalid station ID")};
        }

        int stationId = std::stoi(match[1]);

        std::string name;
        double latitude = 0, longitude = 0;
        int capacity = 0;

        std::regex nameRx(R"("name":\s*"([^"]+)")");
        if (std::regex_search(request.body, match, nameRx)) {
            name = match[1];
        }

        std::regex latRx(R"("latitude":\s*([-+]?[0-9]*\.?[0-9]+))");
        if (std::regex_search(request.body, match, latRx)) {
            latitude = std::stod(match[1]);
        }

        std::regex lonRx(R"("longitude":\s*([-+]?[0-9]*\.?[0-9]+))");
        if (std::regex_search(request.body, match, lonRx)) {
            longitude = std::stod(match[1]);
        }

        std::regex capRx(R"("capacity":\s*(\d+))");
        if (std::regex_search(request.body, match, capRx)) {
            capacity = std::stoi(match[1]);
        }

        if (name.empty() && latitude == 0 && longitude == 0 && capacity == 0) {
            return {400, CreateErrorResponse(400, "No update fields provided")};
        }

        bool success = DAO::getInstance().updateStation(stationId, name, latitude, longitude, capacity);
        if (!success) {
            return {404, CreateErrorResponse(404, "Station not found")};
        }

        std::optional<Station> station = DAO::getInstance().getStationById(stationId);
        if (!station) {
            return {500, CreateErrorResponse(500, "Station not found after update")};
        }

        Cache::getInstance().invalidate("stations_" + std::to_string(stationId));
        Cache::getInstance().invalidate("stations_list");

        return {200, stationToJson(station.value())};
    } catch (...) {
        return {500, CreateErrorResponse(500, "Internal server error")};
    }
}

HttpResponse StationAPI::getStation(const HttpRequest& request) {
    try {
        std::string path = request.path;
        std::regex idRx(R"(/stations/(\d+))");
        std::smatch match;

        if (!std::regex_match(path, match, idRx)) {
            return {400, CreateErrorResponse(400, "Invalid station ID")};
        }

        int stationId = std::stoi(match[1]);
        std::string cacheKey = "stations_" + std::to_string(stationId);

        auto cached = Cache::getInstance().get(cacheKey);
        if (cached) {
            return {200, cached.value()};
        }

        std::optional<Station> station = DAO::getInstance().getStationById(stationId);
        if (!station) {
            return {404, CreateErrorResponse(404, "Station not found")};
        }

        std::string json = stationToJson(station.value());
        Cache::getInstance().set(cacheKey, json, 30000);
        return {200, json};
    } catch (...) {
        return {500, CreateErrorResponse(500, "Internal server error")};
    }
}

HttpResponse StationAPI::getStations(const HttpRequest& request) {
    try {
        int page = 1;
        int pageSize = 10;
        std::optional<int> minAvailableBikes;

        if (request.queryParams.count("page")) {
            page = std::stoi(request.queryParams.at("page"));
        }
        if (request.queryParams.count("page_size")) {
            pageSize = std::stoi(request.queryParams.at("page_size"));
        }
        if (request.queryParams.count("min_available_bikes")) {
            minAvailableBikes = std::stoi(request.queryParams.at("min_available_bikes"));
        }

        std::stringstream cacheKeyStream;
        cacheKeyStream << "stations_list_" << page << "_" << pageSize;
        if (minAvailableBikes) {
            cacheKeyStream << "_" << minAvailableBikes.value();
        }
        std::string cacheKey = cacheKeyStream.str();

        auto cached = Cache::getInstance().get(cacheKey);
        if (cached) {
            return {200, cached.value()};
        }

        StationQueryResult result = DAO::getInstance().getStations(page, pageSize, minAvailableBikes);

        std::vector<std::string> stationJsons;
        for (const auto& station : result.stations) {
            stationJsons.push_back(stationToJson(station));
        }

        std::stringstream ss;
        ss << "{";
        ss << ToJson("page", result.pagination.page) << ",";
        ss << ToJson("page_size", result.pagination.pageSize) << ",";
        ss << ToJson("total_items", result.pagination.totalItems) << ",";
        ss << ToJson("total_pages", result.pagination.totalPages) << ",";
        ss << "\"stations\":" << ToArrayJson(stationJsons);
        ss << "}";

        std::string json = ss.str();
        Cache::getInstance().set(cacheKey, json, 15000);
        return {200, json};
    } catch (...) {
        return {500, CreateErrorResponse(500, "Internal server error")};
    }
}
