#include "Config.hpp"
#include <fstream>
#include <iostream>

// 静态成员初始化
Config::Server Config::s_server;
Config::Database Config::s_database;
Config::Cache Config::s_cache;
bool Config::s_loaded = false;

bool Config::load(const std::string& filename) {
  try {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Failed to open config file: " << filename << std::endl;
      return false;
    }

    json j;
    file >> j;

    // 读取服务器配置
    if (j.contains("server")) {
      const auto& server = j["server"];
      if (server.contains("port")) {
        s_server.port = server["port"];
      }
      if (server.contains("threads")) {
        s_server.threads = server["threads"];
      }
    }

    // 读取数据库配置
    if (j.contains("database")) {
      const auto& database = j["database"];
      if (database.contains("path")) {
        s_database.path = database["path"];
      }
    }

    // 读取缓存配置
    if (j.contains("cache")) {
      const auto& cache = j["cache"];
      if (cache.contains("session_ttl")) {
        s_cache.session_ttl = cache["session_ttl"];
      }
    }

    s_loaded = true;
    std::cout << "Config loaded successfully from " << filename << std::endl;
    return true;

  } catch (const std::exception& e) {
    std::cerr << "Failed to load config: " << e.what() << std::endl;
    return false;
  }
}

const Config::Server& Config::getServer() {
  return s_server;
}

const Config::Database& Config::getDatabase() {
  return s_database;
}

const Config::Cache& Config::getCache() {
  return s_cache;
}
