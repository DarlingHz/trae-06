#include <iostream>
#include "SQLiteHelper.h"
#include "ConfigManager.h"
#include "Cache.h"
#include "HttpServer.h"
#include "UserAPI.h"
#include "StationAPI.h"
#include "RentalAPI.h"
#include "StatsAPI.h"

int main() {
    try {
        std::cout << "Starting Bike Rental System..." << std::endl;

        if (!SQLiteHelper::getInstance().connect("./bike_rental.db")) {
            std::cerr << "Failed to connect to database" << std::endl;
            return 1;
        }

        if (!ConfigManager::getInstance().loadConfig("./config/config.json")) {
            std::cerr << "Failed to load configuration file, using default settings" << std::endl;
        }

        HttpServer server(8080);

        server.Post("/users", UserAPI::createUser);
        server.Get("/users/(\d+)", UserAPI::getUser);

        server.Post("/stations", StationAPI::createStation);
        server.Put("/stations/(\d+)", StationAPI::updateStation);
        server.Get("/stations/(\d+)", StationAPI::getStation);
        server.Get("/stations", StationAPI::getStations);

        server.Post("/rentals/start", RentalAPI::startRental);
        server.Post("/rentals/end", RentalAPI::endRental);
        server.Get("/users/(\d+)/rentals", RentalAPI::getUserRentals);

        server.Get("/stats/top-stations", StatsAPI::getTopStations);
        server.Get("/stats/dashboard", StatsAPI::getDashboardStats);

        server.Run();

        SQLiteHelper::getInstance().disconnect();
    } catch (const std::exception& e) {
        std::cerr << "System error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}