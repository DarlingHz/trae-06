#ifndef GIFT_CARD_REPOSITORY_H
#define GIFT_CARD_REPOSITORY_H

#include <model/GiftCard.h>
#include <model/GiftCardLock.h>
#include <model/GiftCardConsumption.h>
#include <memory>
#include <vector>
#include <string>

namespace giftcard {

class GiftCardRepository {
public:
    // 获取单例实例
    static GiftCardRepository& getInstance();

    // 禁止拷贝和移动
    GiftCardRepository(const GiftCardRepository&) = delete;
    GiftCardRepository& operator=(const GiftCardRepository&) = delete;
    GiftCardRepository(GiftCardRepository&&) = delete;
    GiftCardRepository& operator=(GiftCardRepository&&) = delete;

    // 创建礼品卡
    bool createGiftCard(const GiftCard& gift_card);

    // 批量创建礼品卡
    bool batchCreateGiftCards(const std::vector<GiftCard>& gift_cards);

    // 根据ID获取礼品卡
    std::shared_ptr<GiftCard> getGiftCardById(uint64_t card_id);

    // 根据卡号获取礼品卡
    std::shared_ptr<GiftCard> getGiftCardByCardNo(const std::string& card_no);

    // 根据用户ID获取礼品卡列表
    std::vector<std::shared_ptr<GiftCard>> getGiftCardsByUserId(uint64_t user_id, const std::string& status = "");

    // 更新礼品卡
    bool updateGiftCard(const GiftCard& gift_card);

    // 扣减礼品卡余额
    bool deductGiftCardBalance(uint64_t card_id, double amount);

    // 冻结礼品卡
    bool freezeGiftCard(uint64_t card_id);

    // 解冻礼品卡
    bool unfreezeGiftCard(uint64_t card_id);

    // 创建礼品卡锁定记录
    bool createGiftCardLock(const GiftCardLock& lock);

    // 根据ID获取礼品卡锁定记录
    std::shared_ptr<GiftCardLock> getGiftCardLockById(uint64_t lock_id);

    // 根据礼品卡ID和订单ID获取锁定记录
    std::shared_ptr<GiftCardLock> getGiftCardLockByCardIdAndOrderId(uint64_t card_id, const std::string& order_id);

    // 获取礼品卡的有效锁定记录
    std::vector<std::shared_ptr<GiftCardLock>> getActiveGiftCardLocks(uint64_t card_id);

    // 更新礼品卡锁定记录
    bool updateGiftCardLock(const GiftCardLock& lock);

    // 释放礼品卡锁定记录
    bool releaseGiftCardLock(uint64_t lock_id);

    // 创建礼品卡消费记录
    bool createGiftCardConsumption(const GiftCardConsumption& consumption);

    // 根据ID获取礼品卡消费记录
    std::shared_ptr<GiftCardConsumption> getGiftCardConsumptionById(uint64_t consumption_id);

    // 根据礼品卡ID获取消费记录
    std::vector<std::shared_ptr<GiftCardConsumption>> getGiftCardConsumptionsByCardId(uint64_t card_id);

    // 根据用户ID获取消费记录
    std::vector<std::shared_ptr<GiftCardConsumption>> getGiftCardConsumptionsByUserId(uint64_t user_id);

private:
    // 私有构造函数
    GiftCardRepository() = default;

    // 转换结果集为GiftCard对象
    std::shared_ptr<GiftCard> convertToGiftCard(MYSQL_RES* result);

    // 转换结果集为GiftCard对象列表
    std::vector<std::shared_ptr<GiftCard>> convertToGiftCardList(MYSQL_RES* result);

    // 转换结果集为GiftCardLock对象
    std::shared_ptr<GiftCardLock> convertToGiftCardLock(MYSQL_RES* result);

    // 转换结果集为GiftCardLock对象列表
    std::vector<std::shared_ptr<GiftCardLock>> convertToGiftCardLockList(MYSQL_RES* result);

    // 转换结果集为GiftCardConsumption对象
    std::shared_ptr<GiftCardConsumption> convertToGiftCardConsumption(MYSQL_RES* result);

    // 转换结果集为GiftCardConsumption对象列表
    std::vector<std::shared_ptr<GiftCardConsumption>> convertToGiftCardConsumptionList(MYSQL_RES* result);
};

} // namespace giftcard

#endif // GIFT_CARD_REPOSITORY_H
