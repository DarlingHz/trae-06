#ifndef GIFT_CARD_TEMPLATE_H
#define GIFT_CARD_TEMPLATE_H

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace giftcard {

enum class TemplateType {
    AMOUNT,    // 固定面额
    DISCOUNT   // 折扣
};

enum class TemplateStatus {
    ACTIVE,    // 活跃
    CLOSED     // 已关闭
};

class GiftCardTemplate {
public:
    GiftCardTemplate() = default;
    ~GiftCardTemplate() = default;

    // Getter and Setter methods
    uint64_t getId() const { return id_; }
    void setId(uint64_t id) { id_ = id; }

    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

    TemplateType getType() const { return type_; }
    void setType(TemplateType type) { type_ = type; }

    double getFaceValue() const { return face_value_; }
    void setFaceValue(double face_value) { face_value_ = face_value; }

    double getMinOrderAmount() const { return min_order_amount_; }
    void setMinOrderAmount(double min_order_amount) { min_order_amount_ = min_order_amount; }

    uint32_t getTotalStock() const { return total_stock_; }
    void setTotalStock(uint32_t total_stock) { total_stock_ = total_stock; }

    uint32_t getIssuedCount() const { return issued_count_; }
    void setIssuedCount(uint32_t issued_count) { issued_count_ = issued_count; }

    uint32_t getPerUserLimit() const { return per_user_limit_; }
    void setPerUserLimit(uint32_t per_user_limit) { per_user_limit_ = per_user_limit; }

    const std::chrono::system_clock::time_point& getValidFrom() const { return valid_from_; }
    void setValidFrom(const std::chrono::system_clock::time_point& valid_from) { valid_from_ = valid_from; }

    const std::chrono::system_clock::time_point& getValidTo() const { return valid_to_; }
    void setValidTo(const std::chrono::system_clock::time_point& valid_to) { valid_to_ = valid_to; }

    TemplateStatus getStatus() const { return status_; }
    void setStatus(TemplateStatus status) { status_ = status; }

    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at_; }
    void setCreatedAt(const std::chrono::system_clock::time_point& created_at) { created_at_ = created_at; }

    const std::chrono::system_clock::time_point& getUpdatedAt() const { return updated_at_; }
    void setUpdatedAt(const std::chrono::system_clock::time_point& updated_at) { updated_at_ = updated_at; }

    // 转换为JSON
    json toJson() const;

    // 从JSON转换
    static GiftCardTemplate fromJson(const json& j);

private:
    uint64_t id_ = 0;
    std::string name_;
    TemplateType type_ = TemplateType::AMOUNT;
    double face_value_ = 0.0;
    double min_order_amount_ = 0.0;
    uint32_t total_stock_ = 0;
    uint32_t issued_count_ = 0;
    uint32_t per_user_limit_ = 1;
    std::chrono::system_clock::time_point valid_from_;
    std::chrono::system_clock::time_point valid_to_;
    TemplateStatus status_ = TemplateStatus::ACTIVE;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point updated_at_;
};

// 枚举类型的JSON序列化和反序列化
NLOHMANN_JSON_SERIALIZE_ENUM(TemplateType, {
    {TemplateType::AMOUNT, "amount"},
    {TemplateType::DISCOUNT, "discount"},
});

NLOHMANN_JSON_SERIALIZE_ENUM(TemplateStatus, {
    {TemplateStatus::ACTIVE, "active"},
    {TemplateStatus::CLOSED, "closed"},
});

} // namespace giftcard

#endif // GIFT_CARD_TEMPLATE_H
