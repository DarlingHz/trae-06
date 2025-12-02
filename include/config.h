#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>

namespace recruitment {

class Config {
public:
    /**
     * @brief 加载配置
     * @param config_file 配置文件路径，可选
     * @return 加载成功返回true，否则返回false
     */
    bool load(const std::string& config_file = "");

    /**
     * @brief 获取字符串类型的配置项
     * @param key 配置项键名
     * @param default_value 默认值
     * @return 配置项的值，如果不存在则返回默认值
     */
    std::string getString(const std::string& key, const std::string& default_value = "") const;

    /**
     * @brief 获取整数类型的配置项
     * @param key 配置项键名
     * @param default_value 默认值
     * @return 配置项的值，如果不存在则返回默认值
     */
    int getInt(const std::string& key, int default_value = 0) const;

    /**
     * @brief 获取布尔类型的配置项
     * @param key 配置项键名
     * @param default_value 默认值
     * @return 配置项的值，如果不存在则返回默认值
     */
    bool getBool(const std::string& key, bool default_value = false) const;

    /**
     * @brief 获取单例实例
     * @return 配置类的单例实例
     */
    static Config& instance();

private:
    Config() = default;
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    /**
     * @brief 从环境变量加载配置
     */
    void loadFromEnvironment();

    /**
     * @brief 从配置文件加载配置
     * @param config_file 配置文件路径
     * @return 加载成功返回true，否则返回false
     */
    bool loadFromFile(const std::string& config_file);

    std::unordered_map<std::string, std::string> config_;
};

} // namespace recruitment

#endif // CONFIG_H