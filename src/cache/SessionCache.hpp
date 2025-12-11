#ifndef SessionCache_hpp
#define SessionCache_hpp

#include <unordered_map>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include "../dto/DTOs.hpp"

class SessionCache {
public:
    // 构造函数，设置缓存过期时间（秒）
    explicit SessionCache(int expireSeconds = 30);

    // 缓存查询结果
    void cacheResults(const std::string& key, const oatpp::List<oatpp::Object<ClassSessionDto>>& results);

    // 获取缓存结果，如果不存在或已过期则返回nullptr
    oatpp::List<oatpp::Object<ClassSessionDto>> getResults(const std::string& key);

    // 清空所有缓存
    void clear();

    // 生成缓存键
    static std::string generateKey(const std::string& from, const std::string& to, 
                                     const oatpp::Int32& coachId = 0, 
                                     const oatpp::Int32& templateId = 0);

private:
    // 缓存条目
    struct CacheEntry {
        oatpp::List<oatpp::Object<ClassSessionDto>> results;
        std::chrono::system_clock::time_point timestamp;
    };

    // 缓存容器
    std::unordered_map<std::string, CacheEntry> m_cache;

    // 缓存过期时间（秒）
    int m_expireSeconds;

    // 互斥锁，保证线程安全
    std::mutex m_mutex;
};

#endif /* SessionCache_hpp */
