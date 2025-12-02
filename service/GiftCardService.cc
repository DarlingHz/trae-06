#include "GiftCardService.h"
#include "GiftCardTemplateService.h"
#include <repository/GiftCardRepository.h>
#include <utils/Logger.h>
#include <utils/Config.h>
#include <utils/RedisPool.h>
#include <random>
#include <chrono>
#include <ctime>
#include <iomanip>

namespace giftcard {

GiftCardService& GiftCardService::getInstance() {
    static GiftCardService instance;
    return instance;
}

bool GiftCardService::issueGiftCards(uint64_t user_id, uint64_t template_id, int quantity, const std::string& request_id) {
    LOG_INFO("开始发放礼品卡: user_id={}, template_id={}, quantity={}, request_id={}", 
             user_id, template_id, quantity, request_id);

    // 检查幂等键是否已存在
    auto redis = RedisPool::getInstance().getConnection();
    if (!redis) {
        LOG_ERROR("获取Redis连接失败");
        return false;
    }

    std::string redis_key = "giftcard:issue:" + request_id;
    auto result = redis->exists(redis_key);
    if (result && *result > 0) {
        LOG_INFO("幂等键已存在，跳过发放: request_id={}", request_id);
        return true;
    }

    // 检查模板是否存在且可发放
    auto template_service = GiftCardTemplateService::getInstance();
    auto template_info = template_service.getTemplateById(template_id);
    if (!template_info) {
        LOG_ERROR("模板不存在: template_id={}", template_id);
        return false;
    }

    if (!template_service.checkTemplateIssuable(template_id)) {
        LOG_ERROR("模板不可发放: template_id={}", template_id);
        return false;
    }

    // 检查模板库存是否足够
    if (template_info->getIssuedCount() + quantity > template_info->getTotalStock()) {
        LOG_ERROR("模板库存不足: template_id={}, issued={}, total={}, request={}", 
                  template_id, template_info->getIssuedCount(), template_info->getTotalStock(), quantity);
        return false;
    }

    // 检查用户领取上限
    if (template_info->getPerUserLimit() > 0) {
        if (checkUserLimit(user_id, template_id)) {
            LOG_ERROR("用户领取数量已达上限: user_id={}, template_id={}", user_id, template_id);
            return false;
        }
    }

    // 批量创建礼品卡
    std::vector<std::shared_ptr<GiftCard>> gift_cards;
    for (int i = 0; i < quantity; ++i) {
        auto gift_card = std::make_shared<GiftCard>();
        gift_card->setCardNo(generateCardNo());
        gift_card->setTemplateId(template_id);
        gift_card->setUserId(user_id);
        gift_card->setBalance(template_info->getFaceValue());
        gift_card->setStatus("available");
        gift_card->setValidFrom(template_info->getValidFrom());
        gift_card->setValidTo(template_info->getValidTo());
        gift_cards.push_back(gift_card);
    }

    auto repository = GiftCardRepository::getInstance();
    if (!repository->batchCreateGiftCards(gift_cards)) {
        LOG_ERROR("批量创建礼品卡失败");
        return false;
    }

    // 更新模板已发放数量
    if (!template_service.updateTemplateIssuedCount(template_id, quantity)) {
        LOG_ERROR("更新模板已发放数量失败");
        // 这里可能需要回滚礼品卡创建，但为了简化，暂时不处理
        return false;
    }

    // 存储幂等键到Redis，设置过期时间（例如24小时）
    redis->setex(redis_key, 24 * 60 * 60, "1");

    LOG_INFO("礼品卡发放成功: user_id={}, template_id={}, quantity={}", user_id, template_id, quantity);
    return true;
}

std::vector<std::shared_ptr<GiftCard>> GiftCardService::getGiftCardsByUserId(uint64_t user_id, const std::string& status) {
    LOG_INFO("查询用户礼品卡列表: user_id={}, status={}", user_id, status);

    auto repository = GiftCardRepository::getInstance();
    auto gift_cards = repository->getGiftCardsByUserId(user_id, status);

    LOG_INFO("查询到用户礼品卡数量: user_id={}, count={}", user_id, gift_cards.size());
    return gift_cards;
}

bool GiftCardService::lockGiftCard(uint64_t card_id, uint64_t user_id, const std::string& order_id, double lock_amount, uint32_t lock_ttl_seconds) {
    LOG_INFO("锁定礼品卡: card_id={}, user_id={}, order_id={}, lock_amount={}, lock_ttl_seconds={}", 
             card_id, user_id, order_id, lock_amount, lock_ttl_seconds);

    // 检查礼品卡是否存在且属于该用户
    auto repository = GiftCardRepository::getInstance();
    auto gift_card = repository->getGiftCardById(card_id);
    if (!gift_card) {
        LOG_ERROR("礼品卡不存在: card_id={}", card_id);
        return false;
    }

    if (gift_card->getUserId() != user_id) {
        LOG_ERROR("礼品卡不属于该用户: card_id={}, user_id={}, owner_id={}", 
                  card_id, user_id, gift_card->getUserId());
        return false;
    }

    // 检查礼品卡状态是否可用
    if (gift_card->getStatus() != "available") {
        LOG_ERROR("礼品卡状态不可用: card_id={}, status={}", card_id, gift_card->getStatus());
        return false;
    }

    // 检查礼品卡是否已过期
    auto now = std::chrono::system_clock::now();
    auto valid_to = gift_card->getValidTo();
    if (now > valid_to) {
        LOG_ERROR("礼品卡已过期: card_id={}, valid_to={}", card_id, 
                  std::chrono::system_clock::to_time_t(valid_to));
        return false;
    }

    // 检查余额是否足够
    if (gift_card->getBalance() < lock_amount) {
        LOG_ERROR("礼品卡余额不足: card_id={}, balance={}, lock_amount={}", 
                  card_id, gift_card->getBalance(), lock_amount);
        return false;
    }

    // 使用Redis分布式锁确保并发安全
    auto redis = RedisPool::getInstance().getConnection();
    if (!redis) {
        LOG_ERROR("获取Redis连接失败");
        return false;
    }

    std::string lock_key = "giftcard:lock:" + std::to_string(card_id);
    std::string lock_value = order_id;
    int lock_timeout = 30; // 锁超时时间（秒）

    // 尝试获取锁
    auto lock_result = redis->setnx(lock_key, lock_value);
    if (!lock_result || *lock_result == 0) {
        LOG_ERROR("获取礼品卡锁失败: card_id={}", card_id);
        return false;
    }

    // 设置锁超时时间
    redis->expire(lock_key, lock_timeout);

    // 在数据库事务中创建锁定记录并更新礼品卡余额
    bool success = false;
    try {
        // 开始事务
        repository->beginTransaction();

        // 创建锁定记录
        auto gift_card_lock = std::make_shared<GiftCardLock>();
        gift_card_lock->setCardId(card_id);
        gift_card_lock->setUserId(user_id);
        gift_card_lock->setOrderId(order_id);
        gift_card_lock->setLockAmount(lock_amount);
        gift_card_lock->setStatus("active");

        // 计算锁定过期时间
        auto lock_ttl = std::chrono::seconds(lock_ttl_seconds);
        auto lock_expire_time = std::chrono::system_clock::now() + lock_ttl;
        gift_card_lock->setLockTtl(lock_expire_time);

        if (!repository->createGiftCardLock(gift_card_lock)) {
            throw std::runtime_error("创建锁定记录失败");
        }

        // 更新礼品卡余额（可用余额 = 总余额 - 锁定金额）
        // 注意：这里需要考虑礼品卡可能有多个锁定记录的情况
        // 简化处理：直接将余额减少锁定金额
        double new_balance = gift_card->getBalance() - lock_amount;
        gift_card->setBalance(new_balance);

        if (!repository->updateGiftCard(gift_card)) {
            throw std::runtime_error("更新礼品卡余额失败");
        }

        // 提交事务
        repository->commitTransaction();
        success = true;

        LOG_INFO("礼品卡锁定成功: card_id={}, order_id={}, lock_amount={}", card_id, order_id, lock_amount);
    } catch (const std::exception& e) {
        LOG_ERROR("锁定礼品卡失败: card_id={}, error={}", card_id, e.what());
        // 回滚事务
        repository->rollbackTransaction();
        success = false;
    }

    // 释放Redis锁
    redis->del(lock_key);

    return success;
}

bool GiftCardService::consumeGiftCard(uint64_t card_id, uint64_t user_id, const std::string& order_id, double consume_amount, const std::string& idempotency_key) {
    LOG_INFO("消费礼品卡: card_id={}, user_id={}, order_id={}, consume_amount={}, idempotency_key={}", 
             card_id, user_id, order_id, consume_amount, idempotency_key);

    // 检查幂等键是否已存在
    auto redis = RedisPool::getInstance().getConnection();
    if (!redis) {
        LOG_ERROR("获取Redis连接失败");
        return false;
    }

    std::string redis_key = "giftcard:consume:" + idempotency_key;
    auto result = redis->exists(redis_key);
    if (result && *result > 0) {
        LOG_INFO("幂等键已存在，跳过消费: idempotency_key={}", idempotency_key);
        return true;
    }

    // 检查礼品卡是否存在且属于该用户
    auto repository = GiftCardRepository::getInstance();
    auto gift_card = repository->getGiftCardById(card_id);
    if (!gift_card) {
        LOG_ERROR("礼品卡不存在: card_id={}", card_id);
        return false;
    }

    if (gift_card->getUserId() != user_id) {
        LOG_ERROR("礼品卡不属于该用户: card_id={}, user_id={}, owner_id={}", 
                  card_id, user_id, gift_card->getUserId());
        return false;
    }

    // 检查锁定记录是否存在且有效
    auto gift_card_lock = repository->getActiveGiftCardLockByOrderId(card_id, order_id);
    if (!gift_card_lock) {
        LOG_ERROR("锁定记录不存在或已失效: card_id={}, order_id={}", card_id, order_id);
        return false;
    }

    // 检查锁定金额是否足够
    if (gift_card_lock->getLockAmount() < consume_amount) {
        LOG_ERROR("锁定金额不足: card_id={}, lock_amount={}, consume_amount={}", 
                  card_id, gift_card_lock->getLockAmount(), consume_amount);
        return false;
    }

    // 使用Redis分布式锁确保并发安全
    std::string lock_key = "giftcard:lock:" + std::to_string(card_id);
    std::string lock_value = order_id;
    int lock_timeout = 30; // 锁超时时间（秒）

    // 尝试获取锁
    auto lock_result = redis->setnx(lock_key, lock_value);
    if (!lock_result || *lock_result == 0) {
        LOG_ERROR("获取礼品卡锁失败: card_id={}", card_id);
        return false;
    }

    // 设置锁超时时间
    redis->expire(lock_key, lock_timeout);

    bool success = false;
    try {
        // 开始事务
        repository->beginTransaction();

        // 创建消费记录
        auto consumption = std::make_shared<GiftCardConsumption>();
        consumption->setCardId(card_id);
        consumption->setUserId(user_id);
        consumption->setOrderId(order_id);
        consumption->setConsumeAmount(consume_amount);
        consumption->setConsumeTime(std::chrono::system_clock::now());

        if (!repository->createGiftCardConsumption(consumption)) {
            throw std::runtime_error("创建消费记录失败");
        }

        // 更新锁定记录
        double remaining_lock_amount = gift_card_lock->getLockAmount() - consume_amount;
        if (remaining_lock_amount <= 0) {
            // 锁定金额全部消费完，关闭锁定记录
            gift_card_lock->setStatus("closed");
        } else {
            // 还有剩余锁定金额，更新锁定金额
            gift_card_lock->setLockAmount(remaining_lock_amount);
        }

        if (!repository->updateGiftCardLock(gift_card_lock)) {
            throw std::runtime_error("更新锁定记录失败");
        }

        // 更新礼品卡余额和状态
        // 注意：礼品卡余额在锁定时已经减少，这里不需要再次减少
        // 只需要检查余额是否为0，如果为0则设置状态为used
        if (gift_card->getBalance() <= 0) {
            gift_card->setStatus("used");
            if (!repository->updateGiftCard(gift_card)) {
                throw std::runtime_error("更新礼品卡状态失败");
            }
        }

        // 提交事务
        repository->commitTransaction();
        success = true;

        // 存储幂等键到Redis，设置过期时间（例如24小时）
        redis->setex(redis_key, 24 * 60 * 60, "1");

        LOG_INFO("礼品卡消费成功: card_id={}, order_id={}, consume_amount={}", card_id, order_id, consume_amount);
    } catch (const std::exception& e) {
        LOG_ERROR("消费礼品卡失败: card_id={}, error={}", card_id, e.what());
        // 回滚事务
        repository->rollbackTransaction();
        success = false;
    }

    // 释放Redis锁
    redis->del(lock_key);

    return success;
}

bool GiftCardService::unlockGiftCard(uint64_t card_id, uint64_t user_id, const std::string& order_id) {
    LOG_INFO("释放礼品卡锁定: card_id={}, user_id={}, order_id={}", card_id, user_id, order_id);

    // 检查礼品卡是否存在且属于该用户
    auto repository = GiftCardRepository::getInstance();
    auto gift_card = repository->getGiftCardById(card_id);
    if (!gift_card) {
        LOG_ERROR("礼品卡不存在: card_id={}", card_id);
        return false;
    }

    if (gift_card->getUserId() != user_id) {
        LOG_ERROR("礼品卡不属于该用户: card_id={}, user_id={}, owner_id={}", 
                  card_id, user_id, gift_card->getUserId());
        return false;
    }

    // 检查锁定记录是否存在且有效
    auto gift_card_lock = repository->getActiveGiftCardLockByOrderId(card_id, order_id);
    if (!gift_card_lock) {
        LOG_ERROR("锁定记录不存在或已失效: card_id={}, order_id={}", card_id, order_id);
        return false;
    }

    // 使用Redis分布式锁确保并发安全
    auto redis = RedisPool::getInstance().getConnection();
    if (!redis) {
        LOG_ERROR("获取Redis连接失败");
        return false;
    }

    std::string lock_key = "giftcard:lock:" + std::to_string(card_id);
    std::string lock_value = order_id;
    int lock_timeout = 30; // 锁超时时间（秒）

    // 尝试获取锁
    auto lock_result = redis->setnx(lock_key, lock_value);
    if (!lock_result || *lock_result == 0) {
        LOG_ERROR("获取礼品卡锁失败: card_id={}", card_id);
        return false;
    }

    // 设置锁超时时间
    redis->expire(lock_key, lock_timeout);

    bool success = false;
    try {
        // 开始事务
        repository->beginTransaction();

        // 更新礼品卡余额（将锁定金额恢复到可用余额）
        double new_balance = gift_card->getBalance() + gift_card_lock->getLockAmount();
        gift_card->setBalance(new_balance);

        if (!repository->updateGiftCard(gift_card)) {
            throw std::runtime_error("更新礼品卡余额失败");
        }

        // 关闭锁定记录
        gift_card_lock->setStatus("closed");

        if (!repository->updateGiftCardLock(gift_card_lock)) {
            throw std::runtime_error("更新锁定记录失败");
        }

        // 提交事务
        repository->commitTransaction();
        success = true;

        LOG_INFO("礼品卡锁定释放成功: card_id={}, order_id={}, unlock_amount={}", 
                 card_id, order_id, gift_card_lock->getLockAmount());
    } catch (const std::exception& e) {
        LOG_ERROR("释放礼品卡锁定失败: card_id={}, error={}", card_id, e.what());
        // 回滚事务
        repository->rollbackTransaction();
        success = false;
    }

    // 释放Redis锁
    redis->del(lock_key);

    return success;
}

bool GiftCardService::freezeGiftCard(uint64_t card_id) {
    LOG_INFO("冻结礼品卡: card_id={}", card_id);

    auto repository = GiftCardRepository::getInstance();
    auto gift_card = repository->getGiftCardById(card_id);
    if (!gift_card) {
        LOG_ERROR("礼品卡不存在: card_id={}", card_id);
        return false;
    }

    // 检查礼品卡是否已经被冻结
    if (gift_card->getStatus() == "frozen") {
        LOG_INFO("礼品卡已经被冻结: card_id={}", card_id);
        return true;
    }

    // 更新礼品卡状态为冻结
    gift_card->setStatus("frozen");
    if (!repository->updateGiftCard(gift_card)) {
        LOG_ERROR("冻结礼品卡失败: card_id={}", card_id);
        return false;
    }

    LOG_INFO("礼品卡冻结成功: card_id={}", card_id);
    return true;
}

bool GiftCardService::unfreezeGiftCard(uint64_t card_id) {
    LOG_INFO("解冻礼品卡: card_id={}", card_id);

    auto repository = GiftCardRepository::getInstance();
    auto gift_card = repository->getGiftCardById(card_id);
    if (!gift_card) {
        LOG_ERROR("礼品卡不存在: card_id={}", card_id);
        return false;
    }

    // 检查礼品卡是否已经被解冻
    if (gift_card->getStatus() != "frozen") {
        LOG_INFO("礼品卡未被冻结: card_id={}", card_id);
        return true;
    }

    // 更新礼品卡状态为可用
    gift_card->setStatus("available");
    if (!repository->updateGiftCard(gift_card)) {
        LOG_ERROR("解冻礼品卡失败: card_id={}", card_id);
        return false;
    }

    LOG_INFO("礼品卡解冻成功: card_id={}", card_id);
    return true;
}

std::vector<std::shared_ptr<GiftCardConsumption>> GiftCardService::getGiftCardConsumptions(uint64_t card_id) {
    LOG_INFO("查询礼品卡消费记录: card_id={}", card_id);

    auto repository = GiftCardRepository::getInstance();
    auto consumptions = repository->getGiftCardConsumptionsByCardId(card_id);

    LOG_INFO("查询到礼品卡消费记录数量: card_id={}, count={}", card_id, consumptions.size());
    return consumptions;
}

std::string GiftCardService::generateCardNo() {
    // 生成16位礼品卡号
    // 格式: 前6位为固定前缀，中间8位为随机数，最后2位为校验码

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 9);

    std::string prefix = "100000";
    std::string random_part;

    for (int i = 0; i < 8; ++i) {
        random_part += std::to_string(dis(gen));
    }

    std::string card_no_without_check = prefix + random_part;

    // 计算校验码（Luhn算法）
    int sum = 0;
    bool is_second = false;

    for (int i = card_no_without_check.size() - 1; i >= 0; --i) {
        int digit = card_no_without_check[i] - '0';

        if (is_second) {
            digit *= 2;
            if (digit > 9) {
                digit -= 9;
            }
        }

        sum += digit;
        is_second = !is_second;
    }

    int check_digit = (10 - (sum % 10)) % 10;

    return card_no_without_check + std::to_string(check_digit);
}

bool GiftCardService::checkUserLimit(uint64_t user_id, uint64_t template_id) {
    auto repository = GiftCardRepository::getInstance();
    int count = repository->getGiftCardCountByUserAndTemplate(user_id, template_id);

    auto template_service = GiftCardTemplateService::getInstance();
    auto template_info = template_service.getTemplateById(template_id);

    if (template_info && count >= template_info->getPerUserLimit()) {
        return true;
    }

    return false;
}

} // namespace giftcard
