#include "company.h"
#include <sstream>

namespace recruitment {

std::string Company::toJson() const {
    std::stringstream ss;
    ss << "{";
    ss << "\"id\": " << id_ << ",";
    ss << "\"name\": \"" << name_ << "\",";
    ss << "\"industry\": \"" << industry_ << "\",";
    ss << "\"location\": \"" << location_ << "\",";
    ss << "\"description\": \"" << description_ << "\",";
    ss << "\"created_at\": \"" << created_at_ << "\",";
    ss << "\"updated_at\": \"" << updated_at_ << "\"";
    ss << "}";
    return ss.str();
}

bool Company::fromJson(const std::string& json) {
    // 检查JSON字符串是否以'{'开头和以'}'结尾
    if (json.empty() || json.front() != '{' || json.back() != '}') {
        return false;
    }

    // 去除JSON字符串中的空格和换行符
    std::string cleaned_json;
    for (char c : json) {
        if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
            cleaned_json += c;
        }
    }

    // 分割JSON字符串中的键值对
    std::vector<std::string> key_value_pairs;
    std::stringstream ss(cleaned_json.substr(1, cleaned_json.size() - 2));
    std::string pair;
    while (getline(ss, pair, ',')) {
        key_value_pairs.push_back(pair);
    }

    // 处理每个键值对
    for (const std::string& key_value : key_value_pairs) {
        // 分割键和值
        size_t colon_pos = key_value.find(':');
        if (colon_pos == std::string::npos) {
            return false;
        }

        std::string key = key_value.substr(0, colon_pos);
        std::string value = key_value.substr(colon_pos + 1);

        // 去除键和值的双引号
        if (key.front() == '"' && key.back() == '"') {
            key = key.substr(1, key.size() - 2);
        } else {
            return false;
        }

        if (value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }

        // 处理每个键
        if (key == "id") {
            try {
                id_ = std::stoll(value);
            } catch (const std::exception& e) {
                return false;
            }
        } else if (key == "name") {
            name_ = value;
        } else if (key == "industry") {
            industry_ = value;
        } else if (key == "location") {
            location_ = value;
        } else if (key == "description") {
            description_ = value;
        }
        // 忽略"created_at"和"updated_at"键
    }

    return true;
}

} // namespace recruitment
