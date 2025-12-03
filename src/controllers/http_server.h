#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include <functional>
#include <sstream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <errno.h>

#include "../services/user_service.h"
#include "../services/device_service.h"
#include "../services/warranty_service.h"
#include "../services/service_center_service.h"
#include "../services/repair_service.h"
#include "../services/cache_service.h"
#include "../utils/json_utils.h"
#include "../utils/logger.h"
#include "../utils/config.h"

class HttpRequest {
public:
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    std::map<std::string, std::string> queryParams;

    HttpRequest() = default;
    HttpRequest(const std::string& rawRequest) {
        parse(rawRequest);
    }

private:
    void parse(const std::string& rawRequest) {
        size_t pos = rawRequest.find("\r\n\r\n");
        std::string headerPart = rawRequest.substr(0, pos);
        if (pos != std::string::npos) {
            body = rawRequest.substr(pos + 4);
        }

        std::istringstream headerStream(headerPart);
        std::string line;

        // 解析请求行
        if (std::getline(headerStream, line)) {
            std::istringstream requestLine(line);
            requestLine >> method >> path >> version;

            // 解析查询参数
            size_t qPos = path.find('?');
            if (qPos != std::string::npos) {
                std::string query = path.substr(qPos + 1);
                path = path.substr(0, qPos);
                parseQueryParams(query);
            }
        }

        // 解析头部
        while (std::getline(headerStream, line)) {
            if (line.empty() || line == "\r") continue;
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 1);
                // 去除空格
                size_t startPos = value.find_first_not_of(" ");
                if (startPos != std::string::npos) {
                    value = value.substr(startPos);
                }
                size_t endPos = value.find_last_not_of("\r");
                if (endPos != std::string::npos) {
                    value = value.substr(0, endPos + 1);
                }
                headers[key] = value;
            }
        }
    }

    void parseQueryParams(const std::string& query) {
        std::istringstream ss(query);
        std::string pair;
        while (std::getline(ss, pair, '&')) {
            size_t eqPos = pair.find('=');
            if (eqPos != std::string::npos) {
                std::string key = pair.substr(0, eqPos);
                std::string value = pair.substr(eqPos + 1);
                queryParams[key] = value;
            }
        }
    }
};

class HttpResponse {
public:
    int statusCode;
    std::string statusMessage;
    std::map<std::string, std::string> headers;
    std::string body;

    HttpResponse() : statusCode(200), statusMessage("OK") {
        headers["Content-Type"] = "application/json";
    }

    HttpResponse(int code, const std::string& msg) : statusCode(code), statusMessage(msg) {
        headers["Content-Type"] = "application/json";
    }

    std::string toHttpString() const {
        std::ostringstream oss;
        oss << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
        for (const auto& header : headers) {
            oss << header.first << ": " << header.second << "\r\n";
        }
        oss << "Content-Length: " << body.size() << "\r\n";
        oss << "\r\n" << body;
        return oss.str();
    }

    void setJsonResponse(int code, const std::string& msg, const JsonObject& data = JsonObject()) {
        statusCode = code;
        statusMessage = code == 200 ? "OK" : "Error";
        
        JsonObject jsonResponse;
        jsonResponse.set("code", code);
        jsonResponse.set("msg", msg);
        jsonResponse.set("data", data);
        body = jsonResponse.toString();
    }
};

class HttpServer {
public:
    using HandlerFunc = std::function<HttpResponse(const HttpRequest&)>;

    void init(int port = 8080) {
        port_ = port;
        setupRoutes();
        start();
    }

private:
    int port_;
    int serverSocket_;
    bool running_ = false;

    std::map<std::string, std::map<std::string, HandlerFunc>> routes_;

    void setupRoutes() {
        // 用户相关
        addRoute("POST", "/api/users", handleCreateUser);
        addRoute("GET", "/api/users/(\\d+)", handleGetUser);
        
        // 设备相关
        addRoute("POST", "/api/devices", handleCreateDevice);
        addRoute("GET", "/api/users/(\\d+)/devices", handleGetUserDevices);
        addRoute("GET", "/api/users/(\\d+)/warranty_upcoming", handleGetWarrantyUpcoming);
        
        // 保修策略相关
        addRoute("POST", "/api/devices/(\\d+)/warranties", handleCreateWarranty);
        addRoute("GET", "/api/devices/(\\d+)/warranties", handleGetDeviceWarranties);
        addRoute("DELETE", "/api/warranties/(\\d+)", handleDeleteWarranty);
        
        // 维修单相关
        addRoute("POST", "/api/repair_orders", handleCreateRepairOrder);
        addRoute("GET", "/api/repair_orders", handleGetRepairOrders);
        addRoute("GET", "/api/repair_orders/(\\d+)", handleGetRepairOrder);
        addRoute("PATCH", "/api/repair_orders/(\\d+)/status", handleUpdateRepairStatus);
        
        // 统计相关
        addRoute("GET", "/api/statistics/repair", handleGetRepairStatistics);
    }

    void addRoute(const std::string& method, const std::string& pathPattern, HandlerFunc handler) {
        routes_[method][pathPattern] = handler;
    }

    void start() {
        // 创建socket
        serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket_ < 0) {
            LOG_ERROR("Failed to create socket: %s", strerror(errno));
            return;
        }

        // 设置socket选项
        int opt = 1;
        if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            LOG_ERROR("Failed to set socket options: %s", strerror(errno));
            close(serverSocket_);
            return;
        }

        // 设置非阻塞模式
        if (fcntl(serverSocket_, F_SETFL, O_NONBLOCK) < 0) {
            LOG_ERROR("Failed to set socket non-blocking: %s", strerror(errno));
            close(serverSocket_);
            return;
        }

        // 绑定地址
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port_);

        if (bind(serverSocket_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            LOG_ERROR("Failed to bind socket: %s", strerror(errno));
            close(serverSocket_);
            return;
        }

        // 监听连接
        if (listen(serverSocket_, 10) < 0) {
            LOG_ERROR("Failed to listen on socket: %s", strerror(errno));
            close(serverSocket_);
            return;
        }

        LOG_INFO("HTTP server started on port %d", port_);
        running_ = true;
        acceptConnections();
    }

    void acceptConnections() {
        while (running_) {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            
            int clientSocket = accept(serverSocket_, (struct sockaddr*)&clientAddr, &clientAddrLen);
            if (clientSocket < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    // 没有连接，稍等
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
                LOG_ERROR("Failed to accept connection: %s", strerror(errno));
                continue;
            }

            // 处理连接
            std::thread(&HttpServer::handleConnection, this, clientSocket).detach();
        }
    }

    void handleConnection(int clientSocket) {
        char buffer[4096];
        std::string requestData;

        while (running_) {
            ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
            if (bytesRead < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    // 没有数据，稍等
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
                LOG_ERROR("Failed to read from client: %s", strerror(errno));
                break;
            } else if (bytesRead == 0) {
                // 客户端断开连接
                break;
            }

            buffer[bytesRead] = '\0';
            requestData += buffer;

            // 检查是否完整接收到请求
            if (requestData.find("\r\n\r\n") != std::string::npos) {
                HttpRequest request(requestData);
                HttpResponse response = handleRequest(request);
                sendResponse(clientSocket, response);
                break;
            }
        }

        close(clientSocket);
    }

    HttpResponse handleRequest(const HttpRequest& request) {
        try {
            // 查找路由
            for (const auto& routeGroup : routes_) {
                if (routeGroup.first != request.method) {
                    continue;
                }

                for (const auto& route : routeGroup.second) {
                    std::string pathPattern = route.first;
                    HandlerFunc handler = route.second;

                    // 简单的路由匹配（不支持参数提取）
                    if (request.path == pathPattern) {
                        return handler(request);
                    }
                }
            }

            return HttpResponse(404, "Not Found");
        } catch (const std::exception& e) {
            LOG_ERROR("Request handling error: %s", e.what());
            return HttpResponse(500, "Internal Server Error");
        }
    }

    void sendResponse(int clientSocket, const HttpResponse& response) {
        std::string httpResponse = response.toHttpString();
        ssize_t bytesSent = send(clientSocket, httpResponse.c_str(), httpResponse.size(), 0);
        if (bytesSent < 0) {
            LOG_ERROR("Failed to send response: %s", strerror(errno));
        }
    }

    static HttpResponse handleCreateUser(const HttpRequest& request) {
        try {
            JsonValue json = parseJson(request.body);
            if (json.getType() != JsonValue::Type::Object) {
                return HttpResponse(400, "Invalid JSON body");
            }

            JsonObject obj = json.asObject();
            std::string name = obj.has("name") ? obj["name"].asString() : "";
            std::string email = obj.has("email") ? obj["email"].asString() : "";

            if (name.empty() || email.empty()) {
                return HttpResponse(400, "Name and email are required");
            }

            auto user = UserService::getInstance().createUser(name, email);
            
            JsonObject data;
            data.set("id", user->id);
            data.set("name", user->name);
            data.set("email", user->email);
            data.set("created_at", user->createdAt);

            HttpResponse response;
            response.setJsonResponse(0, "ok", data);
            return response;
        } catch (const std::exception& e) {
            HttpResponse response;
            response.setJsonResponse(400, e.what());
            return response;
        }
    }

    static HttpResponse handleGetUser(const HttpRequest& request) {
        try {
            // 从路径中提取用户ID（需要改进路由匹配）
            size_t pos = request.path.find_last_of('/');
            if (pos == std::string::npos) {
                return HttpResponse(400, "Invalid user ID");
            }
            
            std::string idStr = request.path.substr(pos + 1);
            int userId = std::stoi(idStr);

            auto user = UserService::getInstance().getUserById(userId);
            
            JsonObject data;
            data.set("id", user->id);
            data.set("name", user->name);
            data.set("email", user->email);
            data.set("created_at", user->createdAt);

            HttpResponse response;
            response.setJsonResponse(0, "ok", data);
            return response;
        } catch (const std::invalid_argument&) {
            return HttpResponse(400, "Invalid user ID format");
        } catch (const std::exception& e) {
            HttpResponse response;
            response.setJsonResponse(404, e.what());
            return response;
        }
    }

    // 其他处理方法留空，后续实现
    static HttpResponse handleCreateDevice(const HttpRequest& request) {
        return HttpResponse(501, "Not Implemented");
    }

    static HttpResponse handleGetUserDevices(const HttpRequest& request) {
        return HttpResponse(501, "Not Implemented");
    }

    static HttpResponse handleGetWarrantyUpcoming(const HttpRequest& request) {
        return HttpResponse(501, "Not Implemented");
    }

    static HttpResponse handleCreateWarranty(const HttpRequest& request) {
        return HttpResponse(501, "Not Implemented");
    }

    static HttpResponse handleGetDeviceWarranties(const HttpRequest& request) {
        return HttpResponse(501, "Not Implemented");
    }

    static HttpResponse handleDeleteWarranty(const HttpRequest& request) {
        return HttpResponse(501, "Not Implemented");
    }

    static HttpResponse handleCreateRepairOrder(const HttpRequest& request) {
        return HttpResponse(501, "Not Implemented");
    }

    static HttpResponse handleGetRepairOrders(const HttpRequest& request) {
        return HttpResponse(501, "Not Implemented");
    }

    static HttpResponse handleGetRepairOrder(const HttpRequest& request) {
        return HttpResponse(501, "Not Implemented");
    }

    static HttpResponse handleUpdateRepairStatus(const HttpRequest& request) {
        return HttpResponse(501, "Not Implemented");
    }

    static HttpResponse handleGetRepairStatistics(const HttpRequest& request) {
        return HttpResponse(501, "Not Implemented");
    }
};
