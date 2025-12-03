#pragma once

#include <string>
#include <unordered_map>

class ConfigManager {
private:
    std::unordered_map<std::string, std::string> configMap;

    ConfigManager() {}
    ~ConfigManager() = default;

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = delete;
    ConfigManager& operator=(ConfigManager&&) = delete;

    bool parseJSON(const std::string& jsonContent);

public:
    static ConfigManager& getInstance();

    bool loadConfig(const std::string& filePath);
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;
    bool getBool(const std::string& key, bool defaultValue = false) const;
    double getDouble(const std::string& key, double defaultValue = 0.0) const;

    void printConfig() const;
};