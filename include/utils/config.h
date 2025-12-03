#ifndef CONFIG_H
#define CONFIG_H

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class Config {
private:
    json config_data_;

    // 私有构造函数，实现单例模式
    Config();
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    // 加载配置文件
    void loadConfig(const std::string& config_path);

public:
    // 单例模式获取实例
    static Config& getInstance();

    // 析构函数
    ~Config();

    // 获取字符串配置项
    std::string getString(const std::string& key, const std::string& default_value = "") const;

    // 获取整数配置项
    int getInt(const std::string& key, int default_value = 0) const;

    // 获取浮点数配置项
    double getDouble(const std::string& key, double default_value = 0.0) const;

    // 获取布尔值配置项
    bool getBool(const std::string& key, bool default_value = false) const;
};

#endif // CONFIG_H
