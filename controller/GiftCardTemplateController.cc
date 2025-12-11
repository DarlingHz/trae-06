#include "GiftCardTemplateController.h"
#include "service/GiftCardTemplateService.h"
#include "utils/Response.h"
#include "utils/Logger.h"
#include <model/GiftCardTemplate.h>
#include <string>
#include <vector>
#include <memory>

using namespace giftcard;
using json = nlohmann::json;

void GiftCardTemplateController::createTemplate(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) {
    LOG_INFO("收到创建礼品卡模板请求");

    try {
        // 解析请求体JSON
        auto req_json = req->getJsonObject();
        if (!req_json) {
            LOG_ERROR("请求体不是有效的JSON格式");
            auto resp = Response::failure("请求体格式错误");
            callback(resp);
            return;
        }

        // 验证请求参数
        std::string error_msg;
        if (!validateCreateTemplateParams(*req_json, error_msg)) {
            LOG_ERROR("请求参数验证失败: {}", error_msg);
            auto resp = Response::failure(error_msg);
            callback(resp);
            return;
        }

        // 创建模板对象
        auto template_info = std::make_shared<GiftCardTemplate>();
        template_info->setName((*req_json)["name"].get<std::string>());
        template_info->setType((*req_json)["type"].get<std::string>());
        template_info->setFaceValue((*req_json)["face_value"].get<double>());

        if (req_json->contains("min_order_amount")) {
            template_info->setMinOrderAmount((*req_json)["min_order_amount"].get<double>());
        }

        template_info->setTotalStock((*req_json)["total_stock"].get<int>());

        if (req_json->contains("per_user_limit")) {
            template_info->setPerUserLimit((*req_json)["per_user_limit"].get<int>());
        }

        // 解析有效期
        auto valid_from_str = (*req_json)["valid_from"].get<std::string>();
        auto valid_to_str = (*req_json)["valid_to"].get<std::string>();
        auto valid_from = std::chrono::system_clock::from_time_t(std::stoll(valid_from_str));
        auto valid_to = std::chrono::system_clock::from_time_t(std::stoll(valid_to_str));
        template_info->setValidFrom(valid_from);
        template_info->setValidTo(valid_to);

        // 调用服务层创建模板
        auto template_service = GiftCardTemplateService::getInstance();
        uint64_t template_id = template_service.createTemplate(template_info);
        if (template_id == 0) {
            LOG_ERROR("创建礼品卡模板失败");
            auto resp = Response::failure("创建模板失败");
            callback(resp);
            return;
        }

        // 构建成功响应
        json data;
        data["template_id"] = template_id;
        auto resp = Response::success(data, "模板创建成功");
        callback(resp);
        LOG_INFO("礼品卡模板创建成功: template_id={}", template_id);

    } catch (const std::exception& e) {
        LOG_ERROR("创建礼品卡模板失败: {}", e.what());
        auto resp = Response::failure("服务器内部错误");
        callback(resp);
    }
}

void GiftCardTemplateController::getTemplates(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) {
    LOG_INFO("收到查询礼品卡模板列表请求");

    try {
        // 获取查询参数
        std::string name = req->getParameter("name");
        std::string status = req->getParameter("status");
        int page = req->getParameter("page").empty() ? 1 : std::stoi(req->getParameter("page"));
        int page_size = req->getParameter("page_size").empty() ? 10 : std::stoi(req->getParameter("page_size"));

        // 调用服务层查询模板列表
        auto template_service = GiftCardTemplateService::getInstance();
        auto templates = template_service.getTemplates(name, status, page, page_size);

        // 构建响应数据
        json data;
        data["templates"] = json::array();

        for (const auto& template_info : templates) {
            json template_json;
            template_json["id"] = template_info->getId();
            template_json["name"] = template_info->getName();
            template_json["type"] = template_info->getType();
            template_json["face_value"] = template_info->getFaceValue();
            template_json["min_order_amount"] = template_info->getMinOrderAmount();
            template_json["total_stock"] = template_info->getTotalStock();
            template_json["issued_count"] = template_info->getIssuedCount();
            template_json["per_user_limit"] = template_info->getPerUserLimit();
            template_json["valid_from"] = std::chrono::system_clock::to_time_t(template_info->getValidFrom());
            template_json["valid_to"] = std::chrono::system_clock::to_time_t(template_info->getValidTo());
            template_json["status"] = template_info->getStatus();
            template_json["created_at"] = std::chrono::system_clock::to_time_t(template_info->getCreatedAt());
            template_json["updated_at"] = std::chrono::system_clock::to_time_t(template_info->getUpdatedAt());

            data["templates"].push_back(template_json);
        }

        data["page"] = page;
        data["page_size"] = page_size;
        data["total_count"] = templates.size(); // 简化处理，实际应该从数据库获取总数量

        // 构建成功响应
        auto resp = Response::success(data, "查询成功");
        callback(resp);
        LOG_INFO("礼品卡模板列表查询成功: count={}", templates.size());

    } catch (const std::exception& e) {
        LOG_ERROR("查询礼品卡模板列表失败: {}", e.what());
        auto resp = Response::failure("服务器内部错误");
        callback(resp);
    }
}

void GiftCardTemplateController::getTemplateById(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t template_id) {
    LOG_INFO("收到查询礼品卡模板详情请求: template_id={}", template_id);

    try {
        // 调用服务层查询模板详情
        auto template_service = GiftCardTemplateService::getInstance();
        auto template_info = template_service.getTemplateById(template_id);
        if (!template_info) {
            LOG_ERROR("礼品卡模板不存在: template_id={}", template_id);
            auto resp = Response::failure("模板不存在", 404);
            callback(resp);
            return;
        }

        // 构建响应数据
        json data;
        data["id"] = template_info->getId();
        data["name"] = template_info->getName();
        data["type"] = template_info->getType();
        data["face_value"] = template_info->getFaceValue();
        data["min_order_amount"] = template_info->getMinOrderAmount();
        data["total_stock"] = template_info->getTotalStock();
        data["issued_count"] = template_info->getIssuedCount();
        data["per_user_limit"] = template_info->getPerUserLimit();
        data["valid_from"] = std::chrono::system_clock::to_time_t(template_info->getValidFrom());
        data["valid_to"] = std::chrono::system_clock::to_time_t(template_info->getValidTo());
        data["status"] = template_info->getStatus();
        data["created_at"] = std::chrono::system_clock::to_time_t(template_info->getCreatedAt());
        data["updated_at"] = std::chrono::system_clock::to_time_t(template_info->getUpdatedAt());

        // 构建成功响应
        auto resp = Response::success(data, "查询成功");
        callback(resp);
        LOG_INFO("礼品卡模板详情查询成功: template_id={}", template_id);

    } catch (const std::exception& e) {
        LOG_ERROR("查询礼品卡模板详情失败: template_id={}, error={}", template_id, e.what());
        auto resp = Response::failure("服务器内部错误");
        callback(resp);
    }
}

void GiftCardTemplateController::closeTemplate(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t template_id) {
    LOG_INFO("收到关闭礼品卡模板请求: template_id={}", template_id);

    try {
        // 调用服务层关闭模板
        auto template_service = GiftCardTemplateService::getInstance();
        if (!template_service.closeTemplate(template_id)) {
            LOG_ERROR("关闭礼品卡模板失败: template_id={}", template_id);
            auto resp = Response::failure("关闭模板失败");
            callback(resp);
            return;
        }

        // 构建成功响应
        auto resp = Response::success({}, "模板关闭成功");
        callback(resp);
        LOG_INFO("礼品卡模板关闭成功: template_id={}", template_id);

    } catch (const std::exception& e) {
        LOG_ERROR("关闭礼品卡模板失败: template_id={}, error={}", template_id, e.what());
        auto resp = Response::failure("服务器内部错误");
        callback(resp);
    }
}

bool GiftCardTemplateController::validateCreateTemplateParams(const json& req_json, std::string& error_msg) {
    // 验证必填字段
    std::vector<std::string> required_fields = {"name", "type", "face_value", "total_stock", "valid_from", "valid_to"};
    for (const auto& field : required_fields) {
        if (!req_json.contains(field)) {
            error_msg = "缺少必填字段: " + field;
            return false;
        }
    }

    // 验证名称长度
    auto name = req_json["name"].get<std::string>();
    if (name.empty() || name.size() > 100) {
        error_msg = "模板名称长度必须在1-100个字符之间";
        return false;
    }

    // 验证类型
    auto type = req_json["type"].get<std::string>();
    if (type != "amount" && type != "discount") {
        error_msg = "模板类型必须是'amount'或'discount'";
        return false;
    }

    // 验证面额
    auto face_value = req_json["face_value"].get<double>();
    if (face_value <= 0) {
        error_msg = "面额必须大于0";
        return false;
    }

    // 验证折扣范围（如果是折扣类型）
    if (type == "discount" && (face_value < 1 || face_value > 100)) {
        error_msg = "折扣百分比必须在1-100之间";
        return false;
    }

    // 验证最低订单金额（如果提供）
    if (req_json.contains("min_order_amount")) {
        auto min_order_amount = req_json["min_order_amount"].get<double>();
        if (min_order_amount < 0) {
            error_msg = "最低订单金额不能小于0";
            return false;
        }
    }

    // 验证总库存
    auto total_stock = req_json["total_stock"].get<int>();
    if (total_stock <= 0) {
        error_msg = "总库存必须大于0";
        return false;
    }

    // 验证单用户领取上限（如果提供）
    if (req_json.contains("per_user_limit")) {
        auto per_user_limit = req_json["per_user_limit"].get<int>();
        if (per_user_limit < 0) {
            error_msg = "单用户领取上限不能小于0";
            return false;
        }
    }

    // 验证有效期
    auto valid_from_str = req_json["valid_from"].get<std::string>();
    auto valid_to_str = req_json["valid_to"].get<std::string>();

    try {
        auto valid_from = std::chrono::system_clock::from_time_t(std::stoll(valid_from_str));
        auto valid_to = std::chrono::system_clock::from_time_t(std::stoll(valid_to_str));
        auto now = std::chrono::system_clock::now();

        if (valid_from >= valid_to) {
            error_msg = "有效期开始时间必须早于结束时间";
            return false;
        }

        if (valid_to <= now) {
            error_msg = "有效期结束时间必须晚于当前时间";
            return false;
        }
    } catch (const std::exception& e) {
        error_msg = "有效期格式错误";
        return false;
    }

    // 所有验证通过
    return true;
}
