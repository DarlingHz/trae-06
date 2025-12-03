#ifndef RESPONSE_UTIL_H
#define RESPONSE_UTIL_H

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <ctime>

namespace http {

using namespace web;
using namespace web::http;
using namespace web::json;

// 标准响应结构体
struct StandardResponse {
    bool success;
    value data;
    std::string message;
    std::string error_code;
    std::optional<int> http_status;
    std::optional<std::string> trace_id;
    std::map<std::string, std::string> headers;
};

// 分页响应结构体
struct PaginationResponse {
    std::vector<value> items;
    std::int64_t total;
    int page;
    int per_page;
    int total_pages;
    bool has_next;
    bool has_prev;
};

// 错误码枚举
enum class ErrorCode {
    SUCCESS = 0,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    CONFLICT = 409,
    UNPROCESSABLE_ENTITY = 422,
    INTERNAL_SERVER_ERROR = 500,
    SERVICE_UNAVAILABLE = 503,
    INVALID_INPUT = 1001,
    VALIDATION_FAILED = 1002,
    AUTH_FAILED = 1003,
    TOKEN_EXPIRED = 1004,
    PERMISSION_DENIED = 1005,
    RESOURCE_NOT_FOUND = 1006,
    DUPLICATE_RESOURCE = 1007,
    OPERATION_FAILED = 1008,
    DATABASE_ERROR = 1009,
    NETWORK_ERROR = 1010,
    VALIDATION_ERROR = 2001,
    INVALID_EMAIL = 2002,
    INVALID_PASSWORD = 2003,
    INVALID_USERNAME = 2004,
    USER_NOT_FOUND = 3001,
    USER_ALREADY_EXISTS = 3002,
    USER_DISABLED = 3003,
    ANN_NOT_FOUND = 4001,
    ANN_ALREADY_EXISTS = 4002,
    ANN_PUBLISHED = 4003,
    COMMENT_NOT_FOUND = 5001,
    COMMENT_ALREADY_EXISTS = 5002,
    INVALID_ANN_STATUS = 6001,
    INVALID_DUE_DATE = 6002,
    VALIDATION_FAILED_SCHEMA = 7001,
    DATA_VALIDATION_ERROR = 7002,
    INVALID_JSON_FORMAT = 8001,
    UPLOAD_FAILED = 9001,
    FILE_TOO_LARGE = 9002,
    UNSUPPORTED_FILE_TYPE = 9003,
    INVALID_PAGE_NUMBER = 10001,
    INVALID_PER_PAGE = 10002,
    PAGE_LIMIT_EXCEEDED = 10003
};

// 错误码转换函数
std::string error_code_to_string(ErrorCode code);
int error_code_to_http_status(ErrorCode code);

// 生成成功响应
http_response create_success_response(
    const std::string& message = "Success",
    const value& data = value::null()
);

http_response create_success_response(
    const value& data,
    const std::string& message = "Success"
);

// 生成错误响应
http_response create_error_response(
    ErrorCode error_code,
    const std::string& message = "",
    const value& data = value::null()
);

http_response create_error_response(
    int http_status,
    const std::string& error_code,
    const std::string& message = "",
    const value& data = value::null()
);

// 生成验证错误响应
http_response create_validation_error_response(
    const std::vector<std::pair<std::string, std::string>>& errors,
    const std::string& message = "Validation failed"
);

http_response create_validation_error_response(
    const std::map<std::string, std::vector<std::string>>& field_errors,
    const std::string& message = "Validation failed"
);

// 生成分页响应
http_response create_paginated_response(
    const PaginationResponse& pagination_data,
    const std::string& message = "Success"
);

// 生成标准响应
http_response create_response(const StandardResponse& response_data);

// 生成空响应
http_response create_empty_response(int status_code = status_codes::OK);

// 生成404响应
http_response create_not_found_response(
    const std::string& resource_type = "Resource",
    const std::string& resource_id = ""
);

// 生成400响应
http_response create_bad_request_response(
    const std::string& message = "Bad request"
);

// 生成500响应
http_response create_internal_server_error_response(
    const std::string& message = "Internal server error"
);

// 生成405方法不允许响应
http_response create_method_not_allowed_response(
    const std::vector<std::string>& allowed_methods
);

// 生成重定向响应
http_response create_redirect_response(
    const utility::string_t& location,
    int status_code = status_codes::Found
);

// 生成OPTIONS响应
http_response create_options_response(
    const std::vector<std::string>& allowed_methods,
    const std::string& allow_origin = "*"
);

// 生成无内容响应
http_response create_no_content_response();

// 生成健康检查响应
http_response create_health_check_response(
    bool healthy = true,
    const std::string& status = "ok",
    const std::map<std::string, std::string>& details = {}
);

// 向响应添加标准头信息
void add_standard_headers(
    http_response& response,
    const std::optional<std::string>& trace_id = std::nullopt,
    const std::map<std::string, std::string>& additional_headers = {}
);

// 设置CORS头信息
void set_cors_headers(
    http_response& response,
    const std::string& allow_origin = "*",
    const std::vector<std::string>& allow_methods = {
        "GET", "POST", "PUT", "DELETE", "OPTIONS"
    },
    const std::vector<std::string>& allow_headers = {
        "Content-Type", "Authorization", "X-Trace-ID"
    },
    bool allow_credentials = false
);

// 设置缓存头信息
void set_cache_headers(
    http_response& response,
    int max_age = 3600,
    bool public_cache = false,
    const std::string& cache_control = ""
);

// 设置安全头信息
void set_security_headers(
    http_response& response,
    bool content_security_policy = true,
    bool x_content_type_options = true,
    bool x_frame_options = true,
    bool x_xss_protection = true
);

// 创建API文档响应
http_response create_api_doc_response(
    const std::string& api_version = "1.0",
    const std::string& service_name = "API Service",
    const std::vector<std::pair<std::string, std::string>>& endpoints = {}
);

// 验证JSON格式
bool is_valid_json(const utility::string_t& content);

// JSON验证结果结构体
struct JsonValidationResult {
    bool valid;
    std::vector<std::string> errors;
    std::string error_message;
};

// 验证JSON结构
JsonValidationResult validate_json_schema(
    const value& json_data,
    const std::map<std::string, std::string>& schema_requirements
);

// 处理JSON解析错误
http_response handle_json_parse_error(const std::exception& e);

// 处理异常响应
http_response handle_exception_response(
    const std::exception& e,
    const std::string& context = "",
    bool include_details = false
);

// 生成API错误信息
std::map<std::string, std::string> generate_api_error_info(
    ErrorCode error_code,
    const std::string& message = "",
    const std::string& context = ""
);

// 创建带有速率限制的响应
http_response create_rate_limit_response(
    int remaining,
    int limit,
    int64_t reset_time,
    const std::string& message = "Rate limit exceeded"
);

// 设置速率限制头信息
void set_rate_limit_headers(
    http_response& response,
    int remaining,
    int limit,
    int64_t reset_time,
    const std::string& policy = ""
);

} // namespace http

#endif // RESPONSE_UTIL_H
