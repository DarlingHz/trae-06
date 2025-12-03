#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace config {

struct DatabaseConfig {
    std::string path;
    int max_connections = 10;
};

struct HttpConfig {
    int port = 8080;
    int max_threads = 4;
};

struct JWTConfig {
    std::string secret_key = "secret_key_change_this_in_production";
    int expires_in = 3600; // 1 hour
};

struct CacheConfig {
    int capacity = 1000;
    int ttl = 300; // 5 minutes
};

struct Config {
    DatabaseConfig database;
    HttpConfig http;
    JWTConfig jwt;
    CacheConfig cache;
    bool debug = false;
};

Config load_config(const std::string& config_file);

} // namespace config
