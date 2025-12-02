#ifndef Database_hpp
#define Database_hpp

#include <oatpp-sqlite/orm.hpp>
#include <string>



class Database {
public:
  // 初始化数据库连接
  static bool init(const std::string& dbPath);

  // 获取数据库连接池
  static std::shared_ptr<oatpp::sqlite::ConnectionPool> getConnectionPool();

  // 关闭数据库连接
  static void shutdown();

private:
  static std::shared_ptr<oatpp::sqlite::ConnectionPool> s_connectionPool;
  static bool s_initialized;
};



#endif /* Database_hpp */
