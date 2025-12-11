#include "utils/Database.hpp"
#include <iostream>
#include <stdexcept>

namespace utils {

Database::Database(const std::string& db_path) : db_(nullptr), is_open_(false) {
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_close(db_);
        throw std::runtime_error("Failed to open database");
    }
    is_open_ = true;
    std::cout << "Opened database successfully" << std::endl;
}

Database::~Database() {
    if (is_open_) {
        sqlite3_close(db_);
        is_open_ = false;
        std::cout << "Closed database successfully" << std::endl;
    }
}

bool Database::execute(const std::string& query) {
    return executeWithParams(query, {});
}

bool Database::executeWithParams(const std::string& query, const std::vector<std::string>& params) {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db_) << std::endl;
        std::cerr << "Query: " << query << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    for (size_t i = 0; i < params.size(); ++i) {
        rc = sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            std::cerr << "Parameter binding error: " << sqlite3_errmsg(db_) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Execution error: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

std::vector<std::map<std::string, std::string>> Database::fetch(const std::string& query) {
    return fetchWithParams(query, {});
}

std::vector<std::map<std::string, std::string>> Database::fetchWithParams(const std::string& query, const std::vector<std::string>& params) {
    std::vector<std::map<std::string, std::string>> result;

    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return result;
    }

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db_) << std::endl;
        std::cerr << "Query: " << query << std::endl;
        sqlite3_finalize(stmt);
        return result;
    }

    for (size_t i = 0; i < params.size(); ++i) {
        rc = sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            std::cerr << "Parameter binding error: " << sqlite3_errmsg(db_) << std::endl;
            sqlite3_finalize(stmt);
            return result;
        }
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::map<std::string, std::string> row;
        int column_count = sqlite3_column_count(stmt);

        for (int i = 0; i < column_count; ++i) {
            const char* column_name = sqlite3_column_name(stmt, i);
            const char* column_value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));

            if (column_name && column_value) {
                row[column_name] = column_value;
            } else if (column_name) {
                row[column_name] = "";
            }
        }

        result.push_back(row);
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Fetch error: " << sqlite3_errmsg(db_) << std::endl;
    }

    sqlite3_finalize(stmt);
    return result;
}

int Database::getLastInsertRowId() const {
    if (!is_open_) {
        std::cerr << "Database is not open" << std::endl;
        return -1;
    }

    return sqlite3_last_insert_rowid(db_);
}

bool Database::beginTransaction() {
    return execute("BEGIN TRANSACTION;");
}

bool Database::commitTransaction() {
    return execute("COMMIT;");
}

bool Database::rollbackTransaction() {
    return execute("ROLLBACK;");
}

bool Database::isOpen() const {
    return is_open_;
}

int Database::callback(void* data, int argc, char** argv, char** azColName) {
    auto* result = static_cast<std::vector<std::map<std::string, std::string>>*>(data);
    std::map<std::string, std::string> row;

    for (int i = 0; i < argc; ++i) {
        row[azColName[i]] = argv[i] ? argv[i] : "";
    }

    result->push_back(row);
    return 0;
}

} // namespace utils
