#include <iostream>
#include <vector>
#include <csignal>
#include <thread>
#include <chrono>

// 避免cpprest和mysqlx之间的宏定义冲突
#undef U

#include "controller/UserController.h"
#include "controller/BookController.h"
#include "controller/BorrowController.h"
#include "controller/ReservationController.h"
#include "util/Logger.h"
#include "util/DatabaseConnectionPool.h"

using namespace std;

// 全局变量，用于存储控制器实例
vector<shared_ptr<Controller>> controllers;

// 信号处理函数，用于优雅地关闭服务器
void signalHandler(int signum) {
    Logger::info("Received signal " + to_string(signum) + ", shutting down servers...");
    
    // 停止所有控制器
    for (const auto& controller : controllers) {
        controller->stop();
    }
    
    // 关闭数据库连接池
        DatabaseConnectionPool::close();
    
    Logger::info("All servers have been shut down successfully");
    
    // 退出程序
    exit(signum);
}

int main(int argc, char** argv) {
    try {
        // 初始化日志系统
        Logger::init("library_management_system.log");
        Logger::info("Library Management System started");
        
        // 初始化数据库连接池
        std::string db_host = "localhost";
        std::string db_port = "3306";
        std::string db_name = "library_management_system";
        std::string db_user = "root";
        std::string db_password = "password";
        int pool_size = 10;
        
        // 从命令行参数获取数据库连接信息（可选）
        if (argc >= 6) {
            db_host = argv[1];
            db_port = argv[2];
            db_name = argv[3];
            db_user = argv[4];
            db_password = argv[5];
            if (argc >= 7) {
                pool_size = stoi(argv[6]);
            }
        }
        
        // 初始化数据库连接池
        bool init_success = DatabaseConnectionPool::init(
            pool_size, db_host, stoi(db_port), db_user, db_password, db_name
        );
        if (!init_success) {
            Logger::error("Failed to initialize database connection pool");
            return 1;
        }
        Logger::info("Database connection pool initialized successfully");
        
        // 创建控制器实例
        shared_ptr<UserController> user_controller = make_shared<UserController>("http://localhost:8080/users");
        shared_ptr<BookController> book_controller = make_shared<BookController>("http://localhost:8080/books");
        shared_ptr<BorrowController> borrow_controller = make_shared<BorrowController>("http://localhost:8080/borrows");
        shared_ptr<ReservationController> reservation_controller = make_shared<ReservationController>("http://localhost:8080/reservations");
        
        // 将控制器实例添加到全局向量中
        controllers.push_back(user_controller);
        controllers.push_back(book_controller);
        controllers.push_back(borrow_controller);
        controllers.push_back(reservation_controller);
        
        // 启动所有控制器
        for (const auto& controller : controllers) {
            controller->start();
        }
        
        // 注册信号处理函数
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        
        Logger::info("All HTTP servers have been started successfully");
        Logger::info("Library Management System is now running");
        
        // 保持程序运行
        while (true) {
            this_thread::sleep_for(chrono::seconds(1));
        }
        
    } catch (const exception& e) {
        Logger::error("Failed to start Library Management System: " + std::string(e.what()));
        cerr << "Failed to start Library Management System: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}