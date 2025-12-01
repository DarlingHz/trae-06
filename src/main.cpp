#include <iostream>
#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include "dao/UserDao.h"
#include "dao/SleepSessionDao.h"
#include "dao/UserSettingDao.h"
#include "controller/UserController.h"
#include "controller/SleepSessionController.h"
#include "controller/StatsController.h"
#include "controller/UserSettingController.h"
#include "server/HttpServer.h"

using namespace nlohmann;

int main() {
    try {
        // 初始化数据库连接
        sqlite3* db;
        int rc = sqlite3_open("sleep_tracker.db", &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return 1;
        }

        // 设置数据库连接的错误处理函数
        sqlite3_config(SQLITE_CONFIG_LOG, [](void* arg, int i, const char* c) {
            std::cerr << "SQLite error: " << c << std::endl;
        }, nullptr);

        // 创建DAO实例
        dao::UserDao user_dao(db);
        dao::SleepSessionDao sleep_session_dao(db);
        dao::UserSettingDao user_setting_dao(db);

        // 初始化数据库表
        user_dao.createTable();
        sleep_session_dao.createTable();
        user_setting_dao.createTable();

        // 创建控制器实例
        controller::UserController user_controller(user_dao);
        controller::SleepSessionController sleep_session_controller(sleep_session_dao);
        controller::StatsController stats_controller(sleep_session_dao, user_setting_dao);
        controller::UserSettingController user_setting_controller(user_setting_dao);

        // 创建HTTP服务器实例
        server::HttpServer server(8080, 4, user_dao);

        // 注册用户相关路由
        server.registerRoute("POST", "/api/users/register", [&user_controller](const json& request_json) {
            return user_controller.handleRegister(request_json);
        });

        server.registerRoute("POST", "/api/users/login", [&user_controller](const json& request_json) {
            return user_controller.handleLogin(request_json);
        });

        // 注册睡眠记录相关路由
        server.registerRoute("POST", "/api/sleep_sessions", [&sleep_session_controller](const json& request_json, int user_id) {
            return sleep_session_controller.handleCreate(request_json, user_id);
        });

        server.registerRoute("GET", "/api/sleep_sessions", [&sleep_session_controller](const json& request_json, int user_id) {
            // 从请求中提取参数
            std::string start_date = request_json.value("start_date", "");
            std::string end_date = request_json.value("end_date", "");
            int page = request_json.value("page", 1);
            int page_size = request_json.value("page_size", 10);
            
            // 调用控制器方法
            return sleep_session_controller.handleQuery(start_date, end_date, page, page_size, user_id);
        });

        server.registerRoute("PUT", "/api/sleep_sessions/{id}", [&sleep_session_controller](const json& request_json, int user_id) {
            // 从请求中提取参数
            int id = request_json.value("id", 0);
            
            // 调用控制器方法
            return sleep_session_controller.handleUpdate(id, request_json, user_id);
        });

        server.registerRoute("DELETE", "/api/sleep_sessions/{id}", [&sleep_session_controller](const json& request_json, int user_id) {
            // 从请求中提取参数
            int id = request_json.value("id", 0);
            
            // 调用控制器方法
            return sleep_session_controller.handleDelete(id, user_id);
        });

        // 注册统计相关路由
        server.registerRoute("GET", "/api/stats/summary", [&stats_controller](const json& request_json, int user_id) {
            // 从请求中提取参数
            std::string start_date = request_json.value("start_date", "");
            std::string end_date = request_json.value("end_date", "");
            
            // 调用控制器方法
            return stats_controller.handleSummary(start_date, end_date, user_id);
        });

        // 注册用户设置相关路由
        server.registerRoute("GET", "/api/settings/goal", [&user_setting_controller](const json& request_json, int user_id) {
            return user_setting_controller.handleGet(user_id);
        });

        server.registerRoute("POST", "/api/settings/goal", [&user_setting_controller](const json& request_json, int user_id) {
            return user_setting_controller.handleUpdate(request_json, user_id);
        });

        // 启动服务器
        server.start();

        // 关闭数据库连接
        sqlite3_close(db);

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception in main: " << e.what() << std::endl;
        return 1;
    }
}
