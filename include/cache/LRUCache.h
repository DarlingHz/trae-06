#pragma once

#include <unordered_map>
#include <list>
#include <string>
#include <optional>
#include <chrono>
#include <mutex>

namespace cache {

struct CacheItem {
    std::string key;
    std::string value;
    std::chrono::system_clock::time_point expires_at;
};

class LRUCache {
private:
    using CacheList = std::list<CacheItem>;
    using CacheMap = std::unordered_map<std::string, typename CacheList::iterator>;

    int capacity_;
    int ttl_; // seconds
    mutable std::mutex mutex_;
    
    CacheList list_;
    CacheMap map_;

    void cleanup();

public:
    LRUCache(int capacity = 1000, int ttl = 300);

    void put(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    void remove(const std::string& key);
    void clear();
    size_t size() const;
    size_t capacity() const;
    void invalidate(const std::string& pattern);
};

} // namespace cache
