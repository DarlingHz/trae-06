#pragma once

#include <unordered_map>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <optional>
#include <stdexcept>
#include <cstdint>
#include <chrono>

namespace cache {

template <typename K, typename V>
class LRUCache {
public:
    struct CacheEntry {
        V value;
        std::chrono::system_clock::time_point expire_time;
        bool is_expired() const {
            return expire_time > std::chrono::system_clock::now();
        }
    };
    
    LRUCache(size_t max_entries, std::uint32_t default_expire_seconds = 300)
        : max_entries_(max_entries), default_expire_seconds_(default_expire_seconds) {
        if (max_entries == 0) {
            throw std::invalid_argument("max_entries must be greater than 0");
        }
    }
    
    ~LRUCache() = default;
    
    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;
    
    LRUCache(LRUCache&&) = default;
    LRUCache& operator=(LRUCache&&) = default;
    
    std::optional<V> get(const K& key) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto it = cache_map_.find(key);
        if (it == cache_map_.end()) {
            return std::nullopt;
        }
        
        // 检查是否过期
        const auto& entry = it->second;
        if (entry.is_expired()) {
            // 过期则删除
            cache_list_.erase(it->second);
            cache_map_.erase(it);
            return std::nullopt;
        }
        
        // 移动到链表头部（最近使用）
        cache_list_.splice(cache_list_.begin(), cache_list_, it->second);
        
        return entry.value;
    }
    
    void put(const K& key, const V& value, std::optional<std::uint32_t> expire_seconds = std::nullopt) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto now = std::chrono::system_clock::now();
        auto expire_time = now + std::chrono::seconds(expire_seconds.value_or(default_expire_seconds_));
        
        // 检查是否已存在
        auto it = cache_map_.find(key);
        if (it != cache_map_.end()) {
            // 更新值
            it->second.value = value;
            it->second.expire_time = expire_time;
            // 移动到链表头部
            cache_list_.splice(cache_list_.begin(), cache_list_, it->second);
            return;
        }
        
        // 如果缓存已满，删除最久未使用的条目
        if (cache_map_.size() >= max_entries_) {
            auto& least_recent = cache_list_.back();
            cache_map_.erase(least_recent.first);
            cache_list_.pop_back();
        }
        
        // 创建新条目
        cache_list_.emplace_front(key, CacheEntry{value, expire_time});
        cache_map_[key] = cache_list_.begin();
    }
    
    bool remove(const K& key) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto it = cache_map_.find(key);
        if (it == cache_map_.end()) {
            return false;
        }
        
        cache_list_.erase(it->second);
        cache_map_.erase(it);
        return true;
    }
    
    void clear() {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        cache_map_.clear();
        cache_list_.clear();
    }
    
    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return cache_map_.size();
    }
    
    size_t max_entries() const {
        return max_entries_;
    }
    
    bool contains(const K& key) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return cache_map_.find(key) != cache_map_.end();
    }
    
    // 清理过期条目
    void evict_expired() {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto now = std::chrono::system_clock::now();
        auto it = cache_list_.begin();
        while (it != cache_list_.end()) {
            if (it->second.expire_time <= now) {
                cache_map_.erase(it->first);
                it = cache_list_.erase(it);
            } else {
                ++it;
            }
        }
    }
    
private:
    size_t max_entries_;
    std::uint32_t default_expire_seconds_;
    
    using ListItem = std::pair<K, CacheEntry>;
    using ListIterator = typename std::list<ListItem>::iterator;
    
    std::list<ListItem> cache_list_;
    std::unordered_map<K, ListIterator> cache_map_;
    mutable std::shared_mutex mutex_;
};

} // namespace cache
