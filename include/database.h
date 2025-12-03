#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include "json_utils.h"



class Database {
private:
    sqlite3* db_ = nullptr;
    Database() = default;
    
public:
    static Database& instance() {
        static Database instance;
        return instance;
    }
    
    bool connect(const std::string& db_path);
    void disconnect();
    
    sqlite3* get_db();
    
    bool execute_query(const std::string& sql, 
                      std::function<int(sqlite3_stmt*)> callback = nullptr,
                      void* data = nullptr);
    
    bool execute_update(const std::string& sql, int* last_insert_id = nullptr);
    
    bool transaction_start();
    bool transaction_commit();
    bool transaction_rollback();
};

#endif // DATABASE_H