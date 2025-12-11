#include "config.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

bool Config::load(const std::string& config_file) {
    try {
        // 打开配置文件
        std::ifstream ifs(config_file);
        if (!ifs.is_open()) {
            std::cerr << "Failed to open config file: " << config_file << std::endl;
            return false;
        }

        // 解析JSON
        json j;
        ifs >> j;

        // 读取配置项
        if (j.contains("port")) {
            port_ = j["port"].get<int>();
        }

        if (j.contains("max_threads")) {
            max_threads_ = j["max_threads"].get<int>();
        }

        if (j.contains("cache_capacity")) {
            cache_capacity_ = j["cache_capacity"].get<int>();
        }

        if (j.contains("db_path")) {
            db_path_ = j["db_path"].get<std::string>();
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load config: " << e.what() << std::endl;
        return false;
    }
}