#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>
#include <cstdint>
#include <optional>

#include "cache_config.h"
#include "lru_cache.h"

namespace cache {

class CacheManager {
public:
    enum class CacheType {
        USER,              // 用户信息缓存
        ANNOUNCEMENT,      // 公告详情缓存
        ANNOUNCEMENT_LIST, // 公告列表缓存
        READ_RECEIPT       // 阅读记录缓存
    };
    
    static CacheManager& instance() {
        static CacheManager instance;
        return instance;
    }
    
    CacheManager(const CacheManager&) = delete;
    CacheManager& operator=(const CacheManager&) = delete;
    
    CacheManager(CacheManager&&) = delete;
    CacheManager& operator=(CacheManager&&) = delete;
    
    void initialize(const CacheConfig& config);
    
    // 缓存用户信息（key: user_id）
    std::optional<std::string> get_user(int user_id);
    void put_user(int user_id, const std::string& user_json, std::optional<std::uint32_t> expire_seconds = std::nullopt);
    void remove_user(int user_id);
    
    // 缓存公告详情（key: announcement_id）
    std::optional<std::string> get_announcement(int announcement_id);
    void put_announcement(int announcement_id, const std::string& announcement_json, std::optional<std::uint32_t> expire_seconds = std::nullopt);
    void remove_announcement(int announcement_id);
    void clear_announcements();
    
    // 缓存公告列表（key: filter_hash）
    std::optional<std::string> get_announcement_list(const std::string& filter_hash);
    void put_announcement_list(const std::string& filter_hash, const std::string& announcements_json, std::optional<std::uint32_t> expire_seconds = std::nullopt);
    void remove_announcement_list(const std::string& filter_hash);
    
    // 缓存阅读记录（key: user_id_announcement_id）
    std::optional<std::string> get_read_receipt(int user_id, int announcement_id);
    void put_read_receipt(int user_id, int announcement_id, const std::string& receipt_json, std::optional<std::uint32_t> expire_seconds = std::nullopt);
    void remove_read_receipt(int user_id, int announcement_id);
    
    // 清理所有过期条目
    void evict_all_expired();
    
    // 清理所有缓存
    void clear_all();
    
    // 统计信息
    struct CacheStats {
        size_t user_cache_size;
        size_t announcement_cache_size;
        size_t announcement_list_cache_size;
        size_t read_receipt_cache_size;
    };
    
    CacheStats get_stats() const;
    
    bool is_enabled() const;
    void set_enabled(bool enable);
    
private:
    CacheManager();
    
    template <typename K, typename V>
    std::shared_ptr<LRUCache<K, V>> get_cache(CacheType type);
    
    template <typename K, typename V>
    void create_cache(CacheType type, size_t max_entries, std::uint32_t default_expire_seconds);
    
    mutable std::mutex mutex_;
    bool enabled_ = false;
    CacheConfig config_;
    
    std::unordered_map<CacheType, std::shared_ptr<void>> caches_;
    
    std::string get_read_receipt_key(int user_id, int announcement_id) const;
};

} // namespace cache
