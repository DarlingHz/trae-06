#include "config/Config.h"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

namespace config {

DatabaseConfig parse_database_config(const json& data) {
    DatabaseConfig config;
    
    if(data.contains("path")) {
        config.path = data["path"];
    } else {
        config.path = "./bookmarks.db";
    }
    
    if(data.contains("max_connections")) {
        config.max_connections = data["max_connections"];
    }
    
    return config;
}

HttpConfig parse_http_config(const json& data) {
    HttpConfig config;
    
    if(data.contains("port")) {
        config.port = data["port"];
    }
    
    if(data.contains("max_threads")) {
        config.max_threads = data["max_threads"];
    }
    
    return config;
}

JWTConfig parse_jwt_config(const json& data) {
    JWTConfig config;
    
    if(data.contains("secret_key")) {
        config.secret_key = data["secret_key"];
    }
    
    if(data.contains("expires_in")) {
        config.expires_in = data["expires_in"];
    }
    
    return config;
}

CacheConfig parse_cache_config(const json& data) {
    CacheConfig config;
    
    if(data.contains("capacity")) {
        config.capacity = data["capacity"];
    }
    
    if(data.contains("ttl")) {
        config.ttl = data["ttl"];
    }
    
    return config;
}

Config load_config(const std::string& config_file) {
    Config config;
    
    try {
        std::ifstream file(config_file);
        if(!file.is_open()) {
            // Return default config if file doesn't exist
            return config;
        }
        
        json data;
        file >> data;
        
        if(data.contains("database")) {
            config.database = parse_database_config(data["database"]);
        }
        
        if(data.contains("http")) {
            config.http = parse_http_config(data["http"]);
        }
        
        if(data.contains("jwt")) {
            config.jwt = parse_jwt_config(data["jwt"]);
        }
        
        if(data.contains("cache")) {
            config.cache = parse_cache_config(data["cache"]);
        }
        
        if(data.contains("debug")) {
            config.debug = data["debug"];
        }
        
        return config;
    } catch(const std::exception& e) {
        throw std::runtime_error("Failed to load config: " + std::string(e.what()));
    }
}

} // namespace config
