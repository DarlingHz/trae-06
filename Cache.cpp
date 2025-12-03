#include "Cache.h"
#include <iostream>

Cache::Cache() : maxSize(1000), defaultTTLSeconds(300) {
}

Cache& Cache::getInstance() {
    static Cache instance;
    return instance;
}

void Cache::setMaxSize(size_t size) {
    maxSize = size;
}

void Cache::setDefaultTTL(long long seconds) {
    defaultTTLSeconds = seconds;
}

void Cache::evictExpired() {
    auto now = std::chrono::system_clock::now();
    for (auto it = cacheMap.begin(); it != cacheMap.end();) {
        if (it->second.expiresAt <= now) {
            it = cacheMap.erase(it);
        } else {
            ++it;
        }
    }
}

void Cache::set(const std::string& key, const std::string& value) {
    set(key, value, defaultTTLSeconds);
}

void Cache::set(const std::string& key, const std::string& value, long long ttlSeconds) {
    evictExpired();

    if (cacheMap.size() >= maxSize) {
        cacheMap.clear();
    }

    auto expiresAt = std::chrono::system_clock::now() + std::chrono::seconds(ttlSeconds);
    cacheMap[key] = {value, expiresAt};
}

std::string Cache::get(const std::string& key) {
    evictExpired();

    auto it = cacheMap.find(key);
    if (it != cacheMap.end()) {
        return it->second.data;
    }
    return "";
}

bool Cache::exists(const std::string& key) {
    evictExpired();

    return cacheMap.find(key) != cacheMap.end();
}

void Cache::remove(const std::string& key) {
    cacheMap.erase(key);
}

void Cache::clear() {
    cacheMap.clear();
}

size_t Cache::size() const {
    return cacheMap.size();
}
