#include "database.h"
#include <stdexcept>
#include <iostream>

bool Database::connect(const std::string& db_path) {
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string error = sqlite3_errmsg(db_);
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }
    return true;
}

void Database::disconnect() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

sqlite3* Database::get_db() {
    return db_;
}

bool Database::execute_update(const std::string& sql, int* last_insert_id) {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    
    if (rc != SQLITE_OK) {
        std::string error = err_msg;
        sqlite3_free(err_msg);
        return false;
    }
    
    if (last_insert_id != nullptr) {
        *last_insert_id = sqlite3_last_insert_rowid(db_);
    }
    
    return true;
}

bool Database::transaction_start() {
    return execute_update("BEGIN TRANSACTION;");
}

bool Database::transaction_commit() {
    return execute_update("COMMIT;");
}

bool Database::transaction_rollback() {
    return execute_update("ROLLBACK;");
}

bool Database::execute_query(const std::string& sql, 
                           std::function<int(sqlite3_stmt*)> callback, 
                           void* data) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::string error = sqlite3_errmsg(db_);
        return false;
    }
    
    if (callback == nullptr) {
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE;
    }
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (callback(stmt) != 0) {
            break;
        }
    }
    
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}