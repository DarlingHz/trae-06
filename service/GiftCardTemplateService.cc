#include "GiftCardTemplateService.h"
#include <repository/GiftCardTemplateRepository.h>
#include <utils/Logger.h>
#include <chrono>
#include <string>
#include <vector>

using namespace giftcard;

GiftCardTemplateService& GiftCardTemplateService::getInstance() {
    static GiftCardTemplateService instance;
    return instance;
}

uint64_t GiftCardTemplateService::createTemplate(const GiftCardTemplate& template_info) {
    try {
        // 验证模板信息
        if (template_info.getName().empty()) {
            LOG_ERROR("Template name cannot be empty");
            return 0;
        }

        if (template_info.getType() != "amount" && template_info.getType() != "discount") {
            LOG_ERROR("Invalid template type: %s", template_info.getType().c_str());
            return 0;
        }

        if (template_info.getFaceValue() <= 0) {
            LOG_ERROR("Invalid template face value: %.2f", template_info.getFaceValue());
            return 0;
        }

        if (template_info.getType() == "discount" && (template_info.getFaceValue() < 1 || template_info.getFaceValue() > 100)) {
            LOG_ERROR("Invalid discount rate: %.2f. Must be between 1 and 100", template_info.getFaceValue());
            return 0;
        }

        if (template_info.getTotalStock() <= 0) {
            LOG_ERROR("Invalid template total stock: %d", template_info.getTotalStock());
            return 0;
        }

        if (template_info.getPerUserLimit() <= 0) {
            LOG_ERROR("Invalid template per user limit: %d", template_info.getPerUserLimit());
            return 0;
        }

        auto now = std::chrono::system_clock::now();
        if (template_info.getValidFrom() <= now) {
            LOG_ERROR("Template valid from date must be in the future");
            return 0;
        }

        if (template_info.getValidTo() <= template_info.getValidFrom()) {
            LOG_ERROR("Template valid to date must be after valid from date");
            return 0;
        }

        // 创建模板
        auto template_id = GiftCardTemplateRepository::getInstance().createTemplate(template_info);
        if (template_id == 0) {
            LOG_ERROR("Failed to create gift card template");
            return 0;
        }

        LOG_INFO("Successfully created gift card template with id: %llu", template_id);
        return template_id;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create gift card template: %s", e.what());
        return 0;
    }
}

std::shared_ptr<GiftCardTemplate> GiftCardTemplateService::getTemplateById(uint64_t template_id) {
    try {
        if (template_id == 0) {
            LOG_ERROR("Invalid template id: %llu", template_id);
            return nullptr;
        }

        auto template_info = GiftCardTemplateRepository::getInstance().getTemplateById(template_id);
        if (!template_info) {
            LOG_WARN("Gift card template not found with id: %llu", template_id);
            return nullptr;
        }

        LOG_INFO("Successfully retrieved gift card template with id: %llu", template_id);
        return template_info;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get gift card template by id: %s", e.what());
        return nullptr;
    }
}

std::vector<std::shared_ptr<GiftCardTemplate>> GiftCardTemplateService::getTemplates(
    const std::string& name,
    const std::string& status,
    int page,
    int page_size) {
    try {
        if (page <= 0) {
            LOG_WARN("Invalid page number: %d, using default 1", page);
            page = 1;
        }

        if (page_size <= 0 || page_size > 100) {
            LOG_WARN("Invalid page size: %d, using default 10", page_size);
            page_size = 10;
        }

        auto templates = GiftCardTemplateRepository::getInstance().getTemplates(name, status, page, page_size);
        LOG_INFO("Successfully retrieved %d gift card templates", templates.size());
        return templates;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get gift card templates: %s", e.what());
        return {};
    }
}

bool GiftCardTemplateService::updateTemplate(const GiftCardTemplate& template_info) {
    try {
        if (template_info.getId() == 0) {
            LOG_ERROR("Invalid template id: %llu", template_info.getId());
            return false;
        }

        // 验证模板信息
        if (template_info.getName().empty()) {
            LOG_ERROR("Template name cannot be empty");
            return false;
        }

        if (template_info.getType() != "amount" && template_info.getType() != "discount") {
            LOG_ERROR("Invalid template type: %s", template_info.getType().c_str());
            return false;
        }

        if (template_info.getFaceValue() <= 0) {
            LOG_ERROR("Invalid template face value: %.2f", template_info.getFaceValue());
            return false;
        }

        if (template_info.getType() == "discount" && (template_info.getFaceValue() < 1 || template_info.getFaceValue() > 100)) {
            LOG_ERROR("Invalid discount rate: %.2f. Must be between 1 and 100", template_info.getFaceValue());
            return false;
        }

        if (template_info.getTotalStock() <= 0) {
            LOG_ERROR("Invalid template total stock: %d", template_info.getTotalStock());
            return false;
        }

        if (template_info.getPerUserLimit() <= 0) {
            LOG_ERROR("Invalid template per user limit: %d", template_info.getPerUserLimit());
            return false;
        }

        auto now = std::chrono::system_clock::now();
        if (template_info.getValidFrom() <= now) {
            LOG_ERROR("Template valid from date must be in the future");
            return false;
        }

        if (template_info.getValidTo() <= template_info.getValidFrom()) {
            LOG_ERROR("Template valid to date must be after valid from date");
            return false;
        }

        // 更新模板
        if (!GiftCardTemplateRepository::getInstance().updateTemplate(template_info)) {
            LOG_ERROR("Failed to update gift card template");
            return false;
        }

        LOG_INFO("Successfully updated gift card template with id: %llu", template_info.getId());
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update gift card template: %s", e.what());
        return false;
    }
}

bool GiftCardTemplateService::closeTemplate(uint64_t template_id) {
    try {
        if (template_id == 0) {
            LOG_ERROR("Invalid template id: %llu", template_id);
            return false;
        }

        // 检查模板是否存在
        auto template_info = GiftCardTemplateRepository::getInstance().getTemplateById(template_id);
        if (!template_info) {
            LOG_WARN("Gift card template not found with id: %llu", template_id);
            return false;
        }

        // 检查模板是否已经关闭
        if (template_info->getStatus() == "closed") {
            LOG_WARN("Gift card template already closed with id: %llu", template_id);
            return true; // 已经关闭，返回成功
        }

        // 关闭模板
        if (!GiftCardTemplateRepository::getInstance().closeTemplate(template_id)) {
            LOG_ERROR("Failed to close gift card template");
            return false;
        }

        LOG_INFO("Successfully closed gift card template with id: %llu", template_id);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to close gift card template: %s", e.what());
        return false;
    }
}

bool GiftCardTemplateService::checkTemplateIssuable(uint64_t template_id) {
    try {
        if (template_id == 0) {
            LOG_ERROR("Invalid template id: %llu", template_id);
            return false;
        }

        // 检查模板是否存在
        auto template_info = GiftCardTemplateRepository::getInstance().getTemplateById(template_id);
        if (!template_info) {
            LOG_WARN("Gift card template not found with id: %llu", template_id);
            return false;
        }

        // 检查模板状态
        if (template_info->getStatus() != "active") {
            LOG_WARN("Gift card template not active with id: %llu, status: %s", 
                     template_id, template_info->getStatus().c_str());
            return false;
        }

        // 检查模板有效期
        auto now = std::chrono::system_clock::now();
        if (now < template_info->getValidFrom()) {
            LOG_WARN("Gift card template not yet valid with id: %llu, valid from: %s", 
                     template_id, template_info->getValidFromStr().c_str());
            return false;
        }

        if (now > template_info->getValidTo()) {
            LOG_WARN("Gift card template already expired with id: %llu, valid to: %s", 
                     template_id, template_info->getValidToStr().c_str());
            return false;
        }

        // 检查模板库存
        if (template_info->getIssuedCount() >= template_info->getTotalStock()) {
            LOG_WARN("Gift card template out of stock with id: %llu, issued: %d, total: %d", 
                     template_id, template_info->getIssuedCount(), template_info->getTotalStock());
            return false;
        }

        LOG_INFO("Gift card template is issuable with id: %llu", template_id);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to check template issuable: %s", e.what());
        return false;
    }
}

} // namespace giftcard
