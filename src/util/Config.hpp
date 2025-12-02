#ifndef Config_hpp
#define Config_hpp

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class Config {
public:
  // 服务器配置
  struct Server {
    int port = 8080;
    int threads = 4;
  };

  // 数据库配置
  struct Database {
    std::string path = "./gym_booking.db";
  };

  // 缓存配置
  struct Cache {
    int session_ttl = 30; // 秒
  };

  // 加载配置文件
  static bool load(const std::string& filename);

  // 获取服务器配置
  static const Server& getServer();

  // 获取数据库配置
  static const Database& getDatabase();

  // 获取缓存配置
  static const Cache& getCache();

private:
  static Server s_server;
  static Database s_database;
  static Cache s_cache;
  static bool s_loaded;
};

#endif /* Config_hpp */
