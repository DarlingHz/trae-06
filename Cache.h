#pragma once

#include <string>
#include <unordered_map>
#include <chrono>

struct CacheValue {
    std::string data;
    std::chrono::system_clock::time_point expiresAt;
};

class Cache {
private:
    std::unordered_map<std::string, CacheValue> cacheMap;
    size_t maxSize;
    long long defaultTTLSeconds;

    Cache();
    ~Cache() = default;

    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;
    Cache(Cache&&) = delete;
    Cache& operator=(Cache&&) = delete;

    void evictExpired();

public:
    static Cache& getInstance();

    void setMaxSize(size_t size);
    void setDefaultTTL(long long seconds);

    void set(const std::string& key, const std::string& value);
    void set(const std::string& key, const std::string& value, long long ttlSeconds);

    std::string get(const std::string& key);
    bool exists(const std::string& key);
    void remove(const std::string& key);
    void clear();
    size_t size() const;
};
