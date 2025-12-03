#pragma once

#include <string>
#include <ctime>

class ServiceCenter {
public:
    int id = 0;
    std::string name;
    std::string city;
    std::string address;
    std::string contactPhone;
    time_t createdAt = 0;

    ServiceCenter() = default;

    ServiceCenter(int id, const std::string& name, const std::string& city,
                  const std::string& address, const std::string& contactPhone, time_t createdAt)
        : id(id), name(name), city(city), address(address),
          contactPhone(contactPhone), createdAt(createdAt) {}

    bool isValid() const {
        return !name.empty() && !city.empty() && !address.empty();
    }
};
