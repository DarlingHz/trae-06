#include <config/Config.h>
#include <fstream>
#include <stdexcept>

namespace event_signup_service::config {

AppConfig Config::instance_;
bool Config::initialized_ = false;

void Config::initialize(const std::string& config_path) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        throw std::runtime_error("无法打开配置文件: " + config_path);
    }

    nlohmann::json config_json;
    try {
        config_file >> config_json;
    } catch (const std::exception& e) {
        throw std::runtime_error("解析配置文件失败: " + std::string(e.what()));
    }

    // 解析服务配置
    if (config_json.contains("service")) {
        auto service_json = config_json["service"];
        if (service_json.contains("port")) {
            instance_.service.port = service_json["port"].get<int>();
        }
        if (service_json.contains("host")) {
            instance_.service.host = service_json["host"].get<std::string>();
        }
        if (service_json.contains("log_level")) {
            instance_.service.log_level = service_json["log_level"].get<std::string>();
        }
    }

    // 解析数据库配置
    if (config_json.contains("database")) {
        auto db_json = config_json["database"];
        if (db_json.contains("path")) {
            instance_.database.path = db_json["path"].get<std::string>();
        }
    }

    initialized_ = true;
}

} // namespace event_signup_service::config
