#ifndef GIFT_CARD_TEMPLATE_SERVICE_H
#define GIFT_CARD_TEMPLATE_SERVICE_H

#include <model/GiftCardTemplate.h>
#include <vector>
#include <memory>
#include <string>

namespace giftcard {

class GiftCardTemplateService {
public:
    static GiftCardTemplateService& getInstance();

    /**
     * 创建礼品卡模板
     * @param template_info 模板信息
     * @return 模板ID，失败返回0
     */
    uint64_t createTemplate(const GiftCardTemplate& template_info);

    /**
     * 根据ID获取模板信息
     * @param template_id 模板ID
     * @return 模板信息，失败返回nullptr
     */
    std::shared_ptr<GiftCardTemplate> getTemplateById(uint64_t template_id);

    /**
     * 获取模板列表
     * @param name 模板名称（可选，用于模糊查询）
     * @param status 模板状态（可选）
     * @param page 页码（从1开始）
     * @param page_size 每页大小
     * @return 模板列表
     */
    std::vector<std::shared_ptr<GiftCardTemplate>> getTemplates(
        const std::string& name = "",
        const std::string& status = "",
        int page = 1,
        int page_size = 10);

    /**
     * 更新模板信息
     * @param template_info 模板信息
     * @return 成功返回true，失败返回false
     */
    bool updateTemplate(const GiftCardTemplate& template_info);

    /**
     * 关闭模板
     * @param template_id 模板ID
     * @return 成功返回true，失败返回false
     */
    bool closeTemplate(uint64_t template_id);

    /**
     * 检查模板是否可发放
     * @param template_id 模板ID
     * @return 可发放返回true，否则返回false
     */
    bool checkTemplateIssuable(uint64_t template_id);

private:
    GiftCardTemplateService() = default;
    ~GiftCardTemplateService() = default;
    GiftCardTemplateService(const GiftCardTemplateService&) = delete;
    GiftCardTemplateService& operator=(const GiftCardTemplateService&) = delete;
};

} // namespace giftcard

#endif // GIFT_CARD_TEMPLATE_SERVICE_H
