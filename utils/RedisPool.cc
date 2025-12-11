#include "RedisPool.h"
#include <iostream>
#include <stdexcept>
#include <cstring>

namespace giftcard {

bool RedisPool::init(const std::string& host, int port, const std::string& password, 
                      int db, int max_connections, int timeout) {
    try {
        host_ = host;
        port_ = port;
        password_ = password;
        db_ = db;
        max_connections_ = max_connections;
        timeout_ = timeout;

        // 创建初始连接
        for (int i = 0; i < max_connections_ / 2; ++i) {
            auto conn = createConnection();
            if (conn) {
                connection_pool_.push(conn);
                idle_connections_++;
            } else {
                std::cerr << "Failed to create initial Redis connection" << std::endl;
                return false;
            }
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize Redis pool: " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<redisContext> RedisPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);

    // 等待可用连接
    while (connection_pool_.empty() && current_connections_ >= max_connections_) {
        cond_var_.wait(lock);
    }

    std::shared_ptr<redisContext> conn;

    // 如果连接池为空，但还可以创建新连接
    if (connection_pool_.empty() && current_connections_ < max_connections_) {
        conn = createConnection();
        if (conn) {
            current_connections_++;
        }
    } else {
        // 从连接池获取连接
        conn = connection_pool_.front();
        connection_pool_.pop();
        idle_connections_--;

        // 检查连接是否有效
        if (conn->err || redisPing(conn.get()) == nullptr) {
            std::cerr << "Redis connection is invalid, reconnecting..." << std::endl;
            conn.reset();
            conn = createConnection();
        }
    }

    return conn;
}

void RedisPool::close() {
    std::unique_lock<std::mutex> lock(mutex_);

    // 关闭所有连接
    while (!connection_pool_.empty()) {
        auto conn = connection_pool_.front();
        connection_pool_.pop();
        redisFree(conn.get());
        idle_connections_--;
        current_connections_--;
    }
}

void RedisPool::getStatus(int& current_connections, int& idle_connections) {
    std::unique_lock<std::mutex> lock(mutex_);
    current_connections = current_connections_;
    idle_connections = idle_connections_;
}

std::shared_ptr<redisContext> RedisPool::createConnection() {
    try {
        // 设置连接超时
        struct timeval tv;
        tv.tv_sec = timeout_ / 1000;
        tv.tv_usec = (timeout_ % 1000) * 1000;

        // 连接到Redis服务器
        redisContext* conn = redisConnectWithTimeout(host_.c_str(), port_, tv);
        if (!conn || conn->err) {
            if (conn) {
                std::cerr << "Failed to connect to Redis: " << conn->errstr << std::endl;
                redisFree(conn);
            } else {
                std::cerr << "Failed to allocate Redis context" << std::endl;
            }
            return nullptr;
        }

        // 如果设置了密码，进行身份验证
        if (!password_.empty()) {
            redisReply* reply = (redisReply*)redisCommand(conn, "AUTH %s", password_.c_str());
            if (!reply || reply->type == REDIS_REPLY_ERROR) {
                std::cerr << "Failed to authenticate Redis connection: " << 
                    (reply ? reply->str : "Unknown error") << std::endl;
                freeReplyObject(reply);
                redisFree(conn);
                return nullptr;
            }
            freeReplyObject(reply);
        }

        // 选择数据库
        if (db_ >= 0) {
            redisReply* reply = (redisReply*)redisCommand(conn, "SELECT %d", db_);
            if (!reply || reply->type == REDIS_REPLY_ERROR) {
                std::cerr << "Failed to select Redis database: " << 
                    (reply ? reply->str : "Unknown error") << std::endl;
                freeReplyObject(reply);
                redisFree(conn);
                return nullptr;
            }
            freeReplyObject(reply);
        }

        // 创建智能指针，当连接不再使用时自动归还到连接池
        auto conn_ptr = std::shared_ptr<redisContext>(conn, [this](redisContext* conn) {
            std::unique_lock<std::mutex> lock(mutex_);
            if (!conn->err && redisPing(conn) != nullptr) {
                // 连接有效，归还到连接池
                connection_pool_.push(std::shared_ptr<redisContext>(conn, [](redisContext* conn) {}));
                idle_connections_++;
                cond_var_.notify_one();
            } else {
                // 连接无效，关闭连接
                redisFree(conn);
                current_connections_--;
            }
        });

        return conn_ptr;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create Redis connection: " << e.what() << std::endl;
        return nullptr;
    }
}

} // namespace giftcard
