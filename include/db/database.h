#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>

namespace db {

    class Database {
    public:
        Database() = default;
        ~Database();

        bool open(const std::string& db_path);
        void close();

        bool execute(const std::string& sql);
        bool query(const std::string& sql, std::vector<std::map<std::string, std::string>>& results);

    private:
        sqlite3* db_ = nullptr;
    };

} // namespace db

#endif // DATABASE_H
