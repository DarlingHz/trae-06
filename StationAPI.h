#pragma once
#include "HttpServer.h"
#include "DAO.h"
#include "Cache.h"

class StationAPI {
public:
    static HttpResponse createStation(const HttpRequest& request);
    static HttpResponse updateStation(const HttpRequest& request);
    static HttpResponse getStation(const HttpRequest& request);
    static HttpResponse getStations(const HttpRequest& request);

private:
    static std::string stationToJson(const Station& station);
};
