#ifndef JOB_SERVICE_CONFIG_H
#define JOB_SERVICE_CONFIG_H

#include <string>
#include <nlohmann/json.hpp>

namespace job_service {

// 配置类
class Config {
private:
    int port_;
    size_t thread_pool_size_;
    std::string storage_path_;
    std::string log_level_;
    size_t queue_max_size_;

public:
    // 默认构造
    Config();
    
    // 从JSON文件加载配置
    bool load_from_file(const std::string& file_path);
    
    // 从JSON对象加载配置
    void load_from_json(const nlohmann::json& json);
    
    // 获取监听端口
    int get_port() const;
    
    // 获取线程池大小
    size_t get_thread_pool_size() const;
    
    // 获取存储路径
    const std::string& get_storage_path() const;
    
    // 获取日志级别
    const std::string& get_log_level() const;
    
    // 获取队列最大大小
    size_t get_queue_max_size() const;
    
    // 设置默认配置
    void set_defaults();
    
    // 验证配置
    bool validate() const;
    
    // 转换为JSON
    nlohmann::json to_json() const;
};

} // namespace job_service

#endif // JOB_SERVICE_CONFIG_H
