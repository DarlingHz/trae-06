#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <sqlite3.h>

namespace repository {

class DatabasePool {
private:
    std::string db_path_;
    size_t max_connections_;
    std::vector<sqlite3*> connections_;
    std::mutex mutex_;
    std::condition_variable cv_;

    sqlite3* create_connection();
    void destroy_connection(sqlite3* conn);

public:
    DatabasePool(const std::string& db_path, size_t max_connections = 10);
    ~DatabasePool();

    std::shared_ptr<sqlite3> get_connection();
    void initialize_tables();
};

} // namespace repository
