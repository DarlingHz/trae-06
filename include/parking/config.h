#pragma once
#include <string>
#include <stdexcept>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Config {
public:
    static Config& instance() {
        static Config instance;
        return instance;
    }

    void load(const std::string& filename = "config.json") {
        std::ifstream file(filename);
        if (file.is_open()) {
            json data;
            file >> data;
            if (data.contains("port") && data["port"].is_number()) {
                port_ = data["port"].get<int>();
            }
            if (data.contains("db_path") && data["db_path"].is_string()) {
                db_path_ = data["db_path"].get<std::string>();
            }
            if (data.contains("token_expiration_hours") && data["token_expiration_hours"].is_number()) {
                token_expiration_hours_ = data["token_expiration_hours"].get<int>();
            }
            if (data.contains("min_reservation_duration_hours") && data["min_reservation_duration_hours"].is_number()) {
                min_reservation_duration_hours_ = data["min_reservation_duration_hours"].get<int>();
            }
            if (data.contains("max_reservation_duration_hours") && data["max_reservation_duration_hours"].is_number()) {
                max_reservation_duration_hours_ = data["max_reservation_duration_hours"].get<int>();
            }
        }
    }

    int port() const { return port_; }
    const std::string& db_path() const { return db_path_; }
    int token_expiration_hours() const { return token_expiration_hours_; }
    int token_expiration() const { return token_expiration_hours_ * 3600; } // Convert to seconds
    int min_reservation_duration_hours() const { return min_reservation_duration_hours_; }
    int max_reservation_duration_hours() const { return max_reservation_duration_hours_; }

private:
    Config() : port_(8080), db_path_("parking.db"),
               token_expiration_hours_(24),
               min_reservation_duration_hours_(1),
               max_reservation_duration_hours_(24) {}
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    int port_;
    std::string db_path_;
    int token_expiration_hours_;
    int min_reservation_duration_hours_;
    int max_reservation_duration_hours_;
};

#define CONFIG Config::instance()
