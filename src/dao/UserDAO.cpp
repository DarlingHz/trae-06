#include "UserDAO.h"
#include "../util/Logger.h"
#include <stdexcept>

bool UserDAO::registerUser(const User& user) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for user registration");
            return false;
        }
        
        // 执行插入用户的SQL语句
        Table users_table = session->getSchema(DatabaseConnectionPool::getDatabaseName()).getTable("users");
        auto insert_stmt = users_table.insert("username", "nickname", "email", "password_hash", "role", "status")
            .values(user.getUsername(), user.getNickname(), user.getEmail(), user.getPasswordHash(), user.getRole(), user.getStatus());
        
        mysqlx::Result result = insert_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to insert user into database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        Logger::info("User registered successfully, username: " + user.getUsername());
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to register user: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<User> UserDAO::getUserByUsername(const std::string& username) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting user by username");
            return nullptr;
        }
        
        // 执行查询用户的SQL语句
        Table users_table = session->getSchema(DatabaseConnectionPool::getDatabaseName()).getTable("users");
        auto select_stmt = users_table.select("*")
            .where("username = :username")
            .bind("username", username);
        
        mysqlx::RowResult result = select_stmt.execute();
        std::vector<mysqlx::Row> rows = result.fetchAll();
        if (rows.empty()) {
            Logger::debug("User not found by username: " + username);
            DatabaseConnectionPool::releaseConnection(session);
            return nullptr;
        }
        
        // 从结果集中创建User对象
        mysqlx::Row row = rows[0];
        std::shared_ptr<User> user = createUserFromResult(row);
        
        DatabaseConnectionPool::releaseConnection(session);
        return user;
    } catch (const std::exception& e) {
        Logger::error("Failed to get user by username: " + std::string(e.what()));
        return nullptr;
    }
}

std::shared_ptr<User> UserDAO::getUserByEmail(const std::string& email) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting user by email");
            return nullptr;
        }
        
        // 执行查询用户的SQL语句
        Table users_table = session->getSchema(DatabaseConnectionPool::getDatabaseName()).getTable("users");
        auto select_stmt = users_table.select("*")
            .where("email = :email")
            .bind("email", email);
        
        mysqlx::RowResult result = select_stmt.execute();
        std::vector<mysqlx::Row> rows = result.fetchAll();
        if (rows.empty()) {
            Logger::debug("User not found by email: " + email);
            DatabaseConnectionPool::releaseConnection(session);
            return nullptr;
        }
        
        // 从结果集中创建User对象
        mysqlx::Row row = rows[0];
        std::shared_ptr<User> user = createUserFromResult(row);
        
        DatabaseConnectionPool::releaseConnection(session);
        return user;
    } catch (const std::exception& e) {
        Logger::error("Failed to get user by email: " + std::string(e.what()));
        return nullptr;
    }
}

std::shared_ptr<User> UserDAO::getUserById(int user_id) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting user by id");
            return nullptr;
        }
        
        // 执行查询用户的SQL语句
        Table users_table = session->getSchema(DatabaseConnectionPool::getDatabaseName()).getTable("users");
        auto select_stmt = users_table.select("*")
            .where("id = :user_id")
            .bind("user_id", user_id);
        
        mysqlx::RowResult result = select_stmt.execute();
        std::vector<mysqlx::Row> rows = result.fetchAll();
        if (rows.empty()) {
            Logger::debug("User not found by id: " + std::to_string(user_id));
            DatabaseConnectionPool::releaseConnection(session);
            return nullptr;
        }
        
        // 从结果集中创建User对象
        mysqlx::Row row = rows[0];
        std::shared_ptr<User> user = createUserFromResult(row);
        
        DatabaseConnectionPool::releaseConnection(session);
        return user;
    } catch (const std::exception& e) {
        Logger::error("Failed to get user by id: " + std::string(e.what()));
        return nullptr;
    }
}

bool UserDAO::updateUser(const User& user) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for updating user");
            return false;
        }
        
        // 执行更新用户的SQL语句
        Table users_table = session->getSchema(DatabaseConnectionPool::getDatabaseName()).getTable("users");
        auto update_stmt = users_table.update()
            .set("nickname", user.getNickname())
            .set("email", user.getEmail())
            .set("role", user.getRole())
            .set("status", user.getStatus())
            .where("id = :user_id")
            .bind("user_id", user.getId());
        
        mysqlx::Result result = update_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to update user in database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        Logger::info("User updated successfully, user id: " + std::to_string(user.getId()));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to update user: " + std::string(e.what()));
        return false;
    }
}

bool UserDAO::updateUserPassword(int user_id, const std::string& new_password_hash) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for updating user password");
            return false;
        }
        
        // 执行更新用户密码的SQL语句
        Table users_table = session->getSchema(DatabaseConnectionPool::getDatabaseName()).getTable("users");
        auto update_stmt = users_table.update()
            .set("password_hash", new_password_hash)
            .where("id = :user_id")
            .bind("user_id", user_id);
        
        mysqlx::Result result = update_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to update user password in database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        Logger::info("User password updated successfully, user id: " + std::to_string(user_id));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to update user password: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::shared_ptr<User>> UserDAO::getAllUsers(int page, int page_size) {
    std::vector<std::shared_ptr<User>> users;
    
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting all users");
            return users;
        }
        
        // 计算分页偏移量
        int offset = (page - 1) * page_size;
        
        // 执行查询所有用户的SQL语句
        Table users_table = session->getSchema("library_management_system").getTable("users");
        auto select_stmt = users_table.select("*")
            .orderBy("id")
            .limit(page_size)
            .offset(offset);
        
        mysqlx::RowResult result = select_stmt.execute();
        // 使用fetchAll()方法获取所有行，然后遍历这些行
        std::vector<mysqlx::Row> rows = result.fetchAll();
        for (const auto& row : rows) {
            std::shared_ptr<User> user = createUserFromResult(row);
            users.push_back(user);
        }
        
        DatabaseConnectionPool::releaseConnection(session);
        return users;
    } catch (const std::exception& e) {
        Logger::error("Failed to get all users: " + std::string(e.what()));
        return users;
    }
}

int UserDAO::getUserCount() {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting user count");
            return 0;
        }
        
        // 执行查询用户总数的SQL语句
        Table users_table = session->getSchema("library_management_system").getTable("users");
        auto select_stmt = users_table.select("COUNT(*)");
        
        mysqlx::RowResult result = select_stmt.execute();
        // 使用fetchOne()方法获取结果集中的第一行
        mysqlx::Row row = result.fetchOne();
        if (!row) {
            Logger::debug("No users found");
            DatabaseConnectionPool::releaseConnection(session);
            return 0;
        }
        int user_count = row[0].get<int>();
        
        DatabaseConnectionPool::releaseConnection(session);
        return user_count;
    } catch (const std::exception& e) {
        Logger::error("Failed to get user count: " + std::string(e.what()));
        return 0;
    }
}

bool UserDAO::toggleUserStatus(int user_id, const std::string& status) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for toggling user status");
            return false;
        }
        
        // 执行更新用户状态的SQL语句
        Table users_table = session->getSchema("library_management_system").getTable("users");
        auto update_stmt = users_table.update()
            .set("status", status)
            .where("id = :user_id")
            .bind("user_id", user_id);
        
        mysqlx::Result result = update_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to toggle user status in database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        Logger::info("User status toggled successfully, user id: " + std::to_string(user_id) + ", new status: " + status);
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to toggle user status: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<User> UserDAO::createUserFromResult(const Row& row) {
    int id = row[0].get<int>();
    std::string username = row[1].get<std::string>();
    std::string nickname = row[2].get<std::string>();
    std::string email = row[3].get<std::string>();
    std::string password_hash = row[4].get<std::string>();
    std::string role = row[5].get<std::string>();
    std::string status = row[6].get<std::string>();
    std::string created_at = row[7].get<std::string>();
    std::string updated_at = row[8].get<std::string>();
    
    return std::make_shared<User>(id, username, nickname, email, password_hash, role, status, created_at, updated_at);
}