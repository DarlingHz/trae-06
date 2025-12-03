#include "SQLiteHelper.h"
#include <iostream>
#include <stdexcept>

SQLiteHelper::SQLiteHelper() : db(nullptr), isConnected(false) {
}

SQLiteHelper::~SQLiteHelper() {
    disconnect();
}

SQLiteHelper& SQLiteHelper::getInstance() {
    static SQLiteHelper instance;
    return instance;
}

bool SQLiteHelper::connect(const std::string& dbPath) {
    if (isConnected) {
        return true;
    }

    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        db = nullptr;
        return false;
    }

    isConnected = true;
    return true;
}

void SQLiteHelper::disconnect() {
    if (db != nullptr) {
        sqlite3_close(db);
        db = nullptr;
        isConnected = false;
    }
}

bool SQLiteHelper::isDBConnected() const {
    return isConnected;
}

sqlite3* SQLiteHelper::getDB() const {
    return db;
}

bool SQLiteHelper::createTables() {
    if (!isConnected) {
        return false;
    }

    std::string createTablesSQL = R"(
        -- Users table
        CREATE TABLE IF NOT EXISTS users (
            user_id INTEGER PRIMARY KEY AUTOINCREMENT,
            nickname TEXT NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        -- Stations table
        CREATE TABLE IF NOT EXISTS stations (
            station_id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            latitude REAL NOT NULL,
            longitude REAL NOT NULL,
            capacity INTEGER NOT NULL,
            available_bikes INTEGER NOT NULL DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        -- Bikes table
        CREATE TABLE IF NOT EXISTS bikes (
            bike_id INTEGER PRIMARY KEY AUTOINCREMENT,
            current_station_id INTEGER,
            status TEXT NOT NULL DEFAULT 'normal',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (current_station_id) REFERENCES stations(station_id)
        );

        -- Rentals table
        CREATE TABLE IF NOT EXISTS rentals (
            rental_id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            bike_id INTEGER NOT NULL,
            start_station_id INTEGER NOT NULL,
            end_station_id INTEGER,
            start_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            end_time DATETIME,
            fee REAL DEFAULT 0.0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(user_id),
            FOREIGN KEY (bike_id) REFERENCES bikes(bike_id),
            FOREIGN KEY (start_station_id) REFERENCES stations(station_id),
            FOREIGN KEY (end_station_id) REFERENCES stations(station_id)
        );

        -- Add indexes for better performance
        CREATE INDEX IF NOT EXISTS idx_rentals_user_id ON rentals(user_id);
        CREATE INDEX IF NOT EXISTS idx_rentals_start_time ON rentals(start_time);
        CREATE INDEX IF NOT EXISTS idx_rentals_end_time ON rentals(end_time);
        CREATE INDEX IF NOT EXISTS idx_bikes_station_id ON bikes(current_station_id);
        CREATE INDEX IF NOT EXISTS idx_bikes_status ON bikes(status);
    );

    char* errMsg;
    int rc = sqlite3_exec(db, createTablesSQL.c_str(), nullptr, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

bool SQLiteHelper::executeQuery(const std::string& query) {
    if (!isConnected) {
        return false;
    }

    char* errMsg;
    int rc = sqlite3_exec(db, query.c_str(), nullptr, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}