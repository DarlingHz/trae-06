#ifndef GIFT_CARD_TEMPLATE_REPOSITORY_H
#define GIFT_CARD_TEMPLATE_REPOSITORY_H

#include <memory>
#include <vector>
#include <string>
#include <model/GiftCardTemplate.h>

namespace giftcard {

class GiftCardTemplateRepository {
public:
    // 单例模式
    static GiftCardTemplateRepository& getInstance() {
        static GiftCardTemplateRepository instance;
        return instance;
    }

    // 创建礼品卡模板
    bool createTemplate(const GiftCardTemplate& template_);

    // 根据ID获取礼品卡模板
    std::shared_ptr<GiftCardTemplate> getTemplateById(uint64_t template_id);

    // 分页查询礼品卡模板
    std::vector<std::shared_ptr<GiftCardTemplate>> getTemplates(
        const std::string& name = "",
        const std::string& status = "",
        int page = 1,
        int page_size = 10
    );

    // 更新礼品卡模板
    bool updateTemplate(const GiftCardTemplate& template_);

    // 关闭礼品卡模板
    bool closeTemplate(uint64_t template_id);

    // 更新模板已发放数量
    bool updateTemplateIssuedCount(uint64_t template_id, int increment);

private:
    GiftCardTemplateRepository() = default;
    ~GiftCardTemplateRepository() = default;
    GiftCardTemplateRepository(const GiftCardTemplateRepository&) = delete;
    GiftCardTemplateRepository& operator=(const GiftCardTemplateRepository&) = delete;

    // 将MySQL结果集转换为GiftCardTemplate对象
    std::shared_ptr<GiftCardTemplate> convertToTemplate(MYSQL_RES* result);

    // 将MySQL结果集转换为GiftCardTemplate对象列表
    std::vector<std::shared_ptr<GiftCardTemplate>> convertToTemplateList(MYSQL_RES* result);
};

} // namespace giftcard

#endif // GIFT_CARD_TEMPLATE_REPOSITORY_H
