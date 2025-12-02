#include "Config.h"
#include <fstream>
#include <iostream>

json Config::config_;

bool Config::load(const std::string& config_path) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        std::cerr << "Failed to open config file: " << config_path << std::endl;
        return false;
    }
    
    try {
        config_file >> config_;
        return true;
    } catch (const json::parse_error& e) {
        std::cerr << "Failed to parse config file: " << e.what() << std::endl;
        return false;
    }
}

int Config::getServerPort() {
    return config_.value("server.port", 8080);
}

int Config::getServerMaxConnections() {
    return config_.value("server.max_connections", 100);
}

int Config::getServerThreadPoolSize() {
    return config_.value("server.thread_pool_size", 10);
}

std::string Config::getDatabaseType() {
    return config_.value("database.type", "mysql");
}

std::string Config::getDatabaseHost() {
    return config_.value("database.host", "localhost");
}

int Config::getDatabasePort() {
    return config_.value("database.port", 3306);
}

std::string Config::getDatabaseName() {
    return config_.value("database.name", "library");
}

std::string Config::getDatabaseUsername() {
    return config_.value("database.username", "root");
}

std::string Config::getDatabasePassword() {
    return config_.value("database.password", "");
}

int Config::getDatabaseConnectionPoolSize() {
    return config_.value("database.connection_pool_size", 10);
}

std::string Config::getDatabaseCharset() {
    return config_.value("database.charset", "utf8mb4");
}

std::string Config::getJwtSecret() {
    return config_.value("jwt.secret", "library_jwt_secret_key");
}

int Config::getJwtExpiresIn() {
    return config_.value("jwt.expires_in", 86400);
}

int Config::getLibraryMaxBorrowBooks() {
    return config_.value("library.max_borrow_books", 5);
}

int Config::getLibraryBorrowPeriodDays() {
    return config_.value("library.borrow_period_days", 30);
}

int Config::getLibraryReservationExpireHours() {
    return config_.value("library.reservation_expire_hours", 24);
}

double Config::getLibraryOverdueFinePerDay() {
    return config_.value("library.overdue_fine_per_day", 0.5);
}

std::string Config::getLoggingLevel() {
    return config_.value("logging.level", "info");
}

std::string Config::getLoggingFilePath() {
    return config_.value("logging.file_path", "/var/log/library.log");
}

long Config::getLoggingMaxFileSize() {
    return config_.value("logging.max_file_size", 10485760L);
}

int Config::getLoggingMaxBackupFiles() {
    return config_.value("logging.max_backup_files", 5);
}