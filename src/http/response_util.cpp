#include "response_util.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>

namespace http {

using namespace web;
using namespace web::http;
using namespace web::json;

// ErrorCode 转换实现
std::string error_code_to_string(ErrorCode code) {
    switch (code) {
        case ErrorCode::SUCCESS: return "SUCCESS";
        case ErrorCode::BAD_REQUEST: return "BAD_REQUEST";
        case ErrorCode::UNAUTHORIZED: return "UNAUTHORIZED";
        case ErrorCode::FORBIDDEN: return "FORBIDDEN";
        case ErrorCode::NOT_FOUND: return "NOT_FOUND";
        case ErrorCode::METHOD_NOT_ALLOWED: return "METHOD_NOT_ALLOWED";
        case ErrorCode::CONFLICT: return "CONFLICT";
        case ErrorCode::UNPROCESSABLE_ENTITY: return "UNPROCESSABLE_ENTITY";
        case ErrorCode::INTERNAL_SERVER_ERROR: return "INTERNAL_SERVER_ERROR";
        case ErrorCode::SERVICE_UNAVAILABLE: return "SERVICE_UNAVAILABLE";
        case ErrorCode::INVALID_INPUT: return "INVALID_INPUT";
        case ErrorCode::VALIDATION_FAILED: return "VALIDATION_FAILED";
        case ErrorCode::AUTH_FAILED: return "AUTH_FAILED";
        case ErrorCode::TOKEN_EXPIRED: return "TOKEN_EXPIRED";
        case ErrorCode::PERMISSION_DENIED: return "PERMISSION_DENIED";
        case ErrorCode::RESOURCE_NOT_FOUND: return "RESOURCE_NOT_FOUND";
        case ErrorCode::DUPLICATE_RESOURCE: return "DUPLICATE_RESOURCE";
        case ErrorCode::OPERATION_FAILED: return "OPERATION_FAILED";
        case ErrorCode::DATABASE_ERROR: return "DATABASE_ERROR";
        case ErrorCode::NETWORK_ERROR: return "NETWORK_ERROR";
        case ErrorCode::VALIDATION_ERROR: return "VALIDATION_ERROR";
        case ErrorCode::INVALID_EMAIL: return "INVALID_EMAIL";
        case ErrorCode::INVALID_PASSWORD: return "INVALID_PASSWORD";
        case ErrorCode::INVALID_USERNAME: return "INVALID_USERNAME";
        case ErrorCode::USER_NOT_FOUND: return "USER_NOT_FOUND";
        case ErrorCode::USER_ALREADY_EXISTS: return "USER_ALREADY_EXISTS";
        case ErrorCode::USER_DISABLED: return "USER_DISABLED";
        case ErrorCode::ANN_NOT_FOUND: return "ANN_NOT_FOUND";
        case ErrorCode::ANN_ALREADY_EXISTS: return "ANN_ALREADY_EXISTS";
        case ErrorCode::ANN_PUBLISHED: return "ANN_PUBLISHED";
        case ErrorCode::COMMENT_NOT_FOUND: return "COMMENT_NOT_FOUND";
        case ErrorCode::COMMENT_ALREADY_EXISTS: return "COMMENT_ALREADY_EXISTS";
        case ErrorCode::INVALID_ANN_STATUS: return "INVALID_ANN_STATUS";
        case ErrorCode::INVALID_DUE_DATE: return "INVALID_DUE_DATE";
        case ErrorCode::VALIDATION_FAILED_SCHEMA: return "VALIDATION_FAILED_SCHEMA";
        case ErrorCode::DATA_VALIDATION_ERROR: return "DATA_VALIDATION_ERROR";
        case ErrorCode::INVALID_JSON_FORMAT: return "INVALID_JSON_FORMAT";
        case ErrorCode::UPLOAD_FAILED: return "UPLOAD_FAILED";
        case ErrorCode::FILE_TOO_LARGE: return "FILE_TOO_LARGE";
        case ErrorCode::UNSUPPORTED_FILE_TYPE: return "UNSUPPORTED_FILE_TYPE";
        case ErrorCode::INVALID_PAGE_NUMBER: return "INVALID_PAGE_NUMBER";
        case ErrorCode::INVALID_PER_PAGE: return "INVALID_PER_PAGE";
        case ErrorCode::PAGE_LIMIT_EXCEEDED: return "PAGE_LIMIT_EXCEEDED";
        default: return "UNKNOWN_ERROR";
    }
}

int error_code_to_http_status(ErrorCode code) {
    switch (code) {
        case ErrorCode::SUCCESS: return status_codes::OK;
        case ErrorCode::BAD_REQUEST: return status_codes::BadRequest;
        case ErrorCode::UNAUTHORIZED: return status_codes::Unauthorized;
        case ErrorCode::FORBIDDEN: return status_codes::Forbidden;
        case ErrorCode::NOT_FOUND: return status_codes::NotFound;
        case ErrorCode::METHOD_NOT_ALLOWED: return status_codes::MethodNotAllowed;
        case ErrorCode::CONFLICT: return status_codes::Conflict;
        case ErrorCode::UNPROCESSABLE_ENTITY: return status_codes::UnprocessableEntity;
        case ErrorCode::INTERNAL_SERVER_ERROR: return status_codes::InternalError;
        case ErrorCode::SERVICE_UNAVAILABLE: return status_codes::ServiceUnavailable;
        case ErrorCode::VALIDATION_FAILED: return status_codes::UnprocessableEntity;
        case ErrorCode::AUTH_FAILED: return status_codes::Unauthorized;
        case ErrorCode::TOKEN_EXPIRED: return status_codes::Unauthorized;
        case ErrorCode::PERMISSION_DENIED: return status_codes::Forbidden;
        case ErrorCode::RESOURCE_NOT_FOUND: return status_codes::NotFound;
        case ErrorCode::DUPLICATE_RESOURCE: return status_codes::Conflict;
        case ErrorCode::DATABASE_ERROR: return status_codes::InternalError;
        case ErrorCode::INVALID_JSON_FORMAT: return status_codes::BadRequest;
        default: return status_codes::InternalError;
    }
}

// 成功响应创建
http_response create_success_response(
    const std::string& message,
    const value& data
) {
    StandardResponse response_data;
    response_data.success = true;
    response_data.message = message;
    response_data.data = data;
    response_data.error_code = "";
    response_data.http_status = status_codes::OK;
    
    return create_response(response_data);
}

http_response create_success_response(
    const value& data,
    const std::string& message
) {
    return create_success_response(message, data);
}

// 错误响应创建
http_response create_error_response(
    ErrorCode error_code,
    const std::string& message,
    const value& data
) {
    const int http_status = error_code_to_http_status(error_code);
    const std::string error_code_str = error_code_to_string(error_code);
    const std::string final_message = message.empty() ? error_code_str : message;
    
    return create_error_response(http_status, error_code_str, final_message, data);
}

http_response create_error_response(
    int http_status,
    const std::string& error_code,
    const std::string& message,
    const value& data
) {
    StandardResponse response_data;
    response_data.success = false;
    response_data.message = message;
    response_data.data = data;
    response_data.error_code = error_code;
    response_data.http_status = http_status;
    
    return create_response(response_data);
}

// 验证错误响应
http_response create_validation_error_response(
    const std::vector<std::pair<std::string, std::string>>& errors,
    const std::string& message
) {
    value::object json_errors;
    
    for (const auto& error_pair : errors) {
        const std::string& field = error_pair.first;
        const std::string& message = error_pair.second;
        
        auto field_it = json_errors.find(utility::conversions::to_string_t(field));
        if (field_it == json_errors.end()) {
            json_errors[utility::conversions::to_string_t(field)] = value::array();
        }
        
        json_errors[utility::conversions::to_string_t(field)]
            .as_array()
            .push_back(value::string(utility::conversions::to_string_t(message)));
    }
    
    value data;
    data[U("field_errors")] = value(json_errors);
    
    return create_error_response(ErrorCode::VALIDATION_FAILED, message, data);
}

http_response create_validation_error_response(
    const std::map<std::string, std::vector<std::string>>& field_errors,
    const std::string& message
) {
    value::object json_errors;
    
    for (const auto& [field, errors] : field_errors) {
        value::array error_array;
        for (const auto& error : errors) {
            error_array.push_back(value::string(utility::conversions::to_string_t(error)));
        }
        json_errors[utility::conversions::to_string_t(field)] = error_array;
    }
    
    value data;
    data[U("field_errors")] = value(json_errors);
    
    return create_error_response(ErrorCode::VALIDATION_FAILED, message, data);
}

// 分页响应创建
http_response create_paginated_response(
    const PaginationResponse& pagination_data,
    const std::string& message
) {
    value data;
    data[U("items")] = value::array(pagination_data.items);
    data[U("total")] = value::number(pagination_data.total);
    data[U("page")] = value::number(pagination_data.page);
    data[U("per_page")] = value::number(pagination_data.per_page);
    data[U("total_pages")] = value::number(pagination_data.total_pages);
    data[U("has_next")] = value::boolean(pagination_data.has_next);
    data[U("has_prev")] = value::boolean(pagination_data.has_prev);
    
    StandardResponse response_data;
    response_data.success = true;
    response_data.message = message;
    response_data.data = data;
    response_data.error_code = "";
    response_data.http_status = status_codes::OK;
    
    return create_response(response_data);
}

// 标准响应创建
http_response create_response(const StandardResponse& response_data) {
    int status_code = response_data.http_status.value_or(status_codes::OK);
    http_response response(status_code);
    
    value::object response_json;
    response_json[U("success")] = value::boolean(response_data.success);
    response_json[U("message")] = value::string(utility::conversions::to_string_t(response_data.message));
    
    if (!response_data.error_code.empty()) {
        response_json[U("error_code")] = value::string(utility::conversions::to_string_t(response_data.error_code));
    }
    
    if (response_data.data != value::null()) {
        response_json[U("data")] = response_data.data;
    }
    
    // 添加时间戳
    std::time_t now = std::time(nullptr);
    std::string timestamp = utility::conversions::to_utf8string(web::json::value::datetime(now).to_string());
    response_json[U("timestamp")] = value::string(utility::conversions::to_string_t(timestamp));
    
    response.set_body(value(response_json));
    
    // 添加标准头信息
    add_standard_headers(response, response_data.trace_id, response_data.headers);
    
    return response;
}

// 基础响应类型
http_response create_empty_response(int status_code) {
    http_response response(status_code);
    return response;
}

http_response create_not_found_response(
    const std::string& resource_type,
    const std::string& resource_id
) {
    std::string message = resource_type + " not found";
    if (!resource_id.empty()) {
        message += " (ID: " + resource_id + ")";
    }
    
    return create_error_response(ErrorCode::RESOURCE_NOT_FOUND, message);
}

http_response create_bad_request_response(const std::string& message) {
    return create_error_response(ErrorCode::BAD_REQUEST, message);
}

http_response create_internal_server_error_response(const std::string& message) {
    return create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, message);
}

http_response create_method_not_allowed_response(
    const std::vector<std::string>& allowed_methods
) {
    value data;
    if (!allowed_methods.empty()) {
        value::array methods_array;
        for (const auto& method : allowed_methods) {
            methods_array.push_back(value::string(utility::conversions::to_string_t(method)));
        }
        data[U("allowed_methods")] = methods_array;
    }
    
    std::string message = "Method not allowed";
    if (!allowed_methods.empty()) {
        message += ". Allowed methods: ";
        for (size_t i = 0; i < allowed_methods.size(); ++i) {
            if (i > 0) message += ", ";
            message += allowed_methods[i];
        }
    }
    
    http_response response = create_error_response(ErrorCode::METHOD_NOT_ALLOWED, message, data);
    
    // 添加Allow头
    if (!allowed_methods.empty()) {
        utility::string_t allow_header;
        for (size_t i = 0; i < allowed_methods.size(); ++i) {
            if (i > 0) allow_header += U(", ");
            allow_header += utility::conversions::to_string_t(allowed_methods[i]);
        }
        response.headers().add(U("Allow"), allow_header);
    }
    
    return response;
}

http_response create_redirect_response(
    const utility::string_t& location,
    int status_code
) {
    http_response response(status_code);
    response.headers().add(U("Location"), location);
    return response;
}

http_response create_options_response(
    const std::vector<std::string>& allowed_methods,
    const std::string& allow_origin
) {
    http_response response(status_codes::OK);
    
    // 设置CORS头
    set_cors_headers(response, allow_origin, allowed_methods);
    
    // 添加Allow头
    if (!allowed_methods.empty()) {
        utility::string_t allow_header;
        for (size_t i = 0; i < allowed_methods.size(); ++i) {
            if (i > 0) allow_header += U(", ");
            allow_header += utility::conversions::to_string_t(allowed_methods[i]);
        }
        response.headers().add(U("Allow"), allow_header);
    }
    
    return response;
}

http_response create_no_content_response() {
    return create_empty_response(status_codes::NoContent);
}

// 健康检查响应
http_response create_health_check_response(
    bool healthy,
    const std::string& status,
    const std::map<std::string, std::string>& details
) {
    value data;
    data[U("status")] = value::string(utility::conversions::to_string_t(status));
    
    if (!details.empty()) {
        value::object details_obj;
        for (const auto& [key, value] : details) {
            details_obj[utility::conversions::to_string_t(key)] = 
                value::string(utility::conversions::to_string_t(value));
        }
        data[U("details")] = value(details_obj);
    }
    
    int status_code = healthy ? status_codes::OK : status_codes::ServiceUnavailable;
    
    StandardResponse response_data;
    response_data.success = healthy;
    response_data.message = healthy ? "Service is running" : "Service is unhealthy";
    response_data.data = data;
    response_data.error_code = "";
    response_data.http_status = status_code;
    
    return create_response(response_data);
}

// 头信息操作
void add_standard_headers(
    http_response& response,
    const std::optional<std::string>& trace_id,
    const std::map<std::string, std::string>& additional_headers
) {
    // 设置Content-Type
    response.headers().add(U("Content-Type"), U("application/json; charset=utf-8"));
    
    // 设置CORS头
    set_cors_headers(response);
    
    // 设置安全头
    set_security_headers(response);
    
    // 设置缓存控制
    set_cache_headers(response);
    
    // 添加X-Trace-ID
    if (trace_id) {
        response.headers().add(U("X-Trace-ID"), utility::conversions::to_string_t(*trace_id));
    }
    
    // 添加自定义头
    for (const auto& [key, value] : additional_headers) {
        response.headers().add(
            utility::conversions::to_string_t(key),
            utility::conversions::to_string_t(value)
        );
    }
}

void set_cors_headers(
    http_response& response,
    const std::string& allow_origin,
    const std::vector<std::string>& allow_methods,
    const std::vector<std::string>& allow_headers,
    bool allow_credentials
) {
    response.headers().add(U("Access-Control-Allow-Origin"), utility::conversions::to_string_t(allow_origin));
    
    if (!allow_methods.empty()) {
        utility::string_t methods_str;
        for (size_t i = 0; i < allow_methods.size(); ++i) {
            if (i > 0) methods_str += U(", ");
            methods_str += utility::conversions::to_string_t(allow_methods[i]);
        }
        response.headers().add(U("Access-Control-Allow-Methods"), methods_str);
    }
    
    if (!allow_headers.empty()) {
        utility::string_t headers_str;
        for (size_t i = 0; i < allow_headers.size(); ++i) {
            if (i > 0) headers_str += U(", ");
            headers_str += utility::conversions::to_string_t(allow_headers[i]);
        }
        response.headers().add(U("Access-Control-Allow-Headers"), headers_str);
    }
    
    if (allow_credentials) {
        response.headers().add(U("Access-Control-Allow-Credentials"), U("true"));
    }
}

void set_cache_headers(
    http_response& response,
    int max_age,
    bool public_cache,
    const std::string& cache_control
) {
    if (!cache_control.empty()) {
        response.headers().add(U("Cache-Control"), utility::conversions::to_string_t(cache_control));
        return;
    }
    
    std::stringstream ss;
    ss << (public_cache ? "public" : "private") 
       << ", max-age=" << max_age 
       << ", no-cache, must-revalidate";
    
    response.headers().add(U("Cache-Control"), utility::conversions::to_string_t(ss.str()));
}

void set_security_headers(
    http_response& response,
    bool content_security_policy,
    bool x_content_type_options,
    bool x_frame_options,
    bool x_xss_protection
) {
    if (content_security_policy) {
        response.headers().add(U("Content-Security-Policy"), U("default-src 'self'; frame-ancestors 'none'"));
    }
    
    if (x_content_type_options) {
        response.headers().add(U("X-Content-Type-Options"), U("nosniff"));
    }
    
    if (x_frame_options) {
        response.headers().add(U("X-Frame-Options"), U("DENY"));
    }
    
    if (x_xss_protection) {
        response.headers().add(U("X-XSS-Protection"), U("1; mode=block"));
    }
}

// API文档响应
http_response create_api_doc_response(
    const std::string& api_version,
    const std::string& service_name,
    const std::vector<std::pair<std::string, std::string>>& endpoints
) {
    value data;
    data[U("service_name")] = value::string(utility::conversions::to_string_t(service_name));
    data[U("api_version")] = value::string(utility::conversions::to_string_t(api_version));
    data[U("documentation")] = value::string(U("API documentation is available"));
    
    if (!endpoints.empty()) {
        value::array endpoints_array;
        for (const auto& endpoint : endpoints) {
            value::object endpoint_obj;
            endpoint_obj[U("path")] = value::string(utility::conversions::to_string_t(endpoint.first));
            endpoint_obj[U("description")] = value::string(utility::conversions::to_string_t(endpoint.second));
            endpoints_array.push_back(endpoint_obj);
        }
        data[U("endpoints")] = endpoints_array;
    }
    
    return create_success_response(data, "API Documentation");
}

// JSON验证
bool is_valid_json(const utility::string_t& content) {
    try {
        value json_data = value::parse(content);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

JsonValidationResult validate_json_schema(
    const value& json_data,
    const std::map<std::string, std::string>& schema_requirements
) {
    JsonValidationResult result;
    result.valid = true;
    
    for (const auto& [key, type] : schema_requirements) {
        const auto& json_key = utility::conversions::to_string_t(key);
        
        if (!json_data.has_field(json_key)) {
            result.valid = false;
            result.errors.push_back("Missing required field: " + key);
            continue;
        }
        
        const auto& field = json_data.at(json_key);
        
        if (type == "string" && !field.is_string()) {
            result.valid = false;
            result.errors.push_back("Field " + key + " must be a string");
        } else if (type == "number" && !field.is_number()) {
            result.valid = false;
            result.errors.push_back("Field " + key + " must be a number");
        } else if (type == "boolean" && !field.is_boolean()) {
            result.valid = false;
            result.errors.push_back("Field " + key + " must be a boolean");
        } else if (type == "object" && !field.is_object()) {
            result.valid = false;
            result.errors.push_back("Field " + key + " must be an object");
        } else if (type == "array" && !field.is_array()) {
            result.valid = false;
            result.errors.push_back("Field " + key + " must be an array");
        } else if (type == "null" && !field.is_null()) {
            result.valid = false;
            result.errors.push_back("Field " + key + " must be null");
        }
    }
    
    if (!result.valid) {
        std::stringstream ss;
        ss << "Validation failed with " << result.errors.size() << " errors:";
        for (const auto& error : result.errors) {
            ss << " " << error << ";";
        }
        result.error_message = ss.str();
    }
    
    return result;
}

// 错误处理
http_response handle_json_parse_error(const std::exception& e) {
    std::string error_msg = "Invalid JSON format: " + std::string(e.what());
    return create_error_response(ErrorCode::INVALID_JSON_FORMAT, error_msg);
}

http_response handle_exception_response(
    const std::exception& e,
    const std::string& context,
    bool include_details
) {
    std::string error_msg = "Internal server error";
    
    if (!context.empty()) {
        error_msg += " in " + context;
    }
    
    if (include_details) {
        error_msg += ": " + std::string(e.what());
    }
    
    return create_error_response(ErrorCode::INTERNAL_SERVER_ERROR, error_msg);
}

std::map<std::string, std::string> generate_api_error_info(
    ErrorCode error_code,
    const std::string& message,
    const std::string& context
) {
    std::map<std::string, std::string> error_info;
    
    error_info["error_code"] = error_code_to_string(error_code);
    error_info["http_status"] = std::to_string(error_code_to_http_status(error_code));
    error_info["message"] = message.empty() ? error_code_to_string(error_code) : message;
    
    if (!context.empty()) {
        error_info["context"] = context;
    }
    
    return error_info;
}

// 速率限制响应
http_response create_rate_limit_response(
    int remaining,
    int limit,
    int64_t reset_time,
    const std::string& message
) {
    http_response response(status_codes::TooManyRequests);
    
    set_rate_limit_headers(response, remaining, limit, reset_time);
    
    StandardResponse response_data;
    response_data.success = false;
    response_data.message = message;
    response_data.data = value::null();
    response_data.error_code = "RATE_LIMIT_EXCEEDED";
    response_data.http_status = status_codes::TooManyRequests;
    
    return create_response(response_data);
}

void set_rate_limit_headers(
    http_response& response,
    int remaining,
    int limit,
    int64_t reset_time,
    const std::string& policy
) {
    response.headers().add(U("X-RateLimit-Limit"), utility::conversions::to_string_t(std::to_string(limit)));
    response.headers().add(U("X-RateLimit-Remaining"), utility::conversions::to_string_t(std::to_string(remaining)));
    response.headers().add(U("X-RateLimit-Reset"), utility::conversions::to_string_t(std::to_string(reset_time)));
    
    if (!policy.empty()) {
        response.headers().add(U("X-RateLimit-Policy"), utility::conversions::to_string_t(policy));
    }
}

} // namespace http
