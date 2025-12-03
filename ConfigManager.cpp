#include "ConfigManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cctype>

ConfigManager::ConfigManager() {
}

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::parseJSON(const std::string& jsonContent) {
    size_t pos = 0;
    size_t n = jsonContent.size();
    bool inObject = false;
    std::string currentKey;
    std::string currentValue;
    enum State { INIT, KEY, VALUE, STRING_VALUE };
    State state = INIT;

    while (pos < n) {
        char c = jsonContent[pos];

        switch (state) {
            case INIT:
                if (c == '{') {
                    inObject = true;
                    state = KEY;
                } else if (!isspace(c)) {
                    return false;
                }
                break;

            case KEY:
                if (c == '"') {
                    state = STRING_VALUE;
                    currentKey.clear();
                } else if (!isspace(c)) {
                    return false;
                }
                break;

            case STRING_VALUE:
                if (c == '"') {
                    if (state == STRING_VALUE && !currentKey.empty()) {
                        state = VALUE;
                    } else if (state == VALUE) {
                        configMap[currentKey] = currentValue;
                        currentValue.clear();
                        state = KEY;
                    }
                } else if (c == '\') {
                    if (pos + 1 < n) {
                        currentValue += jsonContent[pos + 1];
                        pos++;
                    }
                } else {
                    if (state == STRING_VALUE) {
                        currentKey += c;
                    } else {
                        currentValue += c;
                    }
                }
                break;

            case VALUE:
                if (c == ':') {
                    state = STRING_VALUE;
                    currentValue.clear();
                } else if (c == ',') {
                    state = KEY;
                } else if (c == '}') {
                    return true;
                } else if (!isspace(c)) {
                    return false;
                }
                break;
        }
        pos++;
    }

    return inObject;
}

bool ConfigManager::loadConfig(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Could not open config file: " << filePath << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonContent = buffer.str();
    file.close();

    return parseJSON(jsonContent);
}

std::string ConfigManager::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = configMap.find(key);
    return (it != configMap.end()) ? it->second : defaultValue;
}

int ConfigManager::getInt(const std::string& key, int defaultValue) const {
    auto it = configMap.find(key);
    if (it == configMap.end()) {
        return defaultValue;
    }
    try {
        return std::stoi(it->second);
    } catch (...) {
        return defaultValue;
    }
}

bool ConfigManager::getBool(const std::string& key, bool defaultValue) const {
    auto it = configMap.find(key);
    if (it == configMap.end()) {
        return defaultValue;
    }
    std::string value = it->second;
    if (value == "true" || value == "1") {
        return true;
    } else if (value == "false" || value == "0") {
        return false;
    }
    return defaultValue;
}

double ConfigManager::getDouble(const std::string& key, double defaultValue) const {
    auto it = configMap.find(key);
    if (it == configMap.end()) {
        return defaultValue;
    }
    try {
        return std::stod(it->second);
    } catch (...) {
        return defaultValue;
    }
}

void ConfigManager::printConfig() const {
    std::cout << "Configuration:" << std::endl;
    for (const auto& pair : configMap) {
        std::cout << "  " << pair.first << ": " << pair.second << std::endl;
    }
}