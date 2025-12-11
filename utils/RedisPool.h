#ifndef REDIS_POOL_H
#define REDIS_POOL_H

#include <hiredis/hiredis.h>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace giftcard {

class RedisPool {
public:
    // 单例模式
    static RedisPool& getInstance() {
        static RedisPool instance;
        return instance;
    }

    // 初始化Redis连接池
    bool init(const std::string& host, int port, const std::string& password, 
              int db, int max_connections, int timeout = 5000);

    // 获取Redis连接
    std::shared_ptr<redisContext> getConnection();

    // 关闭Redis连接池
    void close();

    // 获取连接池状态
    void getStatus(int& current_connections, int& idle_connections);

private:
    RedisPool() = default;
    ~RedisPool() = default;
    RedisPool(const RedisPool&) = delete;
    RedisPool& operator=(const RedisPool&) = delete;

    // 创建Redis连接
    std::shared_ptr<redisContext> createConnection();

    // Redis连接池配置
    std::string host_;
    int port_ = 6379;
    std::string password_;
    int db_ = 0;
    int max_connections_ = 10;
    int timeout_ = 5000;

    // Redis连接池
    std::queue<std::shared_ptr<redisContext>> connection_pool_;

    // 线程同步机制
    std::mutex mutex_;
    std::condition_variable cond_var_;

    // 连接池状态
    int current_connections_ = 0;
    int idle_connections_ = 0;
};

} // namespace giftcard

#endif // REDIS_POOL_H
