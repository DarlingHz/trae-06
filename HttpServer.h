#pragma once
#include <string>
#include <map>
#include <functional>
#include <unordered_map>
#include <sstream>
#include "JsonUtils.h"

// HTTP Status Code Constants
constexpr int HTTP_200_OK = 200;
constexpr int HTTP_400_BAD_REQUEST = 400;
constexpr int HTTP_404_NOT_FOUND = 404;
constexpr int HTTP_500_INTERNAL_SERVER_ERROR = 500;

struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> queryParams;
    std::unordered_map<std::string, std::string> routeParams;
};

struct HttpResponse {
    int statusCode = 200;
    std::string body;
    std::string contentType = "application/json";
    std::unordered_map<std::string, std::string> headers;
};

class HttpServer {
public:
    using HandlerFunc = std::function<HttpResponse(const HttpRequest&)>;

    HttpServer(int port) : port_(port) {}

    void Get(const std::string& path, HandlerFunc handler);
    void Post(const std::string& path, HandlerFunc handler);
    void Put(const std::string& path, HandlerFunc handler);

    void Run();

private:
    struct Route {
        std::string method;
        std::vector<std::string> parts;
        bool hasParams;
        HandlerFunc handler;
    };

    int port_;
    std::vector<Route> routes_;

    std::string readAll(int fd) const;
    HttpRequest parseRequest(const std::string& request) const;
    std::string buildResponse(const HttpResponse& response) const;
    Route* matchRoute(const HttpRequest& request) const;
    std::unordered_map<std::string, std::string> parseQueryParams(const std::string& query) const;
    void setCORSHeaders(HttpResponse& response) const;
};

static inline std::string CreateErrorResponse(int statusCode, const std::string& message) {
    std::stringstream ss;
    ss << "{";
    ss << JsonUtils::ToJson("status", "error") << ",";
    ss << JsonUtils::ToJson("code", statusCode) << ",";
    ss << JsonUtils::ToJson("message", message);
    ss << "}";
    return ss.str();
}
