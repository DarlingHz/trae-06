#include "utils/JsonUtils.hpp"
#include <iostream>
#include <stdexcept>
#include <regex>

namespace utils {

json JsonUtils::parse(const std::string& json_str) {
    try {
        return json::parse(json_str);
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        throw std::invalid_argument("Invalid JSON format");
    }
}

std::string JsonUtils::stringify(const json& json_obj, bool pretty) {
    try {
        if (pretty) {
            return json_obj.dump(4);
        } else {
            return json_obj.dump();
        }
    } catch (const json::type_error& e) {
        std::cerr << "JSON stringify error: " << e.what() << std::endl;
        throw std::runtime_error("Failed to stringify JSON");
    }
}

bool JsonUtils::validateRequiredFields(const json& json_obj, const std::vector<std::string>& required_fields) {
    for (const auto& field : required_fields) {
        if (!json_obj.contains(field) || json_obj[field].is_null()) {
            std::cerr << "Missing required field: " << field << std::endl;
            return false;
        }
    }
    return true;
}

bool JsonUtils::validateOptionalFields(const json& json_obj, const std::map<std::string, std::string>& optional_fields) {
    for (const auto& [field, type] : optional_fields) {
        if (json_obj.contains(field) && !json_obj[field].is_null()) {
            if (type == "string" && !json_obj[field].is_string()) {
                std::cerr << "Field " << field << " must be a string" << std::endl;
                return false;
            } else if (type == "number" && !json_obj[field].is_number()) {
                std::cerr << "Field " << field << " must be a number" << std::endl;
                return false;
            } else if (type == "boolean" && !json_obj[field].is_boolean()) {
                std::cerr << "Field " << field << " must be a boolean" << std::endl;
                return false;
            } else if (type == "array" && !json_obj[field].is_array()) {
                std::cerr << "Field " << field << " must be an array" << std::endl;
                return false;
            } else if (type == "object" && !json_obj[field].is_object()) {
                std::cerr << "Field " << field << " must be an object" << std::endl;
                return false;
            }
        }
    }
    return true;
}

std::string JsonUtils::escape(const std::string& str) {
    std::string escaped_str;
    escaped_str.reserve(str.size() * 2); // Reserve space to avoid reallocations

    for (char c : str) {
        switch (c) {
            case '"': escaped_str += "\\\""; break;
            case '\\': escaped_str += "\\\\"; break;
            case '/': escaped_str += "\\/"; break;
            case '\b': escaped_str += "\\b"; break;
            case '\f': escaped_str += "\\f"; break;
            case '\n': escaped_str += "\\n"; break;
            case '\r': escaped_str += "\\r"; break;
            case '\t': escaped_str += "\\t"; break;
            default: escaped_str += c; break;
        }
    }

    return escaped_str;
}

std::string JsonUtils::unescape(const std::string& str) {
    std::string unescaped_str;
    unescaped_str.reserve(str.size()); // Reserve space to avoid reallocations

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '\\' && i + 1 < str.size()) {
            ++i; // Skip the backslash
            switch (str[i]) {
                case '"': unescaped_str += '"'; break;
                case '\\': unescaped_str += '\\'; break;
                case '/': unescaped_str += '/'; break;
                case 'b': unescaped_str += '\b'; break;
                case 'f': unescaped_str += '\f'; break;
                case 'n': unescaped_str += '\n'; break;
                case 'r': unescaped_str += '\r'; break;
                case 't': unescaped_str += '\t'; break;
                default: unescaped_str += '\\'; unescaped_str += str[i]; break; // Invalid escape sequence, keep as-is
            }
        } else {
            unescaped_str += str[i];
        }
    }

    return unescaped_str;
}

json JsonUtils::createErrorResponse(const std::string& error_code, const std::string& error_message) {
    json error_response;
    error_response["error_code"] = error_code;
    error_response["error_message"] = error_message;
    return error_response;
}

} // namespace utils
