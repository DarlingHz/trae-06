#include "cache/LRUCache.h"
#include <regex>

namespace cache {

LRUCache::LRUCache(int capacity, int ttl) 
    : capacity_(capacity), ttl_(ttl) {
    if(capacity <= 0) {
        capacity_ = 1000;
    }
    if(ttl <= 0) {
        ttl_ = 300;
    }
}

void LRUCache::cleanup() {
    auto now = std::chrono::system_clock::now();
    
    while(!list_.empty()) {
        auto& item = list_.front();
        if(item.expires_at <= now) {
            map_.erase(item.key);
            list_.pop_front();
        } else {
            break;
        }
    }
}

void LRUCache::put(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    cleanup();
    
    // Check if key already exists
    auto it = map_.find(key);
    if(it != map_.end()) {
        // Update existing item
        it->second->value = value;
        it->second->expires_at = std::chrono::system_clock::now() + 
                               std::chrono::seconds(ttl_);
        list_.splice(list_.begin(), list_, it->second);
        return;
    }
    
    // Evict least recently used if at capacity
    if(list_.size() >= capacity_) {
        auto& item = list_.back();
        map_.erase(item.key);
        list_.pop_back();
    }
    
    // Add new item
    list_.emplace_front(CacheItem{key, value, 
        std::chrono::system_clock::now() + std::chrono::seconds(ttl_)});
    map_[key] = list_.begin();
}

std::optional<std::string> LRUCache::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    cleanup();
    
    auto it = map_.find(key);
    if(it == map_.end()) {
        return std::nullopt;
    }
    
    // Update expiration and move to front
    it->second->expires_at = std::chrono::system_clock::now() + 
                           std::chrono::seconds(ttl_);
    list_.splice(list_.begin(), list_, it->second);
    
    return it->second->value;
}

void LRUCache::remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = map_.find(key);
    if(it != map_.end()) {
        list_.erase(it->second);
        map_.erase(it);
    }
}

void LRUCache::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    list_.clear();
    map_.clear();
}

size_t LRUCache::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Cleanup before returning size
    const_cast<LRUCache*>(this)->cleanup();
    return list_.size();
}

size_t LRUCache::capacity() const {
    return capacity_;
}

void LRUCache::invalidate(const std::string& pattern) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::regex re(pattern);
    auto it = list_.begin();
    while(it != list_.end()) {
        if(std::regex_match(it->key, re)) {
            map_.erase(it->key);
            it = list_.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace cache
