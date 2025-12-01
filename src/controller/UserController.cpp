#include "controller/UserController.h"
#include <iostream>
#include <ctime>
#include "util/Utils.h"

namespace controller {

UserController::UserController(dao::UserDao& user_dao) : user_dao_(user_dao) {
}

UserController::~UserController() {
}

json UserController::handleRegister(const json& request) const {
    json response;

    // 验证请求参数是否完整
    if (!request.contains("email") || !request.contains("password") ||
        !request.contains("nickname") || !request.contains("timezone")) {
        response["code"] = 400;
        response["message"] = "Missing required parameters";
        response["data"] = nullptr;
        return response;
    }

    // 获取请求参数
    std::string email = request["email"];
    std::string password = request["password"];
    std::string nickname = request["nickname"];
    std::string timezone = request["timezone"];

    // 验证email格式（简单验证）
    if (email.find("@") == std::string::npos) {
        response["code"] = 400;
        response["message"] = "Invalid email format";
        response["data"] = nullptr;
        return response;
    }

    // 验证密码长度
    if (password.length() < 6) {
        response["code"] = 400;
        response["message"] = "Password must be at least 6 characters long";
        response["data"] = nullptr;
        return response;
    }

    // 检查email是否已经存在
    model::User existing_user = user_dao_.findUserByEmail(email);
    if (existing_user.id != -1) {
        response["code"] = 400;
        response["message"] = "Email already exists";
        response["data"] = nullptr;
        return response;
    }

    // 对密码进行哈希处理
    std::string password_hash = hashPassword(password);

    // 生成随机token
    std::string token = generateToken();

    // 获取当前时间并格式化为ISO 8601字符串
    std::time_t now = std::time(nullptr);
    struct tm tm = *std::localtime(&now);
    std::string created_at = util::time::toIsoString(tm);

    // 创建用户对象
    model::User new_user;
    new_user.email = email;
    new_user.password_hash = password_hash;
    new_user.nickname = nickname;
    new_user.timezone = timezone;
    new_user.created_at = created_at;

    // 保存用户到数据库
    bool insert_success = user_dao_.insertUser(new_user);
    if (!insert_success) {
        response["code"] = 500;
        response["message"] = "Failed to create user";
        response["data"] = nullptr;
        return response;
    }

    // 获取新创建用户的ID
    model::User created_user = user_dao_.findUserByEmail(email);
    if (created_user.id == -1) {
        response["code"] = 500;
        response["message"] = "Failed to retrieve created user";
        response["data"] = nullptr;
        return response;
    }

    // 生成成功响应
    response["code"] = 0;
    response["message"] = "ok";
    json data;
    data["user_id"] = created_user.id;
    data["token"] = token;
    response["data"] = data;

    return response;
}

json UserController::handleLogin(const json& request) const {
    json response;

    // 验证请求参数是否完整
    if (!request.contains("email") || !request.contains("password")) {
        response["code"] = 400;
        response["message"] = "Missing required parameters";
        response["data"] = nullptr;
        return response;
    }

    // 获取请求参数
    std::string email = request["email"];
    std::string password = request["password"];

    // 根据email查询用户记录
    model::User user = user_dao_.findUserByEmail(email);
    if (user.id == -1) {
        response["code"] = 401;
        response["message"] = "Invalid email or password";
        response["data"] = nullptr;
        return response;
    }

    // 验证密码哈希是否匹配
    std::string password_hash = hashPassword(password);
    if (password_hash != user.password_hash) {
        response["code"] = 401;
        response["message"] = "Invalid email or password";
        response["data"] = nullptr;
        return response;
    }

    // 生成新的随机token
    std::string token = generateToken();

    // 生成成功响应
    response["code"] = 0;
    response["message"] = "ok";
    json data;
    data["user_id"] = user.id;
    data["token"] = token;
    response["data"] = data;

    return response;
}

std::string UserController::generateToken() const {
    // 生成一个32位的随机token
    return util::crypto::generateRandomString(32);
}

std::string UserController::hashPassword(const std::string& password) const {
    // 使用SHA256对密码进行哈希处理
    // 实际应用中应该使用更安全的哈希算法，比如bcrypt
    return util::crypto::sha256(password);
}

} // namespace controller
