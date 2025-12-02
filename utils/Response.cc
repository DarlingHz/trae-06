#include "Response.h"
#include <drogon/HttpResponse.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace giftcard {

drogon::HttpResponsePtr Response::success(const json& data) {
    return create(0, "ok", data);
}

drogon::HttpResponsePtr Response::success(const std::string& message, const json& data) {
    return create(0, message, data);
}

drogon::HttpResponsePtr Response::failure(int code, const std::string& message) {
    return create(code, message, json::object());
}

drogon::HttpResponsePtr Response::failure(int code, const std::string& message, const json& data) {
    return create(code, message, data);
}

drogon::HttpResponsePtr Response::create(int code, const std::string& message, const json& data) {
    // 创建响应对象
    auto response = drogon::HttpResponse::newHttpJsonResponse({
        {"code", code},
        {"message", message},
        {"data", data}
    });

    // 设置响应头
    response->addHeader("Content-Type", "application/json; charset=utf-8");

    // 根据错误码设置HTTP状态码
    if (code == 0) {
        response->setStatusCode(drogon::HttpStatusCode::k200OK);
    } else if (code >= 1000 && code < 2000) {
        // 客户端错误
        response->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
    } else if (code >= 2000 && code < 3000) {
        // 服务器错误
        response->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
    } else {
        // 默认使用400
        response->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
    }

    return response;
}

} // namespace giftcard
