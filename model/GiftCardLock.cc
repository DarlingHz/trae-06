#include "GiftCardLock.h"

namespace giftcard {

bool GiftCardLock::isExpired() const {
    auto now = std::chrono::system_clock::now();
    return now > lock_ttl_;
}

bool GiftCardLock::isActive() const {
    if (isExpired()) {
        return false;
    }
    return status_ == LockStatus::ACTIVE;
}

json GiftCardLock::toJson() const {
    json j;
    j["id"] = id_;
    j["card_id"] = card_id_;
    j["user_id"] = user_id_;
    j["order_id"] = order_id_;
    j["lock_amount"] = lock_amount_;
    j["lock_ttl"] = std::chrono::system_clock::to_time_t(lock_ttl_);
    j["status"] = status_;
    j["created_at"] = std::chrono::system_clock::to_time_t(created_at_);
    j["updated_at"] = std::chrono::system_clock::to_time_t(updated_at_);
    return j;
}

GiftCardLock GiftCardLock::fromJson(const json& j) {
    GiftCardLock lock;
    if (j.contains("id")) lock.setId(j["id"]);
    if (j.contains("card_id")) lock.setCardId(j["card_id"]);
    if (j.contains("user_id")) lock.setUserId(j["user_id"]);
    if (j.contains("order_id")) lock.setOrderId(j["order_id"]);
    if (j.contains("lock_amount")) lock.setLockAmount(j["lock_amount"]);
    if (j.contains("lock_ttl")) {
        auto lock_ttl = std::chrono::system_clock::from_time_t(j["lock_ttl"]);
        lock.setLockTtl(lock_ttl);
    }
    if (j.contains("status")) lock.setStatus(j["status"]);
    if (j.contains("created_at")) {
        auto created_at = std::chrono::system_clock::from_time_t(j["created_at"]);
        lock.setCreatedAt(created_at);
    }
    if (j.contains("updated_at")) {
        auto updated_at = std::chrono::system_clock::from_time_t(j["updated_at"]);
        lock.setUpdatedAt(updated_at);
    }
    return lock;
}

} // namespace giftcard
