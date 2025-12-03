#include "utils/config.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

Config::Config() {
    // 加载配置文件
    loadConfig("config/config.json");
}

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

Config::~Config() {}

void Config::loadConfig(const std::string& config_path) {
    try {
        std::ifstream config_file(config_path);
        if (!config_file.is_open()) {
            throw std::runtime_error("Failed to open config file: " + config_path);
        }

        json config_json;
        config_file >> config_json;
        config_data_ = config_json;

        LOG_INFO("Config file loaded successfully: " << config_path);
    } catch (const std::exception& e) {
        LOG_ERROR("Error loading config file: " << e.what());
        throw;
    }
}

std::string Config::getString(const std::string& key, const std::string& default_value) const {
    try {
        return config_data_[key].get<std::string>();
    } catch (const std::exception& e) {
        LOG_WARNING("Key not found in config: " << key << ", using default value: " << default_value);
        return default_value;
    }
}

int Config::getInt(const std::string& key, int default_value) const {
    try {
        return config_data_[key].get<int>();
    } catch (const std::exception& e) {
        LOG_WARNING("Key not found in config: " << key << ", using default value: " << default_value);
        return default_value;
    }
}

double Config::getDouble(const std::string& key, double default_value) const {
    try {
        return config_data_[key].get<double>();
    } catch (const std::exception& e) {
        LOG_WARNING("Key not found in config: " << key << ", using default value: " << default_value);
        return default_value;
    }
}

bool Config::getBool(const std::string& key, bool default_value) const {
    try {
        return config_data_[key].get<bool>();
    } catch (const std::exception& e) {
        LOG_WARNING("Key not found in config: " << key << ", using default value: " << default_value);
        return default_value;
    }
}
