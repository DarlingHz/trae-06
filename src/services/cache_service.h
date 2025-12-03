#pragma once

#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include "../utils/redis_client.h"
#include "../utils/config.h"
#include "../utils/logger.h"

class CacheService {
public:
    void init() {
        Config& config = Config::getInstance();
        std::string redisHost = config.getRedisHost();
        int redisPort = config.getRedisPort();
        std::string redisPassword = config.getRedisPassword();
        int redisDb = config.getRedisDb();

        if (!redisClient.connect(redisHost, redisPort, redisPassword, redisDb)) {
            throw std::runtime_error("Failed to connect to Redis");
        }

        LOG_INFO("Connected to Redis successfully");
    }

    bool set(const std::string& key, const std::string& value, int expireSeconds = 0) {
        if (key.empty()) {
            return false;
        }

        try {
            if (expireSeconds > 0) {
                return redisClient.setEx(key, value, expireSeconds);
            } else {
                return redisClient.set(key, value);
            }
        } catch (const std::exception& e) {
            LOG_WARN("Failed to set cache: %s", e.what());
            return false;
        }
    }

    std::shared_ptr<std::string> get(const std::string& key) {
        if (key.empty()) {
            return nullptr;
        }

        try {
            return redisClient.get(key);
        } catch (const std::exception& e) {
            LOG_WARN("Failed to get cache: %s", e.what());
            return nullptr;
        }
    }

    bool del(const std::string& key) {
        if (key.empty()) {
            return false;
        }

        try {
            return redisClient.del(key);
        } catch (const std::exception& e) {
            LOG_WARN("Failed to delete cache: %s", e.what());
            return false;
        }
    }

    bool exists(const std::string& key) {
        if (key.empty()) {
            return false;
        }

        try {
            return redisClient.exists(key);
        } catch (const std::exception& e) {
            LOG_WARN("Failed to check cache existence: %s", e.what());
            return false;
        }
    }

    // 生成保修即将过期提醒的缓存键
    std::string getWarrantyUpcomingKey(int userId, int days) {
        return "warranty_upcoming:" + std::to_string(userId) + ":" + std::to_string(days);
    }

    // 生成用户设备列表的缓存键
    std::string getUserDevicesKey(int userId, const std::string& categoryFilter = "", bool underWarranty = false) {
        std::string key = "user_devices:" + std::to_string(userId);
        if (!categoryFilter.empty()) {
            key += ":" + categoryFilter;
        }
        if (underWarranty) {
            key += ":under_warranty";
        }
        return key;
    }

    // 清除用户相关的所有缓存
    void invalidateUserCache(int userId) {
        // 这里简单实现，可以根据实际情况改进
        // 实际生产中可能需要使用Redis的scan功能查找相关键
        del("user:" + std::to_string(userId));
        del("user_devices:" + std::to_string(userId));
        del("warranty_upcoming:" + std::to_string(userId) + ":30");
    }

    // 清除设备相关的所有缓存
    void invalidateDeviceCache(int deviceId) {
        del("device:" + std::to_string(deviceId));
        // 清除设备的保修策略缓存
        del("device_warranties:" + std::to_string(deviceId));
    }

    // 清除维修单相关的缓存
    void invalidateRepairOrderCache(int repairOrderId) {
        del("repair_order:" + std::to_string(repairOrderId));
    }

    void invalidateAllCache() {
        try {
            redisClient.flushAll();
        } catch (const std::exception& e) {
            LOG_WARN("Failed to flush all cache: %s", e.what());
        }
    }

private:
    RedisClient redisClient;

    CacheService() = default;
    CacheService(const CacheService&) = delete;
    CacheService& operator=(const CacheService&) = delete;

public:
    static CacheService& getInstance() {
        static CacheService instance;
        return instance;
    }
};
