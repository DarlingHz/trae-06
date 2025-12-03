#include "cache/cache_manager.h"
#include "cache/lru_cache.h"
#include <stdexcept>
#include <sstream>

namespace cache {

CacheManager::CacheManager() {
}

void CacheManager::initialize(const CacheConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    config_ = config;
    enabled_ = config.enable;
    
    if (!enabled_) {
        return;
    }
    
    // 创建用户缓存（默认200条目，1小时过期）
    create_cache<int, std::string>(CacheType::USER, 200, 3600);
    
    // 创建公告详情缓存（默认500条目，10分钟过期）
    create_cache<int, std::string>(CacheType::ANNOUNCEMENT, 500, 600);
    
    // 创建公告列表缓存（默认100条目，5分钟过期）
    create_cache<std::string, std::string>(CacheType::ANNOUNCEMENT_LIST, 100, 300);
    
    // 创建阅读记录缓存（默认1000条目，1小时过期）
    create_cache<std::string, std::string>(CacheType::READ_RECEIPT, 1000, 3600);
}

template <typename K, typename V>
std::shared_ptr<LRUCache<K, V>> CacheManager::get_cache(CacheType type) {
    auto it = caches_.find(type);
    if (it == caches_.end()) {
        return nullptr;
    }
    
    return std::static_pointer_cast<LRUCache<K, V>>(it->second);
}

template <typename K, typename V>
void CacheManager::create_cache(CacheType type, size_t max_entries, std::uint32_t default_expire_seconds) {
    auto cache = std::make_shared<LRUCache<K, V>>(max_entries, default_expire_seconds);
    caches_[type] = cache;
}

std::string CacheManager::get_read_receipt_key(int user_id, int announcement_id) const {
    std::ostringstream oss;
    oss << user_id << "_" << announcement_id;
    return oss.str();
}

std::optional<std::string> CacheManager::get_user(int user_id) {
    if (!enabled_) {
        return std::nullopt;
    }
    
    auto cache = get_cache<int, std::string>(CacheType::USER);
    if (!cache) {
        return std::nullopt;
    }
    
    return cache->get(user_id);
}

void CacheManager::put_user(int user_id, const std::string& user_json, std::optional<std::uint32_t> expire_seconds) {
    if (!enabled_) {
        return;
    }
    
    auto cache = get_cache<int, std::string>(CacheType::USER);
    if (cache) {
        cache->put(user_id, user_json, expire_seconds);
    }
}

void CacheManager::remove_user(int user_id) {
    if (!enabled_) {
        return;
    }
    
    auto cache = get_cache<int, std::string>(CacheType::USER);
    if (cache) {
        cache->remove(user_id);
    }
}

std::optional<std::string> CacheManager::get_announcement(int announcement_id) {
    if (!enabled_) {
        return std::nullopt;
    }
    
    auto cache = get_cache<int, std::string>(CacheType::ANNOUNCEMENT);
    if (!cache) {
        return std::nullopt;
    }
    
    return cache->get(announcement_id);
}

void CacheManager::put_announcement(int announcement_id, const std::string& announcement_json, std::optional<std::uint32_t> expire_seconds) {
    if (!enabled_) {
        return;
    }
    
    auto cache = get_cache<int, std::string>(CacheType::ANNOUNCEMENT);
    if (cache) {
        cache->put(announcement_id, announcement_json, expire_seconds);
    }
}

void CacheManager::remove_announcement(int announcement_id) {
    if (!enabled_) {
        return;
    }
    
    auto cache = get_cache<int, std::string>(CacheType::ANNOUNCEMENT);
    if (cache) {
        cache->remove(announcement_id);
    }
}

void CacheManager::clear_announcements() {
    if (!enabled_) {
        return;
    }
    
    auto announcement_cache = get_cache<int, std::string>(CacheType::ANNOUNCEMENT);
    if (announcement_cache) {
        announcement_cache->clear();
    }
    
    auto list_cache = get_cache<std::string, std::string>(CacheType::ANNOUNCEMENT_LIST);
    if (list_cache) {
        list_cache->clear();
    }
}

std::optional<std::string> CacheManager::get_announcement_list(const std::string& filter_hash) {
    if (!enabled_) {
        return std::nullopt;
    }
    
    auto cache = get_cache<std::string, std::string>(CacheType::ANNOUNCEMENT_LIST);
    if (!cache) {
        return std::nullopt;
    }
    
    return cache->get(filter_hash);
}

void CacheManager::put_announcement_list(const std::string& filter_hash, const std::string& announcements_json, std::optional<std::uint32_t> expire_seconds) {
    if (!enabled_) {
        return;
    }
    
    auto cache = get_cache<std::string, std::string>(CacheType::ANNOUNCEMENT_LIST);
    if (cache) {
        cache->put(filter_hash, announcements_json, expire_seconds);
    }
}

void CacheManager::remove_announcement_list(const std::string& filter_hash) {
    if (!enabled_) {
        return;
    }
    
    auto cache = get_cache<std::string, std::string>(CacheType::ANNOUNCEMENT_LIST);
    if (cache) {
        cache->remove(filter_hash);
    }
}

std::optional<std::string> CacheManager::get_read_receipt(int user_id, int announcement_id) {
    if (!enabled_) {
        return std::nullopt;
    }
    
    auto cache = get_cache<std::string, std::string>(CacheType::READ_RECEIPT);
    if (!cache) {
        return std::nullopt;
    }
    
    std::string key = get_read_receipt_key(user_id, announcement_id);
    return cache->get(key);
}

void CacheManager::put_read_receipt(int user_id, int announcement_id, const std::string& receipt_json, std::optional<std::uint32_t> expire_seconds) {
    if (!enabled_) {
        return;
    }
    
    auto cache = get_cache<std::string, std::string>(CacheType::READ_RECEIPT);
    if (cache) {
        std::string key = get_read_receipt_key(user_id, announcement_id);
        cache->put(key, receipt_json, expire_seconds);
    }
}

void CacheManager::remove_read_receipt(int user_id, int announcement_id) {
    if (!enabled_) {
        return;
    }
    
    auto cache = get_cache<std::string, std::string>(CacheType::READ_RECEIPT);
    if (cache) {
        std::string key = get_read_receipt_key(user_id, announcement_id);
        cache->remove(key);
    }
}

void CacheManager::evict_all_expired() {
    if (!enabled_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& [type, cache_ptr] : caches_) {
        // 这里需要具体类型，我们知道存储的是LRUCache的不同模板实例
        // 由于类型擦除，这里可能需要每个类型单独处理，或者使用多态接口
        // 为简化，暂时不实现完全的过期清理，在get操作时会自动清理
    }
}

void CacheManager::clear_all() {
    if (!enabled_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    caches_.clear();
}

CacheManager::CacheStats CacheManager::get_stats() const {
    CacheStats stats{};
    
    if (!enabled_) {
        return stats;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (auto cache = std::static_pointer_cast<LRUCache<int, std::string>>(caches_.at(CacheType::USER))) {
        stats.user_cache_size = cache->size();
    }
    
    if (auto cache = std::static_pointer_cast<LRUCache<int, std::string>>(caches_.at(CacheType::ANNOUNCEMENT))) {
        stats.announcement_cache_size = cache->size();
    }
    
    if (auto cache = std::static_pointer_cast<LRUCache<std::string, std::string>>(caches_.at(CacheType::ANNOUNCEMENT_LIST))) {
        stats.announcement_list_cache_size = cache->size();
    }
    
    if (auto cache = std::static_pointer_cast<LRUCache<std::string, std::string>>(caches_.at(CacheType::READ_RECEIPT))) {
        stats.read_receipt_cache_size = cache->size();
    }
    
    return stats;
}

bool CacheManager::is_enabled() const {
    return enabled_;
}

void CacheManager::set_enabled(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    enabled_ = enable;
}

// 显式实例化常用模板类型
template std::shared_ptr<LRUCache<int, std::string>> CacheManager::get_cache(CacheType type);
template std::shared_ptr<LRUCache<std::string, std::string>> CacheManager::get_cache(CacheType type);

template void CacheManager::create_cache<int, std::string>(CacheType type, size_t max_entries, std::uint32_t default_expire_seconds);
template void CacheManager::create_cache<std::string, std::string>(CacheType type, size_t max_entries, std::uint32_t default_expire_seconds);

} // namespace cache
