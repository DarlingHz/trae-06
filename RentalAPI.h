#pragma once
#include "HttpServer.h"
#include "DAO.h"
#include "Cache.h"

class RentalAPI {
public:
    static HttpResponse startRental(const HttpRequest& request);
    static HttpResponse endRental(const HttpRequest& request);
    static HttpResponse getUserRentals(const HttpRequest& request);

private:
    static std::string rentalToJson(const Rental& rental);
};
