#pragma once

#include "chat_archive/service/StatsService.h"
#include <httplib.h>
#include <nlohmann/json.hpp>

namespace chat_archive {
namespace controller {

/**
 * @brief 统计数据控制器
 * 
 * 处理统计相关的HTTP请求，包括获取概览统计信息等
 */
class StatsController {
public:
    /**
     * @brief 构造函数
     * 
     * @param stats_service 统计服务实例
     */
    explicit StatsController(service::StatsService& stats_service)
        : stats_service_(stats_service) {}
    
    /**
     * @brief 初始化路由
     * 
     * @param server HTTP服务器实例
     */
    void init_routes(httplib::Server& server);
    
private:
    /**
     * @brief 处理获取概览统计信息的请求
     * 
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handle_get_stats_overview(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 发送成功响应
     * 
     * @param res HTTP响应
     * @param data 响应数据
     */
    void send_success_response(httplib::Response& res, const nlohmann::json& data);
    
    /**
     * @brief 发送错误响应
     * 
     * @param res HTTP响应
     * @param status_code HTTP状态码
     * @param error_code 错误代码
     * @param message 错误消息
     */
    void send_error_response(httplib::Response& res, int status_code, 
                              const std::string& error_code, const std::string& message);
    
    service::StatsService& stats_service_; ///< 统计服务实例
};

} // namespace controller
} // namespace chat_archive