#include "HttpServer.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "JsonUtils.h"

void HttpServer::Get(const std::string& path, HandlerFunc handler) {
    routes_.push_back({
        "GET",
        [&]() {
            std::vector<std::string> parts;
            std::stringstream ss(path);
            std::string part;
            while (std::getline(ss, part, '/')) {
                if (!part.empty()) {
                    parts.push_back(part);
                }
            }
            return parts;
        }(),
        path.find('{') != std::string::npos,
        handler
    });
}

void HttpServer::Post(const std::string& path, HandlerFunc handler) {
    routes_.push_back({
        "POST",
        [&]() {
            std::vector<std::string> parts;
            std::stringstream ss(path);
            std::string part;
            while (std::getline(ss, part, '/')) {
                if (!part.empty()) {
                    parts.push_back(part);
                }
            }
            return parts;
        }(),
        path.find('{') != std::string::npos,
        handler
    });
}

void HttpServer::Put(const std::string& path, HandlerFunc handler) {
    routes_.push_back({
        "PUT",
        [&]() {
            std::vector<std::string> parts;
            std::stringstream ss(path);
            std::string part;
            while (std::getline(ss, part, '/')) {
                if (!part.empty()) {
                    parts.push_back(part);
                }
            }
            return parts;
        }(),
        path.find('{') != std::string::npos,
        handler
    });
}

HttpServer::Route* HttpServer::matchRoute(const HttpRequest& request) const {
    std::vector<std::string> requestParts;
    std::stringstream ss(request.path);
    std::string part;
    while (std::getline(ss, part, '/')) {
        if (!part.empty()) {
            requestParts.push_back(part);
        }
    }

    for (auto& route : routes_) {
        if (route.method != request.method) continue;
        if (route.parts.size() != requestParts.size()) continue;

        bool match = true;
        for (size_t i = 0; i < route.parts.size(); ++i) {
            const auto& routePart = route.parts[i];
            const auto& reqPart = requestParts[i];

            if (routePart[0] == '{' && routePart.back() == '}') {
                continue;
            } else if (routePart != reqPart) {
                match = false;
                break;
            }
        }

        if (match) {
            return const_cast<Route*>(&route);
        }
    }

    return nullptr;
}

std::unordered_map<std::string, std::string> HttpServer::parseQueryParams(const std::string& query) const {
    std::unordered_map<std::string, std::string> params;
    if (query.empty()) return params;

    size_t pos = 0;
    size_t start = 0;
    while (pos < query.size()) {
        if (query[pos] == '&') {
            std::string param = query.substr(start, pos - start);
            size_t eq = param.find('=');
            if (eq != std::string::npos) {
                std::string key = param.substr(0, eq);
                std::string value = param.substr(eq + 1);
                params[key] = value;
            }
            start = pos + 1;
        }
        pos++;
    }

    if (start < query.size()) {
        std::string param = query.substr(start);
        size_t eq = param.find('=');
        if (eq != std::string::npos) {
            std::string key = param.substr(0, eq);
            std::string value = param.substr(eq + 1);
            params[key] = value;
        }
    }

    return params;
}

std::string HttpServer::readAll(int fd) const {
    std::string result;
    char buffer[4096];
    ssize_t bytesRead;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        result.append(buffer, bytesRead);
        if (bytesRead < sizeof(buffer)) {
            break;
        }
    }

    return result;
}

HttpRequest HttpServer::parseRequest(const std::string& request) const {
    HttpRequest req;
    std::stringstream ss(request);
    std::string line;

    std::getline(ss, line);
    if (line.size() > 0 && line.back() == '\r') line.pop_back();
    std::istringstream reqLine(line);
    reqLine >> req.method >> req.path;

    size_t queryPos = req.path.find('?');
    if (queryPos != std::string::npos) {
        std::string query = req.path.substr(queryPos + 1);
        req.queryParams = parseQueryParams(query);
        req.path = req.path.substr(0, queryPos);
    }

    while (std::getline(ss, line)) {
        if (line.size() > 0 && line.back() == '\r') line.pop_back();
        if (line.empty()) break;

        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            if (!value.empty() && value[0] == ' ') value.erase(0, 1);
            req.headers[key] = value;
        }
    }

    std::string body;
    while (std::getline(ss, line)) {
        if (line.size() > 0 && line.back() == '\r') line.pop_back();
        body += line + "\n";
    }
    req.body = body;

    return req;
}

void HttpServer::setCORSHeaders(HttpResponse& response) const {
    response.headers["Access-Control-Allow-Origin"] = "*";
    response.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, OPTIONS";
    response.headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
}

std::string HttpServer::buildResponse(const HttpResponse& response) const {
    std::stringstream ss;
    ss << "HTTP/1.1 " << response.statusCode << " OK\r\n";
    ss << "Content-Type: " << response.contentType << "\r\n";
    ss << "Content-Length: " << response.body.size() << "\r\n";

    for (const auto& header : response.headers) {
        ss << header.first << ": " << header.second << "\r\n";
    }

    ss << "\r\n" << response.body;
    return ss.str();
}

void HttpServer::Run() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(serverSocket);
        return;
    }

    if (listen(serverSocket, 10) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(serverSocket);
        return;
    }

    std::cout << "HTTP server running on port " << port_ << std::endl;

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            std::cerr << "Failed to accept client connection" << std::endl;
            continue;
        }

        std::string requestStr = readAll(clientSocket);
        HttpRequest request = parseRequest(requestStr);

        if (request.method == "OPTIONS") {
            HttpResponse response;
            response.statusCode = 204;
            setCORSHeaders(response);
            std::string responseStr = buildResponse(response);
            write(clientSocket, responseStr.c_str(), responseStr.size());
            close(clientSocket);
            continue;
        }

        Route* route = matchRoute(request);
        HttpResponse response;

        if (route) {
            try {
                response = route->handler(request);
                setCORSHeaders(response);
            } catch (...) {
                response = {500, "{\"error\": \"Internal Server Error\"}"};
                setCORSHeaders(response);
            }
        } else {
            response = {404, "{\"error\": \"Not Found\"}"};
            setCORSHeaders(response);
        }

        std::string responseStr = buildResponse(response);
        write(clientSocket, responseStr.c_str(), responseStr.size());
        close(clientSocket);
    }

    close(serverSocket);
}

std::string JsonEscape(const std::string& s) {
    std::stringstream ss;
    for (char c : s) {
        switch (c) {
            case '"': ss << "\\""";
                break;
            case '\\': ss << "\\\\";
                break;
            case '/': ss << "\\/";
                break;
            case '\b': ss << "\\b";
                break;
            case '\f': ss << "\\f";
                break;
            case '\n': ss << "\\n";
                break;
            case '\r': ss << "\\r";
                break;
            case '\t': ss << "\\t";
                break;
            default:
                ss << c;
        }
    }
    return ss.str();
}

std::string ToJson(const std::string& key, const std::string& value) {
    return "\"" + JsonEscape(key) + "\": \"" + JsonEscape(value) + "\"";
}

std::string ToJson(const std::string& key, int value) {
    return "\"" + JsonEscape(key) + "\": " + std::to_string(value);
}

std::string ToJson(const std::string& key, double value) {
    return "\"" + JsonEscape(key) + "\": " + std::to_string(value);
}

std::string ToJson(const std::string& key, bool value) {
    return "\"" + JsonEscape(key) + "\": " + (value ? "true" : "false");
}

std::string ToArrayJson(const std::vector<std::string>& arr) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        if (i > 0) ss << ",";
        ss << arr[i];
    }
    ss << "]";
    return ss.str();
}

std::string CreateErrorResponse(int statusCode, const std::string& message) {
    return "{\"error\": \"" + JsonEscape(message) + "\"}";
}
