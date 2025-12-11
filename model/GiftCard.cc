#include "GiftCard.h"

namespace giftcard {

bool GiftCard::isExpired() const {
    auto now = std::chrono::system_clock::now();
    return now > valid_to_;
}

bool GiftCard::isAvailable() const {
    if (isExpired()) {
        return false;
    }
    return status_ == GiftCardStatus::AVAILABLE;
}

json GiftCard::toJson() const {
    json j;
    j["id"] = id_;
    j["card_no"] = card_no_;
    j["user_id"] = user_id_;
    j["template_id"] = template_id_;
    j["balance"] = balance_;
    j["discount_rate"] = discount_rate_;
    j["valid_from"] = std::chrono::system_clock::to_time_t(valid_from_);
    j["valid_to"] = std::chrono::system_clock::to_time_t(valid_to_);
    j["status"] = status_;
    j["version"] = version_;
    j["created_at"] = std::chrono::system_clock::to_time_t(created_at_);
    j["updated_at"] = std::chrono::system_clock::to_time_t(updated_at_);
    return j;
}

GiftCard GiftCard::fromJson(const json& j) {
    GiftCard card;
    if (j.contains("id")) card.setId(j["id"]);
    if (j.contains("card_no")) card.setCardNo(j["card_no"]);
    if (j.contains("user_id")) card.setUserId(j["user_id"]);
    if (j.contains("template_id")) card.setTemplateId(j["template_id"]);
    if (j.contains("balance")) card.setBalance(j["balance"]);
    if (j.contains("discount_rate")) card.setDiscountRate(j["discount_rate"]);
    if (j.contains("valid_from")) {
        auto valid_from = std::chrono::system_clock::from_time_t(j["valid_from"]);
        card.setValidFrom(valid_from);
    }
    if (j.contains("valid_to")) {
        auto valid_to = std::chrono::system_clock::from_time_t(j["valid_to"]);
        card.setValidTo(valid_to);
    }
    if (j.contains("status")) card.setStatus(j["status"]);
    if (j.contains("version")) card.setVersion(j["version"]);
    if (j.contains("created_at")) {
        auto created_at = std::chrono::system_clock::from_time_t(j["created_at"]);
        card.setCreatedAt(created_at);
    }
    if (j.contains("updated_at")) {
        auto updated_at = std::chrono::system_clock::from_time_t(j["updated_at"]);
        card.setUpdatedAt(updated_at);
    }
    return card;
}

} // namespace giftcard
