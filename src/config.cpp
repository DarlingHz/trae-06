#include "config.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>

namespace recruitment {

// Config类的构造函数已经在头文件中声明为默认构造函数，所以这里不需要再实现它。

Config& Config::instance() {
    static Config instance;
    return instance;
}

bool Config::load(const std::string& config_file) {
    // 初始化默认配置
    config_["server.port"] = "8080";
    config_["server.thread_pool_size"] = "4";
    config_["database.type"] = "sqlite";
    config_["database.path"] = "./recruitment.db";
    config_["log.level"] = "info";
    config_["log.file"] = "./recruitment.log";
    config_["auth.token"] = "";

    // 从环境变量加载配置
    loadFromEnvironment();

    // 如果提供了配置文件，则从配置文件加载配置
    if (!config_file.empty()) {
        if (!loadFromFile(config_file)) {
            return false;
        }
    }

    return true;
}

bool Config::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // 忽略空行和注释
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // 解析键值对
        size_t delimiter_pos = line.find('=');
        if (delimiter_pos == std::string::npos) {
            throw std::runtime_error("Invalid config line: " + line);
        }

        std::string key = line.substr(0, delimiter_pos);
        std::string value = line.substr(delimiter_pos + 1);

        // 移除键和值的空格
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        config_[key] = value;
    }

    file.close();
    return true;
}

void Config::loadFromEnvironment() {
    // 从环境变量加载配置
    const char* env_value;

    if ((env_value = std::getenv("SERVER_PORT")) != nullptr) {
        config_["server.port"] = env_value;
    }

    if ((env_value = std::getenv("SERVER_THREAD_POOL_SIZE")) != nullptr) {
        config_["server.thread_pool_size"] = env_value;
    }

    if ((env_value = std::getenv("DATABASE_TYPE")) != nullptr) {
        config_["database.type"] = env_value;
    }

    if ((env_value = std::getenv("DATABASE_PATH")) != nullptr) {
        config_["database.path"] = env_value;
    }

    if ((env_value = std::getenv("LOG_LEVEL")) != nullptr) {
        config_["log.level"] = env_value;
    }

    if ((env_value = std::getenv("LOG_FILE")) != nullptr) {
        config_["log.file"] = env_value;
    }

    if ((env_value = std::getenv("AUTH_TOKEN")) != nullptr) {
        config_["auth.token"] = env_value;
    }
}

std::string Config::getString(const std::string& key, const std::string& default_value) const {
    auto it = config_.find(key);
    if (it != config_.end()) {
        return it->second;
    }
    return default_value;
}

int Config::getInt(const std::string& key, int default_value) const {
    auto it = config_.find(key);
    if (it != config_.end()) {
        try {
            return std::stoi(it->second);
        } catch (const std::exception& e) {
            // 转换失败，返回默认值
        }
    }
    return default_value;
}

bool Config::getBool(const std::string& key, bool default_value) const {
    auto it = config_.find(key);
    if (it != config_.end()) {
        const std::string& value = it->second;
        if (value == "true" || value == "1") {
            return true;
        } else if (value == "false" || value == "0") {
            return false;
        }
    }
    return default_value;
}

} // namespace recruitment