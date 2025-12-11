#ifndef GIFT_CARD_TEMPLATE_CONTROLLER_H
#define GIFT_CARD_TEMPLATE_CONTROLLER_H

#include <drogon/HttpController.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpTypes.h>
#include <nlohmann/json.hpp>

using namespace drogon;
using json = nlohmann::json;

namespace giftcard {

class GiftCardTemplateController : public HttpController<GiftCardTemplateController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(GiftCardTemplateController::createTemplate, "/admin/templates", Post);
    ADD_METHOD_TO(GiftCardTemplateController::getTemplates, "/admin/templates", Get);
    ADD_METHOD_TO(GiftCardTemplateController::getTemplateById, "/admin/templates/{template_id}", Get);
    ADD_METHOD_TO(GiftCardTemplateController::closeTemplate, "/admin/templates/{template_id}/close", Post);
    METHOD_LIST_END

    /**
     * 创建礼品卡模板
     * @param req HTTP请求
     * @param callback 回调函数
     */
    void createTemplate(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);

    /**
     * 查询礼品卡模板列表
     * @param req HTTP请求
     * @param callback 回调函数
     */
    void getTemplates(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);

    /**
     * 根据ID查询礼品卡模板详情
     * @param req HTTP请求
     * @param callback 回调函数
     * @param template_id 模板ID
     */
    void getTemplateById(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t template_id);

    /**
     * 关闭礼品卡模板
     * @param req HTTP请求
     * @param callback 回调函数
     * @param template_id 模板ID
     */
    void closeTemplate(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, uint64_t template_id);

private:
    /**
     * 验证创建模板的请求参数
     * @param req_json 请求参数JSON
     * @param error_msg 错误信息
     * @return 验证通过返回true，否则返回false
     */
    bool validateCreateTemplateParams(const json& req_json, std::string& error_msg);
};

} // namespace giftcard

#endif // GIFT_CARD_TEMPLATE_CONTROLLER_H
