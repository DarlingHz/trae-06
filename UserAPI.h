#pragma once
#include "HttpServer.h"
#include "DAO.h"

class UserAPI {
public:
    static HttpResponse createUser(const HttpRequest& request);
    static HttpResponse getUser(const HttpRequest& request);

private:
    static std::string userToJson(const User& user);
};
