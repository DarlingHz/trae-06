#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <stdexcept>
#include "db_connection.h"

class DBPool {
public:
    static DBPool& getInstance() {
        static DBPool instance;
        return instance;
    }

    void init(const std::string& host, int port, const std::string& user,
              const std::string& password, const std::string& dbName, size_t poolSize) {
        std::lock_guard<std::mutex> lock(mutex);
        if (!connections.empty()) {
            throw std::runtime_error("DBPool already initialized");
        }

        this->host = host;
        this->port = port;
        this->user = user;
        this->password = password;
        this->dbName = dbName;
        this->maxPoolSize = poolSize;

        for (size_t i = 0; i < poolSize; ++i) {
            auto conn = std::make_shared<DBConnection>();
            conn->connect(host, port, user, password, dbName);
            connections.push_back(conn);
        }
    }

    std::shared<DBConnection> getConnection() {
        std::unique_lock<std::mutex> lock(mutex);
        while (connections.empty()) {
            if (connections.size() < maxPoolSize) {
                try {
                    auto conn = std::make_shared<DBConnection>();
                    conn->connect(host, port, user, password, dbName);
                    return conn;
                } catch (const std::exception& e) {
                    throw std::runtime_error("Failed to create new DB connection: " + std::string(e.what()));
                }
            }
            cond.wait(lock);
        }

        auto conn = connections.back();
        connections.pop_back();
        return conn;
    }

    void releaseConnection(std::shared_ptr<DBConnection> conn) {
        std::lock_guard<std::mutex> lock(mutex);
        connections.push_back(conn);
        cond.notify_one();
    }

private:
    DBPool() = default;
    DBPool(const DBPool&) = delete;
    DBPool& operator=(const DBPool&) = delete;

    std::vector<std::shared_ptr<DBConnection>> connections;
    std::mutex mutex;
    std::condition_variable cond;

    std::string host;
    int port;
    std::string user;
    std::string password;
    std::string dbName;
    size_t maxPoolSize = 10;
};
