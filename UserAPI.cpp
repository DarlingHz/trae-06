#include "UserAPI.h"
#include <sstream>
#include <regex>

std::string UserAPI::userToJson(const User& user) {
    std::stringstream ss;
    ss << "{";
    ss << ToJson("user_id", user.id) << ",";
    ss << ToJson("nickname", user.nickname) << ",";
    ss << ToJson("created_at", user.createdAt);
    ss << "}";
    return ss.str();
}

HttpResponse UserAPI::createUser(const HttpRequest& request) {
    try {
        std::string nickname;
        std::regex nicknameRx(R"("nickname":\s*"([^"]+)")");
        std::smatch match;

        if (std::regex_search(request.body, match, nicknameRx)) {
            nickname = match[1];
        }

        if (nickname.empty()) {
            return {400, CreateErrorResponse(400, "Nickname is required")};
        }

        int userId = DAO::getInstance().createUser(nickname);
        if (userId < 0) {
            return {500, CreateErrorResponse(500, "Failed to create user")};
        }

        std::optional<User> user = DAO::getInstance().getUserById(userId);
        if (!user) {
            return {500, CreateErrorResponse(500, "User not found after creation")};
        }

        return {201, userToJson(user.value())};
    } catch (...) {
        return {500, CreateErrorResponse(500, "Internal server error")};
    }
}

HttpResponse UserAPI::getUser(const HttpRequest& request) {
    try {
        std::string path = request.path;
        std::regex idRx(R"(/users/(\d+))");
        std::smatch match;

        if (!std::regex_match(path, match, idRx)) {
            return {400, CreateErrorResponse(400, "Invalid user ID")};
        }

        int userId = std::stoi(match[1]);
        std::optional<User> user = DAO::getInstance().getUserById(userId);
        if (!user) {
            return {404, CreateErrorResponse(404, "User not found")};
        }

        return {200, userToJson(user.value())};
    } catch (...) {
        return {500, CreateErrorResponse(500, "Internal server error")};
    }
}
