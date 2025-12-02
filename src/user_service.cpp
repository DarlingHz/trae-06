#include "user_service.hpp"
#include <iostream>
#include <stdexcept>
#include <random>
#include <chrono>
#include <sstream>

using namespace std;

json UserService::registerUser(const json& user_data) {
    json response;

    try {
        // 验证必填字段
        if (!user_data.contains("name") || !user_data.contains("email") || !user_data.contains("password")) {
            response["errorCode"] = "MISSING_FIELDS";
            response["message"] = "缺少必填字段: name, email, password";
            return response;
        }

        string name = user_data["name"];
        string email = user_data["email"];
        string password = user_data["password"];

        // 验证字段长度
        if (name.empty() || email.empty() || password.empty()) {
            response["errorCode"] = "EMPTY_FIELDS";
            response["message"] = "字段不能为空";
            return response;
        }

        // 检查邮箱是否已存在
        if (emailExists(email)) {
            response["errorCode"] = "EMAIL_EXISTS";
            response["message"] = "邮箱已被注册";
            return response;
        }

        // 开始事务
        if (!db_.beginTransaction()) {
            response["errorCode"] = "TRANSACTION_FAILED";
            response["message"] = "事务开始失败";
            return response;
        }

        // 插入用户数据
        stringstream insert_sql;
        insert_sql << "INSERT INTO users (name, email, password) VALUES ('" 
                   << name << "', '" << email << "', '" << password << "');";

        if (!db_.execute(insert_sql.str())) {
            db_.rollbackTransaction();
            response["errorCode"] = "INSERT_FAILED";
            response["message"] = "用户注册失败";
            return response;
        }

        // 获取新创建的用户 ID
        int user_id = sqlite3_last_insert_rowid(db_.getConnection());

        // 生成并存储 token
        string token = generateToken(user_id);
        if (!storeToken(user_id, token)) {
            db_.rollbackTransaction();
            response["errorCode"] = "TOKEN_STORE_FAILED";
            response["message"] = "Token 存储失败";
            return response;
        }

        // 提交事务
        if (!db_.commitTransaction()) {
            db_.rollbackTransaction();
            response["errorCode"] = "TRANSACTION_COMMIT_FAILED";
            response["message"] = "事务提交失败";
            return response;
        }

        // 返回成功响应
        response["userId"] = user_id;
        response["token"] = token;

    } catch (const exception& e) {
        db_.rollbackTransaction();
        response["errorCode"] = "INTERNAL_ERROR";
        response["message"] = "内部服务器错误: " + string(e.what());
        cerr << "用户注册错误: " << e.what() << endl;
    }

    return response;
}

json UserService::loginUser(const json& login_data) {
    json response;

    try {
        // 验证必填字段
        if (!login_data.contains("email") || !login_data.contains("password")) {
            response["errorCode"] = "MISSING_FIELDS";
            response["message"] = "缺少必填字段: email, password";
            return response;
        }

        string email = login_data["email"];
        string password = login_data["password"];

        // 验证字段长度
        if (email.empty() || password.empty()) {
            response["errorCode"] = "EMPTY_FIELDS";
            response["message"] = "字段不能为空";
            return response;
        }

        // 查询用户
        int user_id = -1;
        string stored_password;

        stringstream select_sql;
        select_sql << "SELECT id, password FROM users WHERE email = '" << email << "';";

        if (!db_.query(select_sql.str(), [&](int argc, char** argv, char** col_names) {
            if (argc >= 2 && argv[0] != nullptr && argv[1] != nullptr) {
                user_id = stoi(argv[0]);
                stored_password = argv[1];
            }
            return true;
        })) {
            response["errorCode"] = "QUERY_FAILED";
            response["message"] = "查询用户失败";
            return response;
        }

        // 验证用户是否存在
        if (user_id == -1) {
            response["errorCode"] = "USER_NOT_FOUND";
            response["message"] = "用户不存在";
            return response;
        }

        // 验证密码
        if (stored_password != password) {
            response["errorCode"] = "WRONG_PASSWORD";
            response["message"] = "密码错误";
            return response;
        }

        // 生成新的 token
        string token = generateToken(user_id);

        // 存储 token
        if (!storeToken(user_id, token)) {
            response["errorCode"] = "TOKEN_STORE_FAILED";
            response["message"] = "Token 存储失败";
            return response;
        }

        // 返回成功响应
        response["userId"] = user_id;
        response["token"] = token;

    } catch (const exception& e) {
        response["errorCode"] = "INTERNAL_ERROR";
        response["message"] = "内部服务器错误: " + string(e.what());
        cerr << "用户登录错误: " << e.what() << endl;
    }

    return response;
}

int UserService::verifyToken(const string& token) {
    if (token.empty()) {
        return -1;
    }

    int user_id = -1;

    stringstream select_sql;
    select_sql << "SELECT user_id FROM user_tokens WHERE token = '" << token << "' AND expires_at > CURRENT_TIMESTAMP;";

    db_.query(select_sql.str(), [&](int argc, char** argv, char** col_names) {
        if (argc >= 1 && argv[0] != nullptr) {
            user_id = stoi(argv[0]);
        }
        return true;
    });

    return user_id;
}

string UserService::generateToken(int user_id) {
    // 生成随机字符串作为 token
    string random_str = generateRandomString(32);

    // 组合用户 ID 和随机字符串
    stringstream token_ss;
    token_ss << user_id << ":" << random_str;

    return token_ss.str();
}

bool UserService::emailExists(const string& email) {
    bool exists = false;

    stringstream select_sql;
    select_sql << "SELECT id FROM users WHERE email = '" << email << "';";

    db_.query(select_sql.str(), [&](int argc, char** argv, char** col_names) {
        if (argc >= 1 && argv[0] != nullptr) {
            exists = true;
        }
        return true;
    });

    return exists;
}

string UserService::getTokenForUserId(int user_id) {
    string token;

    stringstream select_sql;
    select_sql << "SELECT token FROM user_tokens WHERE user_id = " << user_id << " AND expires_at > CURRENT_TIMESTAMP;";

    db_.query(select_sql.str(), [&](int argc, char** argv, char** col_names) {
        if (argc >= 1 && argv[0] != nullptr) {
            token = argv[0];
        }
        return true;
    });

    return token;
}

bool UserService::storeToken(int user_id, const string& token) {
    // 先删除该用户的所有过期 token
    stringstream delete_expired_sql;
    delete_expired_sql << "DELETE FROM user_tokens WHERE user_id = " << user_id << " AND expires_at <= CURRENT_TIMESTAMP;";

    if (!db_.execute(delete_expired_sql.str())) {
        cerr << "删除过期 token 失败" << endl;
        return false;
    }

    // 删除该用户的现有 token（如果存在）
    stringstream delete_existing_sql;
    delete_existing_sql << "DELETE FROM user_tokens WHERE user_id = " << user_id << " AND expires_at > CURRENT_TIMESTAMP;";

    if (!db_.execute(delete_existing_sql.str())) {
        cerr << "删除现有 token 失败" << endl;
        return false;
    }

    // 插入新的 token，有效期为 7 天
    stringstream insert_sql;
    insert_sql << "INSERT INTO user_tokens (user_id, token, expires_at) VALUES (" 
               << user_id << ", '" << token << "', datetime('now', '+7 days'));";

    if (!db_.execute(insert_sql.str())) {
        cerr << "插入新 token 失败" << endl;
        return false;
    }

    return true;
}

string UserService::generateRandomString(int length) {
    const string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    string result;

    // 使用当前时间作为随机种子
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    mt19937 generator(seed);
    uniform_int_distribution<int> distribution(0, charset.size() - 1);

    for (int i = 0; i < length; ++i) {
        result += charset[distribution(generator)];
    }

    return result;
}
