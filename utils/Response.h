#ifndef RESPONSE_H
#define RESPONSE_H

#include <nlohmann/json.hpp>
#include <drogon/HttpResponse.h>

using json = nlohmann::json;

namespace giftcard {

class Response {
public:
    // 成功响应
    static drogon::HttpResponsePtr success(const json& data = json::object());
    static drogon::HttpResponsePtr success(const std::string& message, const json& data = json::object());

    // 失败响应
    static drogon::HttpResponsePtr failure(int code, const std::string& message);
    static drogon::HttpResponsePtr failure(int code, const std::string& message, const json& data);

    // 通用响应生成
    static drogon::HttpResponsePtr create(int code, const std::string& message, const json& data = json::object());

private:
    Response() = default;
    ~Response() = default;
    Response(const Response&) = delete;
    Response& operator=(const Response&) = delete;
};

} // namespace giftcard

#endif // RESPONSE_H
