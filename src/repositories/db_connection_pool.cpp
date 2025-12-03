#include "repositories/db_connection_pool.h"
#include <stdexcept>
#include <sqlite3.h>

namespace repositories {

DBConnectionPool& DBConnectionPool::instance() {
    static DBConnectionPool instance;
    return instance;
}

void DBConnectionPool::initialize(const std::string& db_path, int pool_size) {
    if (initialized_) {
        throw std::runtime_error("Connection pool already initialized");
    }
    
    db_path_ = db_path;
    pool_size_ = pool_size;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (int i = 0; i < pool_size; ++i) {
        sqlite3* db = create_connection();
        connections_.push(db);
    }
    
    initialized_ = true;
}

DBConnectionPool::~DBConnectionPool() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    while (!connections_.empty()) {
        sqlite3* db = connections_.front();
        connections_.pop();
        sqlite3_close(db);
    }
}

DBConnectionPool::Connection DBConnectionPool::acquire_connection() {
    if (!initialized_) {
        throw std::runtime_error("Connection pool not initialized");
    }
    
    std::unique_lock<std::mutex> lock(mutex_);
    
    cond_var_.wait(lock, [this] { return !connections_.empty(); });
    
    sqlite3* db = connections_.front();
    connections_.pop();
    
    return Connection(*this, db);
}

sqlite3* DBConnectionPool::create_connection() {
    sqlite3* db;
    int rc = sqlite3_open(db_path_.c_str(), &db);
    
    if (rc != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to open database: ") + sqlite3_errmsg(db));
    }
    
    return db;
}

void DBConnectionPool::release_connection(sqlite3* db) {
    if (!db) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    connections_.push(db);
    cond_var_.notify_one();
}

} // namespace repositories
