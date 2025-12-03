#include "utils/json.h"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <sstream>

using json = nlohmann::json;

Json::Json() {}

Json::~Json() {}

Json& Json::getInstance() {
    static Json instance;
    return instance;
}

std::string Json::serialize(const std::map<std::string, std::any>& data) {
    try {
        json j;
        for (const auto& pair : data) {
            const std::string& key = pair.first;
            const std::any& value = pair.second;

            if (value.type() == typeid(std::string)) {
                j[key] = std::any_cast<std::string>(value);
            } else if (value.type() == typeid(int)) {
                j[key] = std::any_cast<int>(value);
            } else if (value.type() == typeid(double)) {
                j[key] = std::any_cast<double>(value);
            } else if (value.type() == typeid(bool)) {
                j[key] = std::any_cast<bool>(value);
            } else if (value.type() == typeid(std::vector<int>)) {
                j[key] = std::any_cast<std::vector<int>>(value);
            } else if (value.type() == typeid(std::vector<std::string>)) {
                j[key] = std::any_cast<std::vector<std::string>>(value);
            } else {
                throw std::runtime_error("Unsupported type for JSON serialization: " + std::string(value.type().name()));
            }
        }

        return j.dump(4); // 格式化输出，缩进4个空格
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to serialize JSON: " + std::string(e.what()));
    }
}

std::map<std::string, std::any> Json::deserialize(const std::string& json_str) {
    try {
        json j = json::parse(json_str);
        std::map<std::string, std::any> data;

        for (const auto& pair : j.items()) {
            const std::string& key = pair.key();
            const json& value = pair.value();

            if (value.is_string()) {
                data[key] = value.get<std::string>();
            } else if (value.is_number_integer()) {
                data[key] = value.get<int>();
            } else if (value.is_number_float()) {
                data[key] = value.get<double>();
            } else if (value.is_boolean()) {
                data[key] = value.get<bool>();
            } else if (value.is_array()) {
                if (value.empty()) {
                    data[key] = std::vector<std::string>();
                } else if (value[0].is_string()) {
                    data[key] = value.get<std::vector<std::string>>();
                } else if (value[0].is_number_integer()) {
                    data[key] = value.get<std::vector<int>>();
                } else {
                    throw std::runtime_error("Unsupported array type in JSON deserialization");
                }
            } else {
                throw std::runtime_error("Unsupported type for JSON deserialization: " + std::string(value.type_name()));
            }
        }

        return data;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to deserialize JSON: " + std::string(e.what()));
    }
}

std::string Json::createSuccessResponse(const std::map<std::string, std::any>& data) {
    std::map<std::string, std::any> response;
    response["success"] = true;
    response["data"] = data;
    return serialize(response);
}

std::string Json::createErrorResponse(const std::string& error_code, const std::string& message) {
    std::map<std::string, std::any> response;
    response["success"] = false;
    response["error_code"] = error_code;
    response["message"] = message;
    return serialize(response);
}
