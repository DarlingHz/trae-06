#pragma once

#include <mysql/mysql.h>
#include <string>
#include <stdexcept>

class DBConnection {
public:
    DBConnection() : conn(nullptr) {
        conn = mysql_init(nullptr);
        if (!conn) {
            throw std::runtime_error("Failed to initialize MySQL connection");
        }
    }

    ~DBConnection() {
        if (conn) {
            mysql_close(conn);
        }
    }

    DBConnection(const DBConnection&) = delete;
    DBConnection& operator=(const DBConnection&) = delete;

    void connect(const std::string& host, int port, const std::string& user,
                 const std::string& password, const std::string& dbName) {
        if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(),
                               dbName.c_str(), port, nullptr, 0)) {
            throw std::runtime_error("MySQL connection failed: " + std::string(mysql_error(conn)));
        }
    }

    MYSQL* getConn() const {
        return conn;
    }

    void executeQuery(const std::string& query) {
        if (mysql_query(conn, query.c_str()) != 0) {
            throw std::runtime_error("MySQL query failed: " + std::string(mysql_error(conn)));
        }
    }

private:
    MYSQL* conn;
};
