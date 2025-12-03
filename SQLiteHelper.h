#pragma once

#include <string>
#include <sqlite3.h>

class SQLiteHelper {
private:
    sqlite3* db;
    bool isConnected;

    SQLiteHelper() : db(nullptr), isConnected(false) {}
    ~SQLiteHelper();

public:
    SQLiteHelper(const SQLiteHelper&) = delete;
    SQLiteHelper& operator=(const SQLiteHelper&) = delete;
    SQLiteHelper(SQLiteHelper&&) = delete;
    SQLiteHelper& operator=(SQLiteHelper&&) = delete;

    static SQLiteHelper& getInstance();

    bool connect(const std::string& dbPath);
    void disconnect();
    bool isDBConnected() const;
    sqlite3* getDB() const;

    bool createTables();
    bool executeQuery(const std::string& query);
};