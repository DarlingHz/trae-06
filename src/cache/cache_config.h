#pragma once

#include <string>
#include <cstdint>

namespace cache {

struct CacheConfig {
    std::string type;                  // 缓存类型: "memory", "redis"等
    std::uint64_t max_size;            // 最大缓存大小（字节）
    std::uint32_t max_entries;         // 最大缓存条目数
    std::uint32_t default_expire_seconds; // 默认过期时间（秒）
    bool enable;                       // 是否启用缓存
    
    CacheConfig() 
        : max_size(100 * 1024 * 1024), // 默认100MB
          max_entries(1000),
          default_expire_seconds(300), // 默认5分钟
          enable(true) {}
};

} // namespace cache
