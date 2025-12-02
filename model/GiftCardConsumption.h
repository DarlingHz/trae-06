#ifndef GIFT_CARD_CONSUMPTION_H
#define GIFT_CARD_CONSUMPTION_H

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace giftcard {

class GiftCardConsumption {
public:
    GiftCardConsumption() = default;
    ~GiftCardConsumption() = default;

    // Getter and Setter methods
    uint64_t getId() const { return id_; }
    void setId(uint64_t id) { id_ = id; }

    uint64_t getCardId() const { return card_id_; }
    void setCardId(uint64_t card_id) { card_id_ = card_id; }

    uint64_t getUserId() const { return user_id_; }
    void setUserId(uint64_t user_id) { user_id_ = user_id; }

    uint64_t getOrderId() const { return order_id_; }
    void setOrderId(uint64_t order_id) { order_id_ = order_id; }

    double getConsumeAmount() const { return consume_amount_; }
    void setConsumeAmount(double consume_amount) { consume_amount_ = consume_amount; }

    const std::chrono::system_clock::time_point& getConsumeTime() const { return consume_time_; }
    void setConsumeTime(const std::chrono::system_clock::time_point& consume_time) { consume_time_ = consume_time; }

    // 转换为JSON
    json toJson() const;

    // 从JSON转换
    static GiftCardConsumption fromJson(const json& j);

private:
    uint64_t id_ = 0;
    uint64_t card_id_ = 0;
    uint64_t user_id_ = 0;
    uint64_t order_id_ = 0;
    double consume_amount_ = 0.0;
    std::chrono::system_clock::time_point consume_time_;
};

} // namespace giftcard

#endif // GIFT_CARD_CONSUMPTION_H
