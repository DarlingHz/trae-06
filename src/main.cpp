#include "parking/config.h"
#include "parking/database.h"
#include "parking/utils.h"
#include "parking/models.h"
#include "parking/dao.h"
#include "parking/services.h"
#include "parking/controllers.h"
#include <httplib.h>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>

using namespace std;
using namespace httplib;

void load_database_schema(const string& db_path) {
    try {
        ifstream file("sql/init.sql");
        if (!file.is_open()) {
            cerr << "Warning: Could not open database schema file (sql/init.sql)" << endl;
            return;
        }

        string schema((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        db::init(db_path);
        db::get().execute(schema);
        Logger::info("Database schema loaded successfully");
    } catch (const exception& e) {
        cerr << "Error loading database schema: " << e.what() << endl;
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    try {
        // 加载配置
        CONFIG.load();

        // 初始化数据库
        load_database_schema(CONFIG.db_path());

        // 创建DAO实例
        auto user_dao = make_unique<SQLiteUserDAO>(db::get());
        auto spot_dao = make_unique<SQLiteParkingSpotDAO>(db::get());
        auto reservation_dao = make_unique<SQLiteReservationDAO>(db::get());
        auto session_dao = make_unique<SQLiteSessionDAO>(db::get());

        // 初始化Services
        auto user_service = make_unique<UserService>(user_dao.get(), session_dao.get());
        auto parking_spot_service = make_unique<ParkingSpotService>(user_dao.get(), parking_spot_dao.get());
        auto reservation_service = make_unique<ReservationService>(parking_spot_dao.get(), reservation_dao.get());

        // 初始化Controllers
        auto user_controller = make_unique<UserController>(user_service.get());
        auto parking_spot_controller = make_unique<ParkingSpotController>(parking_spot_service.get(), user_service.get());
        auto reservation_controller = make_unique<ReservationController>(reservation_service.get(), user_service.get());

        // 初始化Auth Middleware
        auto auth_middleware = make_unique<AuthMiddleware>(user_service.get());

        // 设置服务器
        Server server;
        setup_routes(server, user_controller.get(), parking_spot_controller.get(), 
                     reservation_controller.get(), auth_middleware.get());

        // 添加请求日志中间件
        server.set_pre_routing_handler([](const Request& req, Response& res) {
            Logger::info(req.method + " " + req.path);
            return true;
        });

        // 设置错误处理器
        server.set_error_handler([](const Request& req, Response& res) {
            if (res.status == 404) {
                ResponseUtils::error(res, 404, "Endpoint not found");
            } else {
                ResponseUtils::error(res, res.status, "Internal server error");
            }
        });

        // 启动服务器
        string host = "0.0.0.0";
        int port = CONFIG.port();
        Logger::info("Starting server on http://" + host + ":" + to_string(port));

        if (!server.listen(host.c_str(), port)) {
            cerr << "Failed to start server" << endl;
            return 1;
        }

        return 0;
    } catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }
}
