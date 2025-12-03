#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include "../models/device.h"
#include "../repositories/device_repository.h"
#include "user_service.h"
#include "../utils/logger.h"
#include "../utils/date_utils.h"

class DeviceService {
public:
    std::shared_ptr<Device> createDevice(int ownerUserId, const std::string& category, 
        const std::string& brand, const std::string& model, const std::string& serialNumber,
        const std::string& purchaseDate) {
        // 验证用户存在
        UserService& userService = UserService::getInstance();
        if (!userService.userExists(ownerUserId)) {
            throw std::runtime_error("User not found");
        }

        // 验证序列号是否重复
        DeviceRepository& repo = DeviceRepository::getInstance();
        if (repo.findBySerialNumber(serialNumber)) {
            throw std::runtime_error("Serial number already exists");
        }

        Device device;
        device.ownerUserId = ownerUserId;
        device.category = Device::categoryFromString(category);
        device.brand = brand;
        device.model = model;
        device.serialNumber = serialNumber;
        device.purchaseDate = purchaseDate;
        device.warrantyExpireAt = purchaseDate; // 默认保修截止日期为购买日期
        
        if (!device.isValid()) {
            throw std::runtime_error("Invalid device data");
        }

        try {
            return repo.create(device);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create device: %s", e.what());
            throw;
        }
    }

    std::shared_ptr<Device> getDeviceById(int id) {
        if (id <= 0) {
            throw std::runtime_error("Invalid device ID");
        }

        try {
            auto device = DeviceRepository::getInstance().findById(id);
            if (!device) {
                throw std::runtime_error("Device not found");
            }
            return device;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get device by ID: %s", e.what());
            throw;
        }
    }

    bool deviceExists(int id) {
        if (id <= 0) {
            return false;
        }

        try {
            return DeviceRepository::getInstance().findById(id) != nullptr;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to check device existence: %s", e.what());
            return false;
        }
    }

    std::vector<std::shared_ptr<Device>> getUserDevices(int userId, const std::string& categoryFilter = "", 
        bool underWarranty = false, int page = 1, int pageSize = 10) {
        UserService& userService = UserService::getInstance();
        if (!userService.userExists(userId)) {
            throw std::runtime_error("User not found");
        }

        try {
            return DeviceRepository::getInstance().findByUserId(userId, categoryFilter, underWarranty, page, pageSize);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get user devices: %s", e.what());
            throw;
        }
    }

    std::vector<std::shared_ptr<Device>> getWarrantyUpcoming(int userId, int days = 30) {
        if (days < 0) {
            days = 30;
        }

        try {
            std::string today = DateUtils::getToday();
            std::string endDate = DateUtils::addDays(today, days);
            return DeviceRepository::getInstance().findWarrantyUpcoming(userId, today, endDate);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get upcoming warranties: %s", e.what());
            throw;
        }
    }

    void updateDeviceWarrantyExpireAt(int deviceId, const std::string& newExpireAt) {
        if (deviceId <= 0) {
            throw std::runtime_error("Invalid device ID");
        }

        try {
            DeviceRepository::getInstance().updateWarrantyExpireAt(deviceId, newExpireAt);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to update device warranty: %s", e.what());
            throw;
        }
    }

private:
    DeviceService(const DeviceService&) = delete;
    DeviceService& operator=(const DeviceService&) = delete;

public:
    static DeviceService& getInstance() {
        static DeviceService instance;
        return instance;
    }
};
