#pragma once

#include <hiredis/hiredis.h>
#include <string>
#include <stdexcept>

class RedisClient {
public:
    RedisClient() : context(nullptr) {}

    ~RedisClient() {
        if (context) {
            redisFree(context);
        }
    }

    RedisClient(const RedisClient&) = delete;
    RedisClient& operator=(const RedisClient&) = delete;

    void connect(const std::string& host, int port, const std::string& password = "", int db = 0) {
        context = redisConnect(host.c_str(), port);
        if (!context || context->err) {
            if (context) {
                std::string err = context->errstr;
                redisFree(context);
                context = nullptr;
                throw std::runtime_error("Redis connection failed: " + err);
            } else {
                throw std::runtime_error("Failed to allocate Redis context");
            }
        }

        if (!password.empty()) {
            redisReply* reply = static_cast<redisReply*>(redisCommand(context, "AUTH %s", password.c_str()));
            checkReply(reply);
            freeReplyObject(reply);
        }

        if (db > 0) {
            redisReply* reply = static_cast<redisReply*>(redisCommand(context, "SELECT %d", db));
            checkReply(reply);
            freeReplyObject(reply);
        }
    }

    std::string get(const std::string& key) {
        redisReply* reply = static_cast<redisReply*>(redisCommand(context, "GET %s", key.c_str()));
        checkReply(reply);

        std::string result;
        if (reply->type == REDIS_REPLY_STRING) {
            result = reply->str;
        }

        freeReplyObject(reply);
        return result;
    }

    void set(const std::string& key, const std::string& value, int expireSeconds = -1) {
        if (expireSeconds <= 0) {
            redisReply* reply = static_cast<redisReply*>(redisCommand(context, "SET %s %s", key.c_str(), value.c_str()));
            checkReply(reply);
            freeReplyObject(reply);
        } else {
            redisReply* reply = static_cast<redisReply*>(redisCommand(context, "SET %s %s EX %d", key.c_str(), value.c_str(), expireSeconds));
            checkReply(reply);
            freeReplyObject(reply);
        }
    }

    void del(const std::string& key) {
        redisReply* reply = static_cast<redisReply*>(redisCommand(context, "DEL %s", key.c_str()));
        checkReply(reply);
        freeReplyObject(reply);
    }

    bool exists(const std::string& key) {
        redisReply* reply = static_cast<redisReply*>(redisCommand(context, "EXISTS %s", key.c_str()));
        checkReply(reply);

        bool result = reply->integer > 0;
        freeReplyObject(reply);
        return result;
    }

private:
    redisContext* context;

    void checkReply(const redisReply* reply) {
        if (!reply || reply->type == REDIS_REPLY_ERROR) {
            if (reply) {
                std::string err = reply->str;
                freeReplyObject(const_cast<redisReply*>(reply));
                throw std::runtime_error("Redis command failed: " + err);
            } else {
                throw std::runtime_error("Redis command failed: no reply");
            }
        }
    }
};
