#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>

using json = nlohmann::json;

namespace utils {

class JsonUtils {
public:
    // Parse JSON string to json object
    static json parse(const std::string& json_str);

    // Convert json object to string
    static std::string stringify(const json& json_obj, bool pretty = false);

    // Validate JSON object has required fields
    static bool validateRequiredFields(const json& json_obj, const std::vector<std::string>& required_fields);

    // Validate JSON object has optional fields of specific types
    static bool validateOptionalFields(const json& json_obj, const std::map<std::string, std::string>& optional_fields);

    // Escape special characters in JSON string
    static std::string escape(const std::string& str);

    // Unescape special characters in JSON string
    static std::string unescape(const std::string& str);

    // Create error response JSON
    static json createErrorResponse(const std::string& error_code, const std::string& error_message);
};

} // namespace utils
