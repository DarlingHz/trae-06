#ifndef CONFIG_H
#define CONFIG_H

#include <string>

/**
 * @brief 配置类，用于读取和存储服务配置
 */
class Config {
public:
    /**
     * @brief 从配置文件中加载配置
     * @param config_file 配置文件路径
     * @return 是否加载成功
     */
    bool load(const std::string& config_file);

    /**
     * @brief 获取服务监听端口
     * @return 端口号
     */
    int getPort() const { return port_; }

    /**
     * @brief 获取最大工作线程数
     * @return 最大工作线程数
     */
    int getMaxThreads() const { return max_threads_; }

    /**
     * @brief 获取LRU缓存容量
     * @return 缓存容量
     */
    int getCacheCapacity() const { return cache_capacity_; }

    /**
     * @brief 获取数据库路径
     * @return 数据库路径
     */
    const std::string& getDbPath() const { return db_path_; }

private:
    int port_ = 8080;               ///< 服务监听端口
    int max_threads_ = 4;           ///< 最大工作线程数
    int cache_capacity_ = 100;      ///< LRU缓存容量
    std::string db_path_ = "./data/collab_doc.db"; ///< 数据库路径
};

#endif // CONFIG_H