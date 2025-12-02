#include "GiftCardController.h"
#include <cstdint>
#include <nlohmann/json.hpp>
#include "service/GiftCardService.h"
#include "utils/Response.h"
#include "utils/Logger.h"
#include <model/GiftCard.h>
#include <model/GiftCardConsumption.h>
#include <string>
#include <vector>
#include <memory>

using namespace giftcard;
using json = nlohmann::json;

void GiftCardController::issueGiftCards(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) {
    LOG_INFO("收到发放礼品卡请求");

    try {
        // 解析请求体JSON
        auto req_json = req->getJsonObject();
        if (!req_json) {
            LOG_ERROR("请求体不是有效的JSON格式");
            auto resp = Response::failure(400, "请求体格式错误");
            callback(resp);
            return;
        }

        // 验证请求参数
        std::string error_msg;
        if (!validateIssueGiftCardParams(req_json, error_msg)) {
            LOG_ERROR("请求参数验证失败: {}", error_msg);
            auto resp = Response::failure(400, error_msg);
            callback(resp);
            return;
        }

        // 调用服务层发放礼品卡
        GiftCardService& gift_card_service = GiftCardService::getInstance();
        uint64_t user_id = (*req_json)["user_id"].asUInt64();
        uint64_t template_id = (*req_json)["template_id"].asUInt64();
        int quantity = (*req_json)["quantity"].asInt();
        std::string request_id = (*req_json)["request_id"].asString();

        if (!gift_card_service.issueGiftCards(user_id, template_id, quantity, request_id)) {
            LOG_ERROR("发放礼品卡失败");
            auto resp = Response::failure(500, "发放礼品卡失败");
            callback(resp);
            return;
        }

        // 构建成功响应
        auto resp = Response::success({}, "礼品卡发放成功");
        callback(resp);
        LOG_INFO("礼品卡发放成功: user_id={}, template_id={}, quantity={}", user_id, template_id, quantity);

    } catch (const std::exception& e) {
        LOG_ERROR("发放礼品卡失败: {}", e.what());
        auto resp = Response::failure(500, "服务器内部错误");
        callback(resp);
    }
}

void GiftCardController::getUserGiftCards(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t user_id) {
    LOG_INFO("收到查询用户礼品卡列表请求: user_id={}", user_id);

    try {
        // 获取查询参数
        std::string status = req->getParameter("status");

        // 调用服务层查询礼品卡列表
        GiftCardService& gift_card_service = GiftCardService::getInstance();
        auto gift_cards = gift_card_service.getGiftCardsByUserId(user_id, status);

        // 构建响应数据
        json data;
        data["giftcards"] = json::array();

        for (const auto& gift_card : gift_cards) {
            json gift_card_json;
            gift_card_json["id"] = gift_card->getId();
            gift_card_json["card_no"] = gift_card->getCardNo();
            gift_card_json["template_id"] = gift_card->getTemplateId();
            gift_card_json["user_id"] = gift_card->getUserId();
            gift_card_json["balance"] = gift_card->getBalance();
            gift_card_json["status"] = gift_card->getStatus();
            gift_card_json["valid_from"] = std::chrono::system_clock::to_time_t(gift_card->getValidFrom());
            gift_card_json["valid_to"] = std::chrono::system_clock::to_time_t(gift_card->getValidTo());
            gift_card_json["created_at"] = std::chrono::system_clock::to_time_t(gift_card->getCreatedAt());
            gift_card_json["updated_at"] = std::chrono::system_clock::to_time_t(gift_card->getUpdatedAt());

            data["giftcards"].push_back(gift_card_json);
        }

        // 构建成功响应
        auto resp = Response::success(data, "查询成功");
        callback(resp);
        LOG_INFO("用户礼品卡列表查询成功: user_id={}, count={}", user_id, gift_cards.size());

    } catch (const std::exception& e) {
        LOG_ERROR("查询用户礼品卡列表失败: user_id={}, error={}", user_id, e.what());
        auto resp = Response::failure(500, "服务器内部错误");
        callback(resp);
    }
}

void GiftCardController::lockGiftCard(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t card_id) {
    LOG_INFO("收到锁定礼品卡请求: card_id={}", card_id);

    try {
        // 解析请求体JSON
        auto req_json = req->getJsonObject();
        if (!req_json) {
            LOG_ERROR("请求体不是有效的JSON格式");
            auto resp = Response::failure(400, "请求体格式错误");
            callback(resp);
            return;
        }

        // 验证请求参数
        std::string error_msg;
        if (!validateLockGiftCardParams(*req_json, error_msg)) {
            LOG_ERROR("请求参数验证失败: {}", error_msg);
            auto resp = Response::failure(400, error_msg);
            callback(resp);
            return;
        }

        // 调用服务层锁定礼品卡
        GiftCardService& gift_card_service = GiftCardService::getInstance();
        uint64_t user_id = (*req_json)["user_id"].asUInt64();
        std::string order_id = (*req_json)["order_id"].asString();
        double lock_amount = (*req_json)["lock_amount"].asDouble();
        uint32_t lock_ttl_seconds = (*req_json)["lock_ttl_seconds"].asUInt64();

        if (!gift_card_service.lockGiftCard(card_id, user_id, order_id, lock_amount, lock_ttl_seconds)) {
            LOG_ERROR("锁定礼品卡失败");
            auto resp = Response::failure(500, "锁定礼品卡失败");
            callback(resp);
            return;
        }

        // 构建成功响应
        auto resp = Response::success({}, "礼品卡锁定成功");
        callback(resp);
        LOG_INFO("礼品卡锁定成功: card_id={}, order_id={}, lock_amount={}", card_id, order_id, lock_amount);

    } catch (const std::exception& e) {
        LOG_ERROR("锁定礼品卡失败: card_id={}, error={}", card_id, e.what());
        auto resp = Response::failure(500, "服务器内部错误");
        callback(resp);
    }
}

void GiftCardController::consumeGiftCard(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t card_id) {
    LOG_INFO("收到消费礼品卡请求: card_id={}", card_id);

    try {
        // 解析请求体JSON
        auto req_json = req->getJsonObject();
        if (!req_json) {
            LOG_ERROR("请求体不是有效的JSON格式");
            auto resp = Response::failure(400, "请求体格式错误");
            callback(resp);
            return;
        }

        // 验证请求参数
        std::string error_msg;
        if (!validateConsumeGiftCardParams(*req_json, error_msg)) {
            LOG_ERROR("请求参数验证失败: {}", error_msg);
            auto resp = Response::failure(400, error_msg);
            callback(resp);
            return;
        }

        // 调用服务层消费礼品卡
        GiftCardService& gift_card_service = GiftCardService::getInstance();
        uint64_t user_id = (*req_json)["user_id"].asUInt64();
        std::string order_id = (*req_json)["order_id"].asString();
        double consume_amount = (*req_json)["consume_amount"].asDouble();
        std::string idempotency_key = (*req_json)["idempotency_key"].asString();

        if (!gift_card_service.consumeGiftCard(card_id, user_id, order_id, consume_amount, idempotency_key)) {
            LOG_ERROR("消费礼品卡失败");
            auto resp = Response::failure(500, "消费礼品卡失败");
            callback(resp);
            return;
        }

        // 构建成功响应
        auto resp = Response::success({}, "礼品卡消费成功");
        callback(resp);
        LOG_INFO("礼品卡消费成功: card_id={}, order_id={}, consume_amount={}", card_id, order_id, consume_amount);

    } catch (const std::exception& e) {
        LOG_ERROR("消费礼品卡失败: card_id={}, error={}", card_id, e.what());
        auto resp = Response::failure(500, "服务器内部错误");
        callback(resp);
    }
}

void GiftCardController::unlockGiftCard(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t card_id) {
    LOG_INFO("收到释放礼品卡锁定请求: card_id={}", card_id);

    try {
        // 解析请求体JSON
        auto req_json = req->getJsonObject();
        if (!req_json) {
            LOG_ERROR("请求体不是有效的JSON格式");
            auto resp = Response::failure(400, "请求体格式错误");
            callback(resp);
            return;
        }

        // 验证请求参数
        std::string error_msg;
        if (!validateUnlockGiftCardParams(*req_json, error_msg)) {
            LOG_ERROR("请求参数验证失败: {}", error_msg);
            auto resp = Response::failure(400, error_msg);
            callback(resp);
            return;
        }

        // 调用服务层释放礼品卡锁定
        GiftCardService& gift_card_service = GiftCardService::getInstance();
        uint64_t user_id = (*req_json)["user_id"].asUInt64();
        std::string order_id = (*req_json)["order_id"].asString();

        if (!gift_card_service.unlockGiftCard(card_id, user_id, order_id)) {
            LOG_ERROR("释放礼品卡锁定失败");
            auto resp = Response::failure(500, "释放礼品卡锁定失败");
            callback(resp);
            return;
        }

        // 构建成功响应
        auto resp = Response::success({}, "礼品卡锁定释放成功");
        callback(resp);
        LOG_INFO("礼品卡锁定释放成功: card_id={}, order_id={}", card_id, order_id);

    } catch (const std::exception& e) {
        LOG_ERROR("释放礼品卡锁定失败: card_id={}, error={}", card_id, e.what());
        auto resp = Response::failure(500, "服务器内部错误");
        callback(resp);
    }
}

void GiftCardController::freezeGiftCard(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t card_id) {
    LOG_INFO("收到冻结礼品卡请求: card_id={}", card_id);

    try {
        // 调用服务层冻结礼品卡
        GiftCardService& gift_card_service = GiftCardService::getInstance();
        if (!gift_card_service.freezeGiftCard(card_id)) {
            LOG_ERROR("冻结礼品卡失败");
            auto resp = Response::failure(500, "冻结礼品卡失败");
            callback(resp);
            return;
        }

        // 构建成功响应
        auto resp = Response::success({}, "礼品卡冻结成功");
        callback(resp);
        LOG_INFO("礼品卡冻结成功: card_id={}", card_id);

    } catch (const std::exception& e) {
        LOG_ERROR("冻结礼品卡失败: card_id={}, error={}", card_id, e.what());
        auto resp = Response::failure(500, "服务器内部错误");
        callback(resp);
    }
}

void GiftCardController::unfreezeGiftCard(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t card_id) {
    LOG_INFO("收到解冻礼品卡请求: card_id={}", card_id);

    try {
        // 调用服务层解冻礼品卡
        GiftCardService& gift_card_service = GiftCardService::getInstance();
        if (!gift_card_service.unfreezeGiftCard(card_id)) {
            LOG_ERROR("解冻礼品卡失败");
            auto resp = Response::failure(500, "解冻礼品卡失败");
            callback(resp);
            return;
        }

        // 构建成功响应
        auto resp = Response::success({}, "礼品卡解冻成功");
        callback(resp);
        LOG_INFO("礼品卡解冻成功: card_id={}", card_id);

    } catch (const std::exception& e) {
        LOG_ERROR("解冻礼品卡失败: card_id={}, error={}", card_id, e.what());
        auto resp = Response::failure(500, "服务器内部错误");
        callback(resp);
    }
}

void GiftCardController::getGiftCardConsumptions(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t card_id) {
    LOG_INFO("收到查询礼品卡消费记录请求: card_id={}", card_id);

    try {
        // 调用服务层查询礼品卡消费记录
        GiftCardService& gift_card_service = GiftCardService::getInstance();
        auto consumptions = gift_card_service.getGiftCardConsumptions(card_id);

        // 构建响应数据
        json data;
        data["consumptions"] = json::array();

        for (const auto& consumption : consumptions) {
            json consumption_json;
            consumption_json["id"] = consumption->getId();
            consumption_json["card_id"] = consumption->getCardId();
            consumption_json["user_id"] = consumption->getUserId();
            consumption_json["order_id"] = consumption->getOrderId();
            consumption_json["consume_amount"] = consumption->getConsumeAmount();
            consumption_json["consume_time"] = std::chrono::system_clock::to_time_t(consumption->getConsumeTime());

            data["consumptions"].push_back(consumption_json);
        }

        // 构建成功响应
        auto resp = Response::success(data, "查询成功");
        callback(resp);
        LOG_INFO("礼品卡消费记录查询成功: card_id={}, count={}", card_id, consumptions.size());

    } catch (const std::exception& e) {
        LOG_ERROR("查询礼品卡消费记录失败: card_id={}, error={}", card_id, e.what());
        auto resp = Response::failure(500, "服务器内部错误");
        callback(resp);
    }
}

bool GiftCardController::validateIssueGiftCardParams(const Json::Value& req_json, std::string& error_msg) {
    // 验证必填字段
    std::vector<std::string> required_fields = {"user_id", "template_id", "quantity", "request_id"};
    for (const auto& field : required_fields) {
        if (!req_json.isMember(field)) {
            error_msg = "缺少必填字段: " + field;
            return false;
        }
    }

    // 验证用户ID
    auto user_id = req_json["user_id"].asUInt64();
    if (user_id == 0) {
        error_msg = "用户ID无效";
        return false;
    }

    // 验证模板ID
    auto template_id = req_json["template_id"].asUInt64();
    if (template_id == 0) {
        error_msg = "模板ID无效";
        return false;
    }

    // 验证发放数量
    auto quantity = req_json["quantity"].asInt();
    if (quantity <= 0) {
        error_msg = "发放数量必须大于0";
        return false;
    }

    // 验证幂等键
    auto request_id = req_json["request_id"].asString();
    if (request_id.empty()) {
        error_msg = "幂等键不能为空";
        return false;
    }

    // 所有验证通过
    return true;
}

bool GiftCardController::validateLockGiftCardParams(const Json::Value& req_json, std::string& error_msg) {
    // 验证必填字段
    std::vector<std::string> required_fields = {"user_id", "order_id", "lock_amount", "lock_ttl_seconds"};
    for (const auto& field : required_fields) {
        if (!req_json.isMember(field)) {
            error_msg = "缺少必填字段: " + field;
            return false;
        }
    }

    // 验证用户ID
    auto user_id = req_json["user_id"].asUInt64();
    if (user_id == 0) {
        error_msg = "用户ID无效";
        return false;
    }

    // 验证订单ID
    auto order_id = req_json["order_id"].asString();
    if (order_id.empty()) {
        error_msg = "订单ID不能为空";
        return false;
    }

    // 验证锁定金额
    auto lock_amount = req_json["lock_amount"].asDouble();
    if (lock_amount <= 0) {
        error_msg = "锁定金额必须大于0";
        return false;
    }

    // 验证锁定TTL
    auto lock_ttl_seconds = req_json["lock_ttl_seconds"].asUInt64();
    if (lock_ttl_seconds <= 0) {
        error_msg = "锁定TTL必须大于0";
        return false;
    }

    // 所有验证通过
    return true;
}

bool GiftCardController::validateConsumeGiftCardParams(const Json::Value& req_json, std::string& error_msg) {
    // 验证必填字段
    std::vector<std::string> required_fields = {"user_id", "order_id", "consume_amount", "idempotency_key"};
    for (const auto& field : required_fields) {
        if (!req_json.isMember(field)) {
            error_msg = "缺少必填字段: " + field;
            return false;
        }
    }

    // 验证用户ID
    auto user_id = req_json["user_id"].asUInt64();
    if (user_id == 0) {
        error_msg = "用户ID无效";
        return false;
    }

    // 验证订单ID
    auto order_id = req_json["order_id"].asString();
    if (order_id.empty()) {
        error_msg = "订单ID不能为空";
        return false;
    }

    // 验证消费金额
    auto consume_amount = req_json["consume_amount"].asDouble();
    if (consume_amount <= 0) {
        error_msg = "消费金额必须大于0";
        return false;
    }

    // 验证幂等键
    auto idempotency_key = req_json["idempotency_key"].asString();
    if (idempotency_key.empty()) {
        error_msg = "幂等键不能为空";
        return false;
    }

    // 所有验证通过
    return true;
}

bool GiftCardController::validateUnlockGiftCardParams(const Json::Value& req_json, std::string& error_msg) {
    // 验证必填字段
    std::vector<std::string> required_fields = {"user_id", "order_id"};
    for (const auto& field : required_fields) {
        if (!req_json.isMember(field)) {
            error_msg = "缺少必填字段: " + field;
            return false;
        }
    }

    // 验证用户ID
    auto user_id = req_json["user_id"].asUInt64();
    if (user_id == 0) {
        error_msg = "用户ID无效";
        return false;
    }

    // 验证订单ID
    auto order_id = req_json["order_id"].asString();
    if (order_id.empty()) {
        error_msg = "订单ID不能为空";
        return false;
    }

    // 所有验证通过
    return true;
}
