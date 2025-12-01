#include "db/database.h"
#include <iostream>

namespace db {

    Database::~Database() {
        close();
    }

    bool Database::open(const std::string& db_path) {
        if (db_ != nullptr) {
            close();
        }

        int rc = sqlite3_open(db_path.c_str(), &db_);
        if (rc != SQLITE_OK) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db_) << std::endl;
            sqlite3_close(db_);
            db_ = nullptr;
            return false;
        }

        return true;
    }

    void Database::close() {
        if (db_ != nullptr) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

    bool Database::execute(const std::string& sql) {
        if (db_ == nullptr) {
            std::cerr << "Database not open" << std::endl;
            return false;
        }

        char* err_msg = nullptr;
        int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);

        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << err_msg << std::endl;
            sqlite3_free(err_msg);
            return false;
        }

        return true;
    }

    bool Database::query(const std::string& sql, std::vector<std::map<std::string, std::string>>& results) {
        if (db_ == nullptr) {
            std::cerr << "Database not open" << std::endl;
            return false;
        }

        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << sqlite3_errmsg(db_) << std::endl;
            return false;
        }

        results.clear();

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            std::map<std::string, std::string> row;
            int column_count = sqlite3_column_count(stmt);

            for (int i = 0; i < column_count; ++i) {
                const char* column_name = sqlite3_column_name(stmt, i);
                const char* column_value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));

                if (column_value != nullptr) {
                    row[column_name] = column_value;
                } else {
                    row[column_name] = "";
                }
            }

            results.push_back(row);
        }

        if (rc != SQLITE_DONE) {
            std::cerr << "SQL error: " << sqlite3_errmsg(db_) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        sqlite3_finalize(stmt);
        return true;
    }

} // namespace db
