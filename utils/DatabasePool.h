#ifndef DATABASE_POOL_H
#define DATABASE_POOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace giftcard {

class DatabasePool {
public:
    // 单例模式
    static DatabasePool& getInstance() {
        static DatabasePool instance;
        return instance;
    }

    // 初始化数据库连接池
    bool init(const std::string& host, int port, const std::string& user, 
              const std::string& password, const std::string& dbname, 
              int max_connections, const std::string& charset = "utf8mb4");

    // 获取数据库连接
    std::shared_ptr<MYSQL> getConnection();

    // 关闭数据库连接池
    void close();

    // 获取连接池状态
    void getStatus(int& current_connections, int& idle_connections);

private:
    DatabasePool() = default;
    ~DatabasePool() = default;
    DatabasePool(const DatabasePool&) = delete;
    DatabasePool& operator=(const DatabasePool&) = delete;

    // 创建数据库连接
    std::shared_ptr<MYSQL> createConnection();

    // 数据库连接池配置
    std::string host_;
    int port_ = 3306;
    std::string user_;
    std::string password_;
    std::string dbname_;
    std::string charset_ = "utf8mb4";
    int max_connections_ = 20;

    // 数据库连接池
    std::queue<std::shared_ptr<MYSQL>> connection_pool_;

    // 线程同步机制
    std::mutex mutex_;
    std::condition_variable cond_var_;

    // 连接池状态
    int current_connections_ = 0;
    int idle_connections_ = 0;
};

} // namespace giftcard

#endif // DATABASE_POOL_H
