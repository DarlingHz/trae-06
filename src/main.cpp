#include <iostream>
#include <string>
#include "server.h"
#include "config.h"
#include "database.h"
#include "logger.h"

int main() {
    try {
        // 加载配置
        ConfigManager& config = ConfigManager::instance();
        if (!config.load_config("config.json")) {
            std::cerr << "Failed to load config" << std::endl;
            return 1;
        }
        
        // 初始化日志
        Logger::instance().init(
            config.get_config().log.file,
            config.get_config().log.level == "debug" ? LogLevel::DEBUG :
            config.get_config().log.level == "info" ? LogLevel::INFO :
            config.get_config().log.level == "warn" ? LogLevel::WARN :
            config.get_config().log.level == "error" ? LogLevel::ERROR : LogLevel::FATAL
        );
        
        Logger::instance().log(LogLevel::INFO, "Server starting...");
        
        // 连接数据库
        if (!Database::instance().connect(config.get_config().database.path)) {
            Logger::instance().log(LogLevel::FATAL, "Failed to connect to database");
            return 1;
        }
        
        Logger::instance().log(LogLevel::INFO, "Database connected");
        
        // 设置并启动服务器
        httplib::Server svr;
        setup_routes(svr);
        
        std::string host = config.get_config().server.host;
        int port = config.get_config().server.port;
        
        Logger::instance().log(LogLevel::INFO, 
            "Server listening on http://" + host + ":" + std::to_string(port));
        
        if (svr.listen(host.c_str(), port)) {
            Logger::instance().log(LogLevel::INFO, "Server started successfully");
        } else {
            Logger::instance().log(LogLevel::FATAL, 
                "Failed to start server on port " + std::to_string(port));
            return 1;
        }
        
    } catch (const std::exception& e) {
        // 直接尝试记录日志，因为Logger::init()会检查文件是否打开
        try {
            Logger::instance().log(LogLevel::FATAL, std::string("Server error: ") + e.what());
        } catch (...) {
            // 如果日志记录失败，直接输出到标准错误
            std::cerr << "Server error: " << e.what() << std::endl;
        }
        return 1;
    }
    
    return 0;
}