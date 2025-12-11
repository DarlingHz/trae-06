#include "Database.hpp"
#include <oatpp-sqlite/orm.hpp>
#include <iostream>
#include "../util/Logger.hpp"

// 静态成员初始化
std::shared_ptr<oatpp::sqlite::ConnectionPool> Database::s_connectionPool = nullptr;
bool Database::s_initialized = false;

bool Database::init(const std::string& dbPath) {
  try {
    // 创建数据库连接池
    s_connectionPool = oatpp::sqlite::ConnectionPool::createShared(
      dbPath,
      10, // 最大连接数
      std::chrono::seconds(5) // 连接超时
    );

    // 获取数据库连接
    auto connection = s_connectionPool->getConnection();
    if (!connection) {
      Logger::error("Failed to get database connection");
      return false;
    }

    // 创建会员表
    const char* createMemberTable = R"(
      CREATE TABLE IF NOT EXISTS members (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        phone TEXT NOT NULL UNIQUE,
        level TEXT NOT NULL DEFAULT 'normal',
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
      );
    )";
    connection->execute(createMemberTable);

    // 创建教练表
    const char* createCoachTable = R"(
      CREATE TABLE IF NOT EXISTS coaches (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        speciality TEXT NOT NULL
      );
    )";
    connection->execute(createCoachTable);

    // 创建课程模板表
    const char* createClassTemplateTable = R"(
      CREATE TABLE IF NOT EXISTS class_templates (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        title TEXT NOT NULL,
        level_required TEXT NOT NULL DEFAULT 'normal',
        capacity INTEGER NOT NULL,
        duration_minutes INTEGER NOT NULL,
        coach_id INTEGER NOT NULL,
        FOREIGN KEY (coach_id) REFERENCES coaches(id)
      );
    )";
    connection->execute(createClassTemplateTable);

    // 创建课节表
    const char* createClassSessionTable = R"(
      CREATE TABLE IF NOT EXISTS class_sessions (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        template_id INTEGER NOT NULL,
        start_time TEXT NOT NULL,
        status TEXT NOT NULL DEFAULT 'scheduled',
        capacity INTEGER NOT NULL,
        booked_count INTEGER NOT NULL DEFAULT 0,
        FOREIGN KEY (template_id) REFERENCES class_templates(id)
      );
    )";
    connection->execute(createClassSessionTable);

    // 创建预约表
    const char* createBookingTable = R"(
      CREATE TABLE IF NOT EXISTS bookings (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        member_id INTEGER NOT NULL,
        session_id INTEGER NOT NULL,
        status TEXT NOT NULL DEFAULT 'booked',
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (member_id) REFERENCES members(id),
        FOREIGN KEY (session_id) REFERENCES class_sessions(id),
        UNIQUE(member_id, session_id)
      );
    )";
    connection->execute(createBookingTable);

    // 创建训练记录表
    const char* createTrainingLogTable = R"(
      CREATE TABLE IF NOT EXISTS training_logs (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        member_id INTEGER NOT NULL,
        session_id INTEGER,
        notes TEXT NOT NULL,
        duration_minutes INTEGER NOT NULL,
        calories INTEGER,
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (member_id) REFERENCES members(id),
        FOREIGN KEY (session_id) REFERENCES class_sessions(id)
      );
    )";
    connection->execute(createTrainingLogTable);

    s_initialized = true;
    Logger::info("Database initialized successfully at %s", dbPath.c_str());
    return true;

  } catch (const std::exception& e) {
    Logger::error("Failed to initialize database: %s", e.what());
    return false;
  }
}

std::shared_ptr<oatpp::sqlite::ConnectionPool> Database::getConnectionPool() {
  return s_connectionPool;
}

void Database::shutdown() {
  if (s_connectionPool) {
    s_connectionPool->stop();
    s_connectionPool = nullptr;
  }
  s_initialized = false;
  Logger::info("Database shutdown completed");
}
