#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include "../models/service_center.h"
#include "../repositories/service_center_repository.h"
#include "../utils/logger.h"

class ServiceCenterService {
public:
    std::shared_ptr<ServiceCenter> createServiceCenter(const std::string& name, const std::string& city,
        const std::string& address, const std::string& contactPhone) {
        ServiceCenter serviceCenter;
        serviceCenter.name = name;
        serviceCenter.city = city;
        serviceCenter.address = address;
        serviceCenter.contactPhone = contactPhone;

        if (!serviceCenter.isValid()) {
            throw std::runtime_error("Invalid service center data");
        }

        try {
            return ServiceCenterRepository::getInstance().create(serviceCenter);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create service center: %s", e.what());
            throw;
        }
    }

    std::shared_ptr<ServiceCenter> getServiceCenterById(int id) {
        if (id <= 0) {
            throw std::runtime_error("Invalid service center ID");
        }

        try {
            auto serviceCenter = ServiceCenterRepository::getInstance().findById(id);
            if (!serviceCenter) {
                throw std::runtime_error("Service center not found");
            }
            return serviceCenter;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get service center by ID: %s", e.what());
            throw;
        }
    }

    bool serviceCenterExists(int id) {
        if (id <= 0) {
            return false;
        }

        try {
            return ServiceCenterRepository::getInstance().exists(id);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to check service center existence: %s", e.what());
            return false;
        }
    }

    std::vector<std::shared_ptr<ServiceCenter>> getServiceCentersByCity(const std::string& city) {
        if (city.empty()) {
            throw std::runtime_error("Invalid city name");
        }

        try {
            return ServiceCenterRepository::getInstance().findByCity(city);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get service centers by city: %s", e.what());
            throw;
        }
    }

private:
    ServiceCenterService(const ServiceCenterService&) = delete;
    ServiceCenterService& operator=(const ServiceCenterService&) = delete;

public:
    static ServiceCenterService& getInstance() {
        static ServiceCenterService instance;
        return instance;
    }
};
