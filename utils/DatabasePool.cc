#include "DatabasePool.h"
#include <iostream>
#include <stdexcept>

namespace giftcard {

bool DatabasePool::init(const std::string& host, int port, const std::string& user, 
                          const std::string& password, const std::string& dbname, 
                          int max_connections, const std::string& charset) {
    try {
        host_ = host;
        port_ = port;
        user_ = user;
        password_ = password;
        dbname_ = dbname;
        charset_ = charset;
        max_connections_ = max_connections;

        // 初始化MySQL库
        mysql_library_init(0, nullptr, nullptr);

        // 创建初始连接
        for (int i = 0; i < max_connections_ / 2; ++i) {
            auto conn = createConnection();
            if (conn) {
                connection_pool_.push(conn);
                idle_connections_++;
            } else {
                std::cerr << "Failed to create initial database connection" << std::endl;
                return false;
            }
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize database pool: " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<MYSQL> DatabasePool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);

    // 等待可用连接
    while (connection_pool_.empty() && current_connections_ >= max_connections_) {
        cond_var_.wait(lock);
    }

    std::shared_ptr<MYSQL> conn;

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
        if (mysql_ping(conn.get()) != 0) {
            std::cerr << "Database connection is invalid, reconnecting..." << std::endl;
            mysql_close(conn.get());
            conn = createConnection();
        }
    }

    return conn;
}

void DatabasePool::close() {
    std::unique_lock<std::mutex> lock(mutex_);

    // 关闭所有连接
    while (!connection_pool_.empty()) {
        auto conn = connection_pool_.front();
        connection_pool_.pop();
        mysql_close(conn.get());
        idle_connections_--;
        current_connections_--;
    }

    // 清理MySQL库
    mysql_library_end();
}

void DatabasePool::getStatus(int& current_connections, int& idle_connections) {
    std::unique_lock<std::mutex> lock(mutex_);
    current_connections = current_connections_;
    idle_connections = idle_connections_;
}

std::shared_ptr<MYSQL> DatabasePool::createConnection() {
    try {
        // 创建MySQL连接
        MYSQL* conn = mysql_init(nullptr);
        if (!conn) {
            std::cerr << "Failed to initialize MySQL connection: " << mysql_error(conn) << std::endl;
            return nullptr;
        }

        // 设置连接超时
        mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, "5");
        mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, "30");
        mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, "30");

        // 连接到数据库
        if (!mysql_real_connect(conn, host_.c_str(), user_.c_str(), password_.c_str(), 
                                 dbname_.c_str(), port_, nullptr, 0)) {
            std::cerr << "Failed to connect to database: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            return nullptr;
        }

        // 设置字符集
        if (mysql_set_character_set(conn, charset_.c_str()) != 0) {
            std::cerr << "Failed to set database character set: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            return nullptr;
        }

        // 设置自动提交
        if (mysql_autocommit(conn, 1) != 0) {
            std::cerr << "Failed to set autocommit: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            return nullptr;
        }

        // 创建智能指针，当连接不再使用时自动归还到连接池
        auto conn_ptr = std::shared_ptr<MYSQL>(conn, [this](MYSQL* conn) {
            std::unique_lock<std::mutex> lock(mutex_);
            if (mysql_ping(conn) == 0) {
                // 连接有效，归还到连接池
                connection_pool_.push(std::shared_ptr<MYSQL>(conn, [](MYSQL* conn) {}));
                idle_connections_++;
                cond_var_.notify_one();
            } else {
                // 连接无效，关闭连接
                mysql_close(conn);
                current_connections_--;
            }
        });

        return conn_ptr;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create database connection: " << e.what() << std::endl;
        return nullptr;
    }
}

} // namespace giftcard
