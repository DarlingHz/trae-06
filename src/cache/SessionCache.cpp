#include "SessionCache.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std::chrono;

// 构造函数，设置缓存过期时间（秒）
SessionCache::SessionCache(int expireSeconds) : m_expireSeconds(expireSeconds) {
}

// 缓存查询结果
void SessionCache::cacheResults(const std::string& key, const oatpp::List<oatpp::Object<ClassSessionDto>>& results) {
    std::lock_guard<std::mutex> lock(m_mutex);
    CacheEntry entry;
    entry.results = results;
    entry.timestamp = system_clock::now();
    m_cache[key] = entry;
}

// 获取缓存结果，如果不存在或已过期则返回nullptr
oatpp::List<oatpp::Object<ClassSessionDto>> SessionCache::getResults(const std::string& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_cache.find(key);
    if (it == m_cache.end()) {
        return nullptr;
    }

    // 检查是否过期
    auto now = system_clock::now();
    auto duration = duration_cast<seconds>(now - it->second.timestamp);
    if (duration.count() > m_expireSeconds) {
        m_cache.erase(it);
        return nullptr;
    }

    return it->second.results;
}

// 清空所有缓存
void SessionCache::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cache.clear();
}

// 生成缓存键
std::string SessionCache::generateKey(const std::string& from, const std::string& to, 
                                         const oatpp::Int32& coachId, 
                                         const oatpp::Int32& templateId) {
    std::ostringstream keyStream;
    keyStream << "from=" << from << ",to=" << to;
    
    if (coachId > 0) {
        keyStream << ",coach_id=" << coachId;
    }
    
    if (templateId > 0) {
        keyStream << ",template_id=" << templateId;
    }
    
    return keyStream.str();
}
