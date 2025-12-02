#ifndef GIFT_CARD_H
#define GIFT_CARD_H

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace giftcard {

enum class GiftCardStatus {
    AVAILABLE,  // 可用
    LOCKED,     // 已锁定
    USED,       // 已使用
    EXPIRED,    // 已过期
    FROZEN      // 已冻结
};

class GiftCard {
public:
    GiftCard() = default;
    ~GiftCard() = default;

    // Getter and Setter methods
    uint64_t getId() const { return id_; }
    void setId(uint64_t id) { id_ = id; }

    const std::string& getCardNo() const { return card_no_; }
    void setCardNo(const std::string& card_no) { card_no_ = card_no; }

    uint64_t getUserId() const { return user_id_; }
    void setUserId(uint64_t user_id) { user_id_ = user_id; }

    uint64_t getTemplateId() const { return template_id_; }
    void setTemplateId(uint64_t template_id) { template_id_ = template_id; }

    double getBalance() const { return balance_; }
    void setBalance(double balance) { balance_ = balance; }

    double getDiscountRate() const { return discount_rate_; }
    void setDiscountRate(double discount_rate) { discount_rate_ = discount_rate; }

    const std::chrono::system_clock::time_point& getValidFrom() const { return valid_from_; }
    void setValidFrom(const std::chrono::system_clock::time_point& valid_from) { valid_from_ = valid_from; }

    const std::chrono::system_clock::time_point& getValidTo() const { return valid_to_; }
    void setValidTo(const std::chrono::system_clock::time_point& valid_to) { valid_to_ = valid_to; }

    GiftCardStatus getStatus() const { return status_; }
    void setStatus(GiftCardStatus status) { status_ = status; }

    uint32_t getVersion() const { return version_; }
    void setVersion(uint32_t version) { version_ = version; }

    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at_; }
    void setCreatedAt(const std::chrono::system_clock::time_point& created_at) { created_at_ = created_at; }

    const std::chrono::system_clock::time_point& getUpdatedAt() const { return updated_at_; }
    void setUpdatedAt(const std::chrono::system_clock::time_point& updated_at) { updated_at_ = updated_at; }

    // 检查礼品卡是否过期
    bool isExpired() const;

    // 检查礼品卡是否可用
    bool isAvailable() const;

    // 转换为JSON
    json toJson() const;

    // 从JSON转换
    static GiftCard fromJson(const json& j);

private:
    uint64_t id_ = 0;
    std::string card_no_;
    uint64_t user_id_ = 0;
    uint64_t template_id_ = 0;
    double balance_ = 0.0;
    double discount_rate_ = 0.0;
    std::chrono::system_clock::time_point valid_from_;
    std::chrono::system_clock::time_point valid_to_;
    GiftCardStatus status_ = GiftCardStatus::AVAILABLE;
    uint32_t version_ = 1;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point updated_at_;
};

// 枚举类型的JSON序列化和反序列化
NLOHMANN_JSON_SERIALIZE_ENUM(GiftCardStatus, {
    {GiftCardStatus::AVAILABLE, "available"},
    {GiftCardStatus::LOCKED, "locked"},
    {GiftCardStatus::USED, "used"},
    {GiftCardStatus::EXPIRED, "expired"},
    {GiftCardStatus::FROZEN, "frozen"},
});

} // namespace giftcard

#endif // GIFT_CARD_H
