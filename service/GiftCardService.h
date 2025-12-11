#ifndef GIFT_CARD_SERVICE_H
#define GIFT_CARD_SERVICE_H

#include <model/GiftCard.h>
#include <model/GiftCardLock.h>
#include <model/GiftCardConsumption.h>
#include <vector>
#include <memory>
#include <string>

namespace giftcard {

class GiftCardService {
public:
    static GiftCardService& getInstance();

    /**
     * 发放礼品卡给用户
     * @param user_id 用户ID
     * @param template_id 模板ID
     * @param quantity 发放数量
     * @param request_id 幂等键
     * @return 成功返回true，失败返回false
     */
    bool issueGiftCards(uint64_t user_id, uint64_t template_id, int quantity, const std::string& request_id);

    /**
     * 用户查询自己的礼品卡列表
     * @param user_id 用户ID
     * @param status 礼品卡状态（可选）
     * @return 礼品卡列表
     */
    std::vector<std::shared_ptr<GiftCard>> getGiftCardsByUserId(uint64_t user_id, const std::string& status = "");

    /**
     * 锁定礼品卡用于某个订单
     * @param card_id 礼品卡ID
     * @param user_id 用户ID
     * @param order_id 订单ID
     * @param lock_amount 锁定金额
     * @param lock_ttl_seconds 锁定TTL（秒）
     * @return 成功返回true，失败返回false
     */
    bool lockGiftCard(uint64_t card_id, uint64_t user_id, const std::string& order_id, double lock_amount, uint32_t lock_ttl_seconds);

    /**
     * 确认消费礼品卡
     * @param card_id 礼品卡ID
     * @param user_id 用户ID
     * @param order_id 订单ID
     * @param consume_amount 消费金额
     * @param idempotency_key 幂等键
     * @return 成功返回true，失败返回false
     */
    bool consumeGiftCard(uint64_t card_id, uint64_t user_id, const std::string& order_id, double consume_amount, const std::string& idempotency_key);

    /**
     * 取消订单时释放锁定金额
     * @param card_id 礼品卡ID
     * @param user_id 用户ID
     * @param order_id 订单ID
     * @return 成功返回true，失败返回false
     */
    bool unlockGiftCard(uint64_t card_id, uint64_t user_id, const std::string& order_id);

    /**
     * 冻结礼品卡
     * @param card_id 礼品卡ID
     * @return 成功返回true，失败返回false
     */
    bool freezeGiftCard(uint64_t card_id);

    /**
     * 解冻礼品卡
     * @param card_id 礼品卡ID
     * @return 成功返回true，失败返回false
     */
    bool unfreezeGiftCard(uint64_t card_id);

    /**
     * 查询礼品卡的消费记录
     * @param card_id 礼品卡ID
     * @return 消费记录列表
     */
    std::vector<std::shared_ptr<GiftCardConsumption>> getGiftCardConsumptions(uint64_t card_id);

private:
    GiftCardService() = default;
    ~GiftCardService() = default;
    GiftCardService(const GiftCardService&) = delete;
    GiftCardService& operator=(const GiftCardService&) = delete;

    /**
     * 生成礼品卡号
     * @return 礼品卡号
     */
    std::string generateCardNo();

    /**
     * 检查用户是否已经达到该模板的领取上限
     * @param user_id 用户ID
     * @param template_id 模板ID
     * @return 达到上限返回true，否则返回false
     */
    bool checkUserLimit(uint64_t user_id, uint64_t template_id);
};

} // namespace giftcard

#endif // GIFT_CARD_SERVICE_H
