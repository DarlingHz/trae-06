#ifndef CPP_HTTPLIB_HTTPLIB_H
#define CPP_HTTPLIB_HTTPLIB_H

#include <string>
#include <map>
#include <functional>

namespace httplib {

struct Request {
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;
    std::map<std::string, std::string> params;
    std::vector<std::string> matches;
};

struct Response {
    int status;
    std::map<std::string, std::string> headers;
    std::string body;

    void set_content(const std::string& content, const std::string& type) {
        body = content;
        headers["Content-Type"] = type;
    }
};

class Server {
public:
    using Handler = std::function<void(const Request&, Response&)>;

    bool listen(const std::string& host, int port) {
        return true;
    }

    void Get(const std::string& pattern, const Handler& handler) {
        handlers_["GET"][pattern] = handler;
    }

    void Post(const std::string& pattern, const Handler& handler) {
        handlers_["POST"][pattern] = handler;
    }

    void Put(const std::string& pattern, const Handler& handler) {
        handlers_["PUT"][pattern] = handler;
    }

    void Delete(const std::string& pattern, const Handler& handler) {
        handlers_["DELETE"][pattern] = handler;
    }

private:
    std::map<std::string, std::map<std::string, Handler>> handlers_;
};

} // namespace httplib

#endif // CPP_HTTPLIB_HTTPLIB_H