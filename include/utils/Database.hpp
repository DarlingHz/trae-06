#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>

namespace utils {

class Database {
public:
    Database(const std::string& db_path);
    ~Database();

    bool execute(const std::string& query);
    bool executeWithParams(const std::string& query, const std::vector<std::string>& params);
    std::vector<std::map<std::string, std::string>> fetch(const std::string& query);
    std::vector<std::map<std::string, std::string>> fetchWithParams(const std::string& query, const std::vector<std::string>& params);
    int getLastInsertRowId() const;
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    bool isOpen() const;


private:
    sqlite3* db_;
    bool is_open_;

    static int callback(void* data, int argc, char** argv, char** azColName);
};

} // namespace utils
