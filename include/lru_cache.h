#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <map>
#include <list>
#include <mutex>
#include <optional>

/**
 * @brief 线程安全的LRU缓存类
 * @tparam KeyType 缓存键的类型
 * @tparam ValueType 缓存值的类型
 */
template<typename KeyType, typename ValueType>
class LRUCache {
public:
    /**
     * @brief 构造函数
     * @param capacity 缓存容量
     */
    explicit LRUCache(size_t capacity);

    /**
     * @brief 从缓存中获取值
     * @param key 缓存键
     * @return 缓存值，如果不存在则返回std::nullopt
     */
    std::optional<ValueType> get(const KeyType& key);

    /**
     * @brief 向缓存中添加值
     * @param key 缓存键
     * @param value 缓存值
     */
    void put(const KeyType& key, const ValueType& value);

    /**
     * @brief 从缓存中移除值
     * @param key 缓存键
     */
    void remove(const KeyType& key);

    /**
     * @brief 清空缓存
     */
    void clear();

    /**
     * @brief 获取缓存的当前大小
     * @return 缓存大小
     */
    size_t size() const;

    /**
     * @brief 获取缓存的容量
     * @return 缓存容量
     */
    size_t capacity() const;

private:
    using ListIterator = typename std::list<std::pair<KeyType, ValueType>>::iterator;

    size_t capacity_;                                  ///< 缓存容量
    std::list<std::pair<KeyType, ValueType>> list_; ///< 双向链表，存储键值对
    std::map<KeyType, ListIterator> map_;  ///< 有序映射，键到链表迭代器的映射
    mutable std::mutex mutex_;                        ///< 互斥锁，保证线程安全
};

#include "lru_cache.tpp" // 包含模板实现

#endif // LRU_CACHE_H