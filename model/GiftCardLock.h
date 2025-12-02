#ifndef GIFT_CARD_LOCK_H
#define GIFT_CARD_LOCK_H

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace giftcard {

enum class LockStatus {
    ACTIVE,     // 活跃
    CONSUMED,   // 已消费
    RELEASED    // 已释放
};

class GiftCardLock {
public:
    GiftCardLock() = default;
    ~GiftCardLock() = default;

    // Getter and Setter methods
    uint64_t getId() const { return id_; }
    void setId(uint64_t id) { id_ = id; }

    uint64_t getCardId() const { return card_id_; }
    void setCardId(uint64_t card_id) { card_id_ = card_id; }

    uint64_t getUserId() const { return user_id_; }
    void setUserId(uint64_t user_id) { user_id_ = user_id; }

    uint64_t getOrderId() const { return order_id_; }
    void setOrderId(uint64_t order_id) { order_id_ = order_id; }

    double getLockAmount() const { return lock_amount_; }
    void setLockAmount(double lock_amount) { lock_amount_ = lock_amount; }

    const std::chrono::system_clock::time_point& getLockTtl() const { return lock_ttl_; }
    void setLockTtl(const std::chrono::system_clock::time_point& lock_ttl) { lock_ttl_ = lock_ttl; }

    LockStatus getStatus() const { return status_; }
    void setStatus(LockStatus status) { status_ = status; }

    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at_; }
    void setCreatedAt(const std::chrono::system_clock::time_point& created_at) { created_at_ = created_at; }

    const std::chrono::system_clock::time_point& getUpdatedAt() const { return updated_at_; }
    void setUpdatedAt(const std::chrono::system_clock::time_point& updated_at) { updated_at_ = updated_at; }

    // 检查锁定是否过期
    bool isExpired() const;

    // 检查锁定是否活跃
    bool isActive() const;

    // 转换为JSON
    json toJson() const;

    // 从JSON转换
    static GiftCardLock fromJson(const json& j);

private:
    uint64_t id_ = 0;
    uint64_t card_id_ = 0;
    uint64_t user_id_ = 0;
    uint64_t order_id_ = 0;
    double lock_amount_ = 0.0;
    std::chrono::system_clock::time_point lock_ttl_;
    LockStatus status_ = LockStatus::ACTIVE;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point updated_at_;
};

// 枚举类型的JSON序列化和反序列化
NLOHMANN_JSON_SERIALIZE_ENUM(LockStatus, {
    {LockStatus::ACTIVE, "active"},
    {LockStatus::CONSUMED, "consumed"},
    {LockStatus::RELEASED, "released"},
});

} // namespace giftcard

#endif // GIFT_CARD_LOCK_H
