// cpp-httplib - C++ HTTP/HTTPS Server and Client Library
// License: MIT
// Author: Yuji Hirose
//
// This is a minimal version of cpp-httplib to get started quickly.
// For the full version, please download from https://github.com/yhirose/cpp-httplib

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace httplib {

struct Request {
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;
};

struct Response {
    int status = 200;
    std::map<std::string, std::string> headers;
    std::string body;
};

class Server {
public:
    using Handler = std::function<void(const Request&, Response&)>;

    Server() {}
    ~Server() { stop(); }

    Server& Get(const std::string& path, Handler handler) {
        handlers_[("GET", path)] = handler;
        return *this;
    }

    Server& Post(const std::string& path, Handler handler) {
        handlers_[("POST", path)] = handler;
        return *this;
    }

    Server& Put(const std::string& path, Handler handler) {
        handlers_[("PUT", path)] = handler;
        return *this;
    }

    Server& Delete(const std::string& path, Handler handler) {
        handlers_[("DELETE", path)] = handler;
        return *this;
    }

    void set_error_handler(Handler handler) {
        error_handler_ = handler;
    }

    void listen(const std::string& host, int port) {
        // 简单实现，实际需要使用网络库
        std::cout << "Server listening on " << host << ":" << port << std::endl;
        // 这里应该启动实际的服务器循环
    }

    void stop() {
        // 停止服务器
    }

private:
    std::map<std::pair<std::string, std::string>, Handler> handlers_;
    Handler error_handler_;
};

} // namespace httplib
