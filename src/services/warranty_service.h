#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include "../models/warranty_policy.h"
#include "../repositories/warranty_policy_repository.h"
#include "device_service.h"
#include "../utils/logger.h"
#include "../utils/date_utils.h"

class WarrantyService {
public:
    std::shared_ptr<WarrantyPolicy> createWarrantyPolicy(int deviceId, const std::string& providerName,
        const std::string& policyType, const std::string& coverageDesc, const std::string& expireAt) {
        // 验证设备存在
        DeviceService& deviceService = DeviceService::getInstance();
        if (!deviceService.deviceExists(deviceId)) {
            throw std::runtime_error("Device not found");
        }

        WarrantyPolicy policy;
        policy.deviceId = deviceId;
        policy.providerName = providerName;
        policy.policyType = WarrantyPolicy::policyTypeFromString(policyType);
        policy.coverageDesc = coverageDesc;
        policy.expireAt = expireAt;

        if (!policy.isValid()) {
            throw std::runtime_error("Invalid warranty policy data");
        }

        try {
            auto createdPolicy = WarrantyPolicyRepository::getInstance().create(policy);
            // 更新设备的保修截止日期
            updateDeviceWarrantyExpireAt(deviceId);
            return createdPolicy;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create warranty policy: %s", e.what());
            throw;
        }
    }

    std::shared_ptr<WarrantyPolicy> getWarrantyPolicyById(int id) {
        if (id <= 0) {
            throw std::runtime_error("Invalid warranty policy ID");
        }

        try {
            auto policy = WarrantyPolicyRepository::getInstance().findById(id);
            if (!policy) {
                throw std::runtime_error("Warranty policy not found");
            }
            return policy;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get warranty policy by ID: %s", e.what());
            throw;
        }
    }

    std::vector<std::shared_ptr<WarrantyPolicy>> getDeviceWarranties(int deviceId) {
        if (deviceId <= 0) {
            throw std::runtime_error("Invalid device ID");
        }

        try {
            return WarrantyPolicyRepository::getInstance().findByDeviceId(deviceId);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get device warranties: %s", e.what());
            throw;
        }
    }

    void deleteWarrantyPolicy(int id) {
        if (id <= 0) {
            throw std::runtime_error("Invalid warranty policy ID");
        }

        try {
            auto policy = WarrantyPolicyRepository::getInstance().findById(id);
            if (!policy) {
                throw std::runtime_error("Warranty policy not found");
            }

            int deviceId = policy->deviceId;
            WarrantyPolicyRepository::getInstance().deleteById(id);
            // 更新设备的保修截止日期
            updateDeviceWarrantyExpireAt(deviceId);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to delete warranty policy: %s", e.what());
            throw;
        }
    }

    bool warrantyPolicyExists(int id) {
        if (id <= 0) {
            return false;
        }

        try {
            return WarrantyPolicyRepository::getInstance().findById(id) != nullptr;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to check warranty policy existence: %s", e.what());
            return false;
        }
    }

private:
    void updateDeviceWarrantyExpireAt(int deviceId) {
        try {
            auto repo = WarrantyPolicyRepository::getInstance();
            std::string maxExpireAt = repo.getMaxExpireAtForDevice(deviceId);
            if (!maxExpireAt.empty()) {
                DeviceService::getInstance().updateDeviceWarrantyExpireAt(deviceId, maxExpireAt);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to update device warranty expire at: %s", e.what());
            throw;
        }
    }

    WarrantyService(const WarrantyService&) = delete;
    WarrantyService& operator=(const WarrantyService&) = delete;

public:
    static WarrantyService& getInstance() {
        static WarrantyService instance;
        return instance;
    }
};
