#pragma once

#include <string>
#include <ctime>
#include <stdexcept>
#include <map>

class WarrantyPolicy {
public:
    enum class PolicyType {
        Manufacturer,
        Extended,
        Accidental
    };

    int id = 0;
    int deviceId = 0;
    std::string providerName;
    PolicyType policyType = PolicyType::Manufacturer;
    std::string coverageDesc;
    time_t expireAt = 0;

    WarrantyPolicy() = default;

    WarrantyPolicy(int id, int deviceId, const std::string& providerName,
                   PolicyType policyType, const std::string& coverageDesc, time_t expireAt)
        : id(id), deviceId(deviceId), providerName(providerName), policyType(policyType),
          coverageDesc(coverageDesc), expireAt(expireAt) {}

    static PolicyType policyTypeFromString(const std::string& str) {
        static const std::map<std::string, PolicyType> typeMap = {
            {"manufacturer", PolicyType::Manufacturer},
            {"extended", PolicyType::Extended},
            {"accidental", PolicyType::Accidental}
        };
        auto it = typeMap.find(str);
        if (it == typeMap.end()) {
            throw std::runtime_error("Invalid policy type: " + str);
        }
        return it->second;
    }

    static std::string policyTypeToString(PolicyType type) {
        static const std::map<PolicyType, std::string> typeMap = {
            {PolicyType::Manufacturer, "manufacturer"},
            {PolicyType::Extended, "extended"},
            {PolicyType::Accidental, "accidental"}
        };
        auto it = typeMap.find(type);
        if (it == typeMap.end()) return "manufacturer";
        return it->second;
    }

    bool isValid() const {
        return deviceId > 0 && !providerName.empty() && expireAt > 0;
    }
};
