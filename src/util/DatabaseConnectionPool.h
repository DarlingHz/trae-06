#ifndef DATABASE_CONNECTION_POOL_H
#define DATABASE_CONNECTION_POOL_H

// 避免cpprest和mysqlx之间的宏定义冲突
#undef U

#include <mysqlx/xdevapi.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

using namespace mysqlx;

class DatabaseConnectionPool {
public:
    // 初始化数据库连接池
    static bool init(int pool_size, const std::string& host, int port,
                     const std::string& user, const std::string& password,
                     const std::string& database, const std::string& charset = "utf8mb4");
    
    // 获取数据库连接
    static std::shared_ptr<Session> getConnection(int timeout_ms = 5000);
    
    // 释放数据库连接
    static void releaseConnection(std::shared_ptr<Session> connection);
    
    // 关闭数据库连接池
    static void close();
    
    // 获取连接池状态
    static int getPoolSize();
    static int getAvailableConnections();
    static int getUsedConnections();
    
    // 获取数据库名称
    static std::string getDatabaseName();
    
private:
    // 私有构造函数，防止实例化
    DatabaseConnectionPool() = delete;
    DatabaseConnectionPool(const DatabaseConnectionPool&) = delete;
    DatabaseConnectionPool& operator=(const DatabaseConnectionPool&) = delete;
    
    // 创建数据库连接
    static std::shared_ptr<Session> createConnection();
    
    // 成员变量
    static std::queue<std::shared_ptr<Session>> connection_queue_;
    static std::mutex mutex_;
    static std::condition_variable condition_;
    static bool initialized_;
    static int pool_size_;
    static std::string host_;
    static int port_;
    static std::string user_;
    static std::string password_;
    static std::string database_;
    static std::string charset_;
    static int used_connections_;
};

#endif // DATABASE_CONNECTION_POOL_H