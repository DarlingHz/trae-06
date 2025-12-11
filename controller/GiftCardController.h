#ifndef GIFT_CARD_CONTROLLER_H
#define GIFT_CARD_CONTROLLER_H

#include <drogon/HttpController.h>
#include <json/value.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpTypes.h>
#include <nlohmann/json.hpp>

using namespace drogon;
using json = nlohmann::json;

namespace giftcard {

class GiftCardController : public HttpController<GiftCardController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(GiftCardController::issueGiftCards, "/admin/giftcards/issue", Post);
    ADD_METHOD_TO(GiftCardController::getUserGiftCards, "/users/{user_id}/giftcards", Get);
    ADD_METHOD_TO(GiftCardController::lockGiftCard, "/giftcards/{card_id}/lock", Post);
    ADD_METHOD_TO(GiftCardController::consumeGiftCard, "/giftcards/{card_id}/consume", Post);
    ADD_METHOD_TO(GiftCardController::unlockGiftCard, "/giftcards/{card_id}/unlock", Post);
    ADD_METHOD_TO(GiftCardController::freezeGiftCard, "/admin/giftcards/{card_id}/freeze", Post);
    ADD_METHOD_TO(GiftCardController::unfreezeGiftCard, "/admin/giftcards/{card_id}/unfreeze", Post);
    ADD_METHOD_TO(GiftCardController::getGiftCardConsumptions, "/giftcards/{card_id}/consumptions", Get);
    METHOD_LIST_END

    /**
     * 发放礼品卡给用户
     * @param req HTTP请求
     * @param callback 回调函数
     */
    void issueGiftCards(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);

    /**
     * 用户查询自己的礼品卡列表
     * @param req HTTP请求
     * @param callback 回调函数
     * @param user_id 用户ID
     */
    void getUserGiftCards(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t user_id);

    /**
     * 锁定礼品卡用于某个订单
     * @param req HTTP请求
     * @param callback 回调函数
     * @param card_id 礼品卡ID
     */
    void lockGiftCard(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t card_id);

    /**
     * 确认消费礼品卡
     * @param req HTTP请求
     * @param callback 回调函数
     * @param card_id 礼品卡ID
     */
    void consumeGiftCard(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t card_id);

    /**
     * 取消订单时释放锁定金额
     * @param req HTTP请求
     * @param callback 回调函数
     * @param card_id 礼品卡ID
     */
    void unlockGiftCard(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t card_id);

    /**
     * 冻结礼品卡
     * @param req HTTP请求
     * @param callback 回调函数
     * @param card_id 礼品卡ID
     */
    void freezeGiftCard(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t card_id);

    /**
     * 解冻礼品卡
     * @param req HTTP请求
     * @param callback 回调函数
     * @param card_id 礼品卡ID
     */
    void unfreezeGiftCard(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t card_id);

    /**
     * 查询礼品卡的消费记录
     * @param req HTTP请求
     * @param callback 回调函数
     * @param card_id 礼品卡ID
     */
    void getGiftCardConsumptions(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t card_id);

private:
    /**
     * 验证发放礼品卡的请求参数
     * @param req_json 请求参数JSON
     * @param error_msg 错误信息
     * @return 验证通过返回true，否则返回false
     */
    bool validateIssueGiftCardParams(const Json::Value& req_json, std::string& error_msg);

    /**
     * 验证锁定礼品卡的请求参数
     * @param req_json 请求参数JSON
     * @param error_msg 错误信息
     * @return 验证通过返回true，否则返回false
     */
    bool validateLockGiftCardParams(const Json::Value& req_json, std::string& error_msg);

    /**
     * 验证消费礼品卡的请求参数
     * @param req_json 请求参数JSON
     * @param error_msg 错误信息
     * @return 验证通过返回true，否则返回false
     */
    bool validateConsumeGiftCardParams(const Json::Value& req_json, std::string& error_msg);

    /**
     * 验证释放锁定金额的请求参数
     * @param req_json 请求参数JSON
     * @param error_msg 错误信息
     * @return 验证通过返回true，否则返回false
     */
    bool validateUnlockGiftCardParams(const Json::Value& req_json, std::string& error_msg);
};

} // namespace giftcard

#endif // GIFT_CARD_CONTROLLER_H
