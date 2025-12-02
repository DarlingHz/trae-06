#include "DatabaseConnectionPool.h"
#include "Logger.h"
#include <stdexcept>

// 初始化静态成员变量
std::queue<std::shared_ptr<Session>> DatabaseConnectionPool::connection_queue_;
std::mutex DatabaseConnectionPool::mutex_;
std::condition_variable DatabaseConnectionPool::condition_;
bool DatabaseConnectionPool::initialized_ = false;
int DatabaseConnectionPool::pool_size_ = 0;
std::string DatabaseConnectionPool::host_;
int DatabaseConnectionPool::port_ = 0;
std::string DatabaseConnectionPool::user_;
std::string DatabaseConnectionPool::password_;
std::string DatabaseConnectionPool::database_;
std::string DatabaseConnectionPool::charset_;
int DatabaseConnectionPool::used_connections_ = 0;

bool DatabaseConnectionPool::init(int pool_size, const std::string& host, int port,
                                     const std::string& user, const std::string& password,
                                     const std::string& database, const std::string& charset) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        Logger::warn("Database connection pool is already initialized");
        return true;
    }
    
    try {
        // 设置成员变量
        pool_size_ = pool_size;
        host_ = host;
        port_ = port;
        user_ = user;
        password_ = password;
        database_ = database;
        charset_ = charset;
        used_connections_ = 0;
        
        // 创建数据库连接并添加到连接队列中
        for (int i = 0; i < pool_size_; ++i) {
            std::shared_ptr<Session> connection = createConnection();
            if (connection) {
                connection_queue_.push(connection);
            } else {
                Logger::error("Failed to create database connection, index: " + std::to_string(i));
                // 清空已创建的连接
                while (!connection_queue_.empty()) {
                    connection_queue_.pop();
                }
                return false;
            }
        }
        
        initialized_ = true;
        Logger::info("Database connection pool initialized successfully, pool size: " + std::to_string(pool_size_));
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to initialize database connection pool: " + std::string(e.what()));
        // 清空已创建的连接
        while (!connection_queue_.empty()) {
            connection_queue_.pop();
        }
        return false;
    }
}

std::shared_ptr<Session> DatabaseConnectionPool::getConnection(int timeout_ms) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        Logger::error("Database connection pool is not initialized");
        return nullptr;
    }
    
    // 等待连接可用，直到超时
    if (!condition_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                               []() { return !connection_queue_.empty(); })) {
        Logger::warn("Failed to get database connection, timeout after " + std::to_string(timeout_ms) + "ms");
        return nullptr;
    }
    
    // 从连接队列中取出一个连接
    std::shared_ptr<Session> connection = connection_queue_.front();
    connection_queue_.pop();
    used_connections_++;
    
    // 检查连接是否有效
    try {
        // 尝试执行一个简单的查询来检查连接是否有效
        connection->sql("SELECT 1").execute();
    } catch (const std::exception& e) {
        Logger::error("Database connection is invalid, creating a new connection: " + std::string(e.what()));
        connection = createConnection();
        if (!connection) {
            Logger::error("Failed to create new database connection");
            used_connections_--;
            condition_.notify_one();
            return nullptr;
        }
    }
    
    Logger::debug("Get database connection successfully, used connections: " + std::to_string(used_connections_) + ", available connections: " + std::to_string(connection_queue_.size()));
    return connection;
}

void DatabaseConnectionPool::releaseConnection(std::shared_ptr<Session> connection) {
    if (!connection) {
        Logger::warn("Trying to release a null database connection");
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        Logger::warn("Database connection pool is not initialized");
        return;
    }
    
    // 检查连接是否有效
    try {
        // 尝试执行一个简单的查询来检查连接是否有效
        connection->sql("SELECT 1").execute();
    } catch (const std::exception& e) {
        Logger::error("Database connection is invalid, creating a new connection to replace it: " + std::string(e.what()));
        std::shared_ptr<Session> new_connection = createConnection();
        if (new_connection) {
            connection_queue_.push(new_connection);
        } else {
            Logger::error("Failed to create new database connection to replace the invalid one");
        }
        used_connections_--;
        condition_.notify_one();
        return;
    }
    
    // 将连接放回连接队列中
    connection_queue_.push(connection);
    used_connections_--;
    condition_.notify_one();
    
    Logger::debug("Release database connection successfully, used connections: " + std::to_string(used_connections_) + ", available connections: " + std::to_string(connection_queue_.size()));
}

void DatabaseConnectionPool::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        Logger::warn("Database connection pool is not initialized");
        return;
    }
    
    // 清空连接队列
    while (!connection_queue_.empty()) {
        connection_queue_.pop();
    }
    
    // 重置成员变量
    initialized_ = false;
    pool_size_ = 0;
    host_.clear();
    port_ = 0;
    user_.clear();
    password_.clear();
    database_.clear();
    charset_.clear();
    used_connections_ = 0;
    
    Logger::info("Database connection pool closed successfully");
}

std::string DatabaseConnectionPool::getDatabaseName() {
    return database_;
}

int DatabaseConnectionPool::getPoolSize() {
    std::lock_guard<std::mutex> lock(mutex_);
    return pool_size_;
}

int DatabaseConnectionPool::getAvailableConnections() {
    std::lock_guard<std::mutex> lock(mutex_);
    return connection_queue_.size();
}

int DatabaseConnectionPool::getUsedConnections() {
    std::lock_guard<std::mutex> lock(mutex_);
    return used_connections_;
}

std::shared_ptr<Session> DatabaseConnectionPool::createConnection() {
    try {
        // 创建数据库连接
        Session* session = new Session(
            SessionOption::HOST, host_,
            SessionOption::PORT, port_,
            SessionOption::USER, user_,
            SessionOption::PWD, password_,
            SessionOption::DB, database_
        );
        
        // 设置字符集
        if (!charset_.empty()) {
            session->sql("SET NAMES " + charset_).execute();
        }
        
        Logger::debug("Database connection created successfully");
        return std::shared_ptr<Session>(session);
    } catch (const std::exception& e) {
        Logger::error("Failed to create database connection: " + std::string(e.what()));
        return nullptr;
    }
}