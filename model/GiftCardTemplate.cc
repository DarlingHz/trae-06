#include "GiftCardTemplate.h"

namespace giftcard {

json GiftCardTemplate::toJson() const {
    json j;
    j["id"] = id_;
    j["name"] = name_;
    j["type"] = type_;
    j["face_value"] = face_value_;
    j["min_order_amount"] = min_order_amount_;
    j["total_stock"] = total_stock_;
    j["issued_count"] = issued_count_;
    j["per_user_limit"] = per_user_limit_;
    j["valid_from"] = std::chrono::system_clock::to_time_t(valid_from_);
    j["valid_to"] = std::chrono::system_clock::to_time_t(valid_to_);
    j["status"] = status_;
    j["created_at"] = std::chrono::system_clock::to_time_t(created_at_);
    j["updated_at"] = std::chrono::system_clock::to_time_t(updated_at_);
    return j;
}

GiftCardTemplate GiftCardTemplate::fromJson(const json& j) {
    GiftCardTemplate template_;
    if (j.contains("id")) template_.setId(j["id"]);
    if (j.contains("name")) template_.setName(j["name"]);
    if (j.contains("type")) template_.setType(j["type"]);
    if (j.contains("face_value")) template_.setFaceValue(j["face_value"]);
    if (j.contains("min_order_amount")) template_.setMinOrderAmount(j["min_order_amount"]);
    if (j.contains("total_stock")) template_.setTotalStock(j["total_stock"]);
    if (j.contains("issued_count")) template_.setIssuedCount(j["issued_count"]);
    if (j.contains("per_user_limit")) template_.setPerUserLimit(j["per_user_limit"]);
    if (j.contains("valid_from")) {
        auto valid_from = std::chrono::system_clock::from_time_t(j["valid_from"]);
        template_.setValidFrom(valid_from);
    }
    if (j.contains("valid_to")) {
        auto valid_to = std::chrono::system_clock::from_time_t(j["valid_to"]);
        template_.setValidTo(valid_to);
    }
    if (j.contains("status")) template_.setStatus(j["status"]);
    if (j.contains("created_at")) {
        auto created_at = std::chrono::system_clock::from_time_t(j["created_at"]);
        template_.setCreatedAt(created_at);
    }
    if (j.contains("updated_at")) {
        auto updated_at = std::chrono::system_clock::from_time_t(j["updated_at"]);
        template_.setUpdatedAt(updated_at);
    }
    return template_;
}

} // namespace giftcard
