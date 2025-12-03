#pragma once
#include "HttpServer.h"
#include "DAO.h"
#include "Cache.h"

class StatsAPI {
public:
    static HttpResponse getTopStations(const HttpRequest& request);
    static HttpResponse getDashboardStats(const HttpRequest& request);

private:
    static std::string stationStatsToJson(const StationStats& stats);
};
