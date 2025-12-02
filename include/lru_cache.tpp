#ifndef LRU_CACHE_TPP
#define LRU_CACHE_TPP

#include "lru_cache.h"

template<typename KeyType, typename ValueType>
LRUCache<KeyType, ValueType>::LRUCache(size_t capacity) : capacity_(capacity) {
}

template<typename KeyType, typename ValueType>
std::optional<ValueType> LRUCache<KeyType, ValueType>::get(const KeyType& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = map_.find(key);
    if (it == map_.end()) {
        return std::nullopt;
    }

    // 将访问的元素移到链表头部
    list_.splice(list_.begin(), list_, it->second);

    return it->second->second;
}

template<typename KeyType, typename ValueType>
void LRUCache<KeyType, ValueType>::put(const KeyType& key, const ValueType& value) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = map_.find(key);
    if (it != map_.end()) {
        // 更新现有元素的值
        it->second->second = value;
        // 将元素移到链表头部
        list_.splice(list_.begin(), list_, it->second);
        return;
    }

    // 检查缓存是否已满
    if (list_.size() >= capacity_) {
        // 移除链表尾部的元素（最近最少使用）
        auto last = list_.back();
        map_.erase(last.first);
        list_.pop_back();
    }

    // 添加新元素到链表头部
    list_.emplace_front(key, value);
    map_[key] = list_.begin();
}

template<typename KeyType, typename ValueType>
void LRUCache<KeyType, ValueType>::remove(const KeyType& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = map_.find(key);
    if (it != map_.end()) {
        list_.erase(it->second);
        map_.erase(it);
    }
}

template<typename KeyType, typename ValueType>
void LRUCache<KeyType, ValueType>::clear() {
    std::lock_guard<std::mutex> lock(mutex_);

    list_.clear();
    map_.clear();
}

template<typename KeyType, typename ValueType>
size_t LRUCache<KeyType, ValueType>::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return list_.size();
}

template<typename KeyType, typename ValueType>
size_t LRUCache<KeyType, ValueType>::capacity() const {
    return capacity_;
}

#endif // LRU_CACHE_TPP