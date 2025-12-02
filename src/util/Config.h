#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Config {
public:
    // 读取配置文件
    static bool load(const std::string& config_path);
    
    // 获取服务器配置
    static int getServerPort();
    static int getServerMaxConnections();
    static int getServerThreadPoolSize();
    
    // 获取数据库配置
    static std::string getDatabaseType();
    static std::string getDatabaseHost();
    static int getDatabasePort();
    static std::string getDatabaseName();
    static std::string getDatabaseUsername();
    static std::string getDatabasePassword();
    static int getDatabaseConnectionPoolSize();
    static std::string getDatabaseCharset();
    
    // 获取JWT配置
    static std::string getJwtSecret();
    static int getJwtExpiresIn();
    
    // 获取图书馆配置
    static int getLibraryMaxBorrowBooks();
    static int getLibraryBorrowPeriodDays();
    static int getLibraryReservationExpireHours();
    static double getLibraryOverdueFinePerDay();
    
    // 获取日志配置
    static std::string getLoggingLevel();
    static std::string getLoggingFilePath();
    static long getLoggingMaxFileSize();
    static int getLoggingMaxBackupFiles();
    
private:
    static json config_;
};

#endif // CONFIG_H