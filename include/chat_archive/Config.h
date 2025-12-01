#pragma once

#include <string>
#include <optional>

namespace chat_archive {

class Config {
public:
    static Config& get() {
        static Config instance;
        return instance;
    }
    
    bool load(const std::string& file_path = "");
    
    // 服务器配置
    int get_port() const { return port_; }
    
    // 数据库配置
    const std::string& get_db_path() const { return db_path_; }
    
    // 日志配置
    const std::string& get_log_level() const { return log_level_; }
    
private:
    Config() = default;
    ~Config() = default;
    
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    // 服务器配置
    int port_ = 8080;
    
    // 数据库配置
    std::string db_path_ = "./chat_archive.db";
    
    // 日志配置
    std::string log_level_ = "info";
};

} // namespace chat_archive