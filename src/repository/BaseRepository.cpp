#include "repository/BaseRepository.h"
#include <sqlite3.h>
#include <string>
#include <stdexcept>

namespace repository {

BaseRepository::BaseRepository(const std::string& db_path) {
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string error_msg = "Can't open database: " + std::string(sqlite3_errmsg(db_));
        sqlite3_close(db_);
        db_ = nullptr;
        throw DatabaseException(error_msg);
    }
}

BaseRepository::~BaseRepository() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

BaseRepository::BaseRepository(BaseRepository&& other) noexcept {
    db_ = other.db_;
    other.db_ = nullptr;
}

BaseRepository& BaseRepository::operator=(BaseRepository&& other) noexcept {
    if (this != &other) {
        if (db_ != nullptr) {
            sqlite3_close(db_);
        }
        db_ = other.db_;
        other.db_ = nullptr;
    }
    return *this;
}

void BaseRepository::execute(const std::string& sql) {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        std::string error_msg = "SQL error: " + std::string(err_msg);
        sqlite3_free(err_msg);
        throw DatabaseException(error_msg);
    }
}

} // namespace repository