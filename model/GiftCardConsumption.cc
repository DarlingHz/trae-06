#include "GiftCardConsumption.h"

namespace giftcard {

json GiftCardConsumption::toJson() const {
    json j;
    j["id"] = id_;
    j["card_id"] = card_id_;
    j["user_id"] = user_id_;
    j["order_id"] = order_id_;
    j["consume_amount"] = consume_amount_;
    j["consume_time"] = std::chrono::system_clock::to_time_t(consume_time_);
    return j;
}

GiftCardConsumption GiftCardConsumption::fromJson(const json& j) {
    GiftCardConsumption consumption;
    if (j.contains("id")) consumption.setId(j["id"]);
    if (j.contains("card_id")) consumption.setCardId(j["card_id"]);
    if (j.contains("user_id")) consumption.setUserId(j["user_id"]);
    if (j.contains("order_id")) consumption.setOrderId(j["order_id"]);
    if (j.contains("consume_amount")) consumption.setConsumeAmount(j["consume_amount"]);
    if (j.contains("consume_time")) {
        auto consume_time = std::chrono::system_clock::from_time_t(j["consume_time"]);
        consumption.setConsumeTime(consume_time);
    }
    return consumption;
}

} // namespace giftcard
