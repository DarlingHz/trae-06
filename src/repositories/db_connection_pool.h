#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <sqlite3.h>
#include <stdexcept>

namespace repositories {

class DBConnectionPool {
public:
    static DBConnectionPool& instance();

    void initialize(const std::string& db_path, int pool_size = 5);
    
    class Connection {
    public:
        Connection(DBConnectionPool& pool, sqlite3* db)
            : pool_(pool), db_(db) {}
        
        ~Connection() {
            if (db_) {
                pool_.release_connection(db_);
            }
        }
        
        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;
        
        Connection(Connection&& other) noexcept
            : pool_(other.pool_), db_(other.db_) {
            other.db_ = nullptr;
        }
        
        Connection& operator=(Connection&& other) noexcept {
            if (this != &other) {
                if (db_) {
                    pool_.release_connection(db_);
                }
                db_ = other.db_;
                other.db_ = nullptr;
            }
            return *this;
        }
        
        sqlite3* get() const { return db_; }
        operator sqlite3*() const { return db_; }
        
    private:
        DBConnectionPool& pool_;
        sqlite3* db_ = nullptr;
    };
    
    Connection acquire_connection();
    
private:
    DBConnectionPool() = default;
    ~DBConnectionPool();
    
    DBConnectionPool(const DBConnectionPool&) = delete;
    DBConnectionPool& operator=(const DBConnectionPool&) = delete;
    
    sqlite3* create_connection();
    void release_connection(sqlite3* db);
    
    std::string db_path_;
    int pool_size_ = 0;
    std::queue<sqlite3*> connections_;
    std::mutex mutex_;
    std::condition_variable cond_var_;
    bool initialized_ = false;
};

} // namespace repositories
