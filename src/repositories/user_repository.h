#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include "../models/user.h"
#include "../db/db_pool.h"
#include "../utils/logger.h"

class UserRepository {
public:
    std::shared_ptr<User> create(const User& user) {
        if (!user.isValid()) {
            throw std::runtime_error("Invalid user data");
        }

        // 检查邮箱是否已存在
        if (findByEmail(user.email)) {
            throw std::runtime_error("Email already exists");
        }

        auto conn = DBPool::getInstance().getConnection();
        MYSQL* mysqlConn = conn->getConn();

        std::string query = "INSERT INTO users (name, email, created_at) VALUES (?, ?, ?)";
        MYSQL_STMT* stmt = mysql_stmt_init(mysqlConn);
        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            std::string err = mysql_error(mysqlConn);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to prepare statement: " + err);
        }

        time_t now = time(nullptr);
        MYSQL_BIND params[3] = {0};

        params[0].buffer_type = MYSQL_TYPE_STRING;
        params[0].buffer = const_cast<char*>(user.name.c_str());
        params[0].buffer_length = user.name.length();

        params[1].buffer_type = MYSQL_TYPE_STRING;
        params[1].buffer = const_cast<char*>(user.email.c_str());
        params[1].buffer_length = user.email.length();

        params[2].buffer_type = MYSQL_TYPE_LONG;
        params[2].buffer = &now;

        if (mysql_stmt_bind_param(stmt, params) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind parameters: " + err);
        }

        if (mysql_stmt_execute(stmt) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to execute statement: " + err);
        }

        int64_t userId = mysql_stmt_insert_id(stmt);
        mysql_stmt_close(stmt);

        return std::make_shared<User>(userId, user.name, user.email, now);
    }

    std::shared_ptr<User> findById(int id) {
        if (id <= 0) {
            throw std::runtime_error("Invalid user ID");
        }

        auto conn = DBPool::getInstance().getConnection();
        MYSQL* mysqlConn = conn->getConn();

        std::string query = "SELECT id, name, email, created_at FROM users WHERE id = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(mysqlConn);
        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            std::string err = mysql_error(mysqlConn);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to prepare statement: " + err);
        }

        MYSQL_BIND params[1] = {0};
        params[0].buffer_type = MYSQL_TYPE_LONG;
        params[0].buffer = &id;

        if (mysql_stmt_bind_param(stmt, params) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind parameters: " + err);
        }

        if (mysql_stmt_execute(stmt) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to execute statement: " + err);
        }

        MYSQL_BIND result[4] = {0};
        int userId;
        char name[256] = {0};
        char email[256] = {0};
        time_t createdAt;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &userId;

        result[1].buffer_type = MYSQL_TYPE_STRING;
        result[1].buffer = name;
        result[1].buffer_length = 256;

        result[2].buffer_type = MYSQL_TYPE_STRING;
        result[2].buffer = email;
        result[2].buffer_length = 256;

        result[3].buffer_type = MYSQL_TYPE_LONG;
        result[3].buffer = &createdAt;

        if (mysql_stmt_bind_result(stmt, result) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind result: " + err);
        }

        std::shared_ptr<User> user;
        if (mysql_stmt_fetch(stmt) == 0) {
            user = std::make_shared<User>(userId, std::string(name), std::string(email), createdAt);
        }

        mysql_stmt_close(stmt);
        return user;
    }

    std::shared_ptr<User> findByEmail(const std::string& email) {
        if (email.empty()) {
            throw std::runtime_error("Invalid email");
        }

        auto conn = DBPool::getInstance().getConnection();
        MYSQL* mysqlConn = conn->getConn();

        std::string query = "SELECT id, name, email, created_at FROM users WHERE email = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(mysqlConn);
        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            std::string err = mysql_error(mysqlConn);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to prepare statement: " + err);
        }

        MYSQL_BIND params[1] = {0};
        params[0].buffer_type = MYSQL_TYPE_STRING;
        params[0].buffer = const_cast<char*>(email.c_str());
        params[0].buffer_length = email.length();

        if (mysql_stmt_bind_param(stmt, params) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind parameters: " + err);
        }

        if (mysql_stmt_execute(stmt) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to execute statement: " + err);
        }

        MYSQL_BIND result[4] = {0};
        int userId;
        char name[256] = {0};
        char emailResult[256] = {0};
        time_t createdAt;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &userId;

        result[1].buffer_type = MYSQL_TYPE_STRING;
        result[1].buffer = name;
        result[1].buffer_length = 256;

        result[2].buffer_type = MYSQL_TYPE_STRING;
        result[2].buffer = emailResult;
        result[2].buffer_length = 256;

        result[3].buffer_type = MYSQL_TYPE_LONG;
        result[3].buffer = &createdAt;

        if (mysql_stmt_bind_result(stmt, result) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind result: " + err);
        }

        std::shared_ptr<User> user;
        if (mysql_stmt_fetch(stmt) == 0) {
            user = std::make_shared<User>(userId, std::string(name), std::string(emailResult), createdAt);
        }

        mysql_stmt_close(stmt);
        return user;
    }

private:
    // 禁止拷贝构造和赋值运算符
    UserRepository(const UserRepository&) = delete;
    UserRepository& operator=(const UserRepository&) = delete;

public:
    // 提供全局访问点
    static UserRepository& getInstance() {
        static UserRepository instance;
        return instance;
    }
};
