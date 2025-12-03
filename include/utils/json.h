#ifndef JSON_H
#define JSON_H

#include <string>
#include <map>
#include <vector>
#include <any>
#include "json.hpp"

using json = nlohmann::json;

class Json {
private:
    Json() = default;
    ~Json() = default;

public:
    static Json& getInstance();

    // 序列化map到JSON字符串
    std::string serialize(const std::map<std::string, std::any>& data) const;

    // 反序列化JSON字符串到map
    std::map<std::string, std::any> deserialize(const std::string& json_str) const;

    // 创建成功响应
    std::string createSuccessResponse(int code = 200, const std::string& message = "success", const std::map<std::string, std::any>& data = {}) const;

    // 创建错误响应
    std::string createErrorResponse(int code = 500, const std::string& message = "Internal server error") const;

private:
    // 辅助方法：将std::any转换为json
    void anyToJson(const std::any& value, json& j) const;

    // 辅助方法：将json转换为std::any
    std::any jsonToAny(const json& j) const;
};

#endif // JSON_H
