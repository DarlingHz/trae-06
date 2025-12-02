#ifndef ROUTER_H
#define ROUTER_H

#include <memory>
#include <httplib.h>
#include "service.h"

/**
 * @brief 路由类，处理HTTP请求并转发给业务逻辑层
 */
class Router {
public:
    /**
     * @brief 构造函数
     * @param service 业务逻辑服务对象指针
     */
    explicit Router(std::shared_ptr<Service> service);

    /**
     * @brief 初始化路由
     * @param server HTTP服务器对象
     */
    void init(httplib::Server& server);

private:
    // 用户管理相关路由
    /**
     * @brief 处理创建用户请求
     * @param req HTTP请求对象
     * @param res HTTP响应对象
     */
    void handleCreateUser(const httplib::Request& req, httplib::Response& res);

    // 文档管理相关路由
    /**
     * @brief 处理创建文档请求
     * @param req HTTP请求对象
     * @param res HTTP响应对象
     */
    void handleCreateDocument(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 处理获取文档列表请求
     * @param req HTTP请求对象
     * @param res HTTP响应对象
     */
    void handleGetDocuments(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 处理获取文档详情请求
     * @param req HTTP请求对象
     * @param res HTTP响应对象
     */
    void handleGetDocumentDetail(const httplib::Request& req, httplib::Response& res);

    // 文档版本管理相关路由
    /**
     * @brief 处理创建文档版本请求
     * @param req HTTP请求对象
     * @param res HTTP响应对象
     */
    void handleCreateDocumentVersion(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 处理获取文档版本列表请求
     * @param req HTTP请求对象
     * @param res HTTP响应对象
     */
    void handleGetDocumentVersions(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 处理获取文档指定版本详情请求
     * @param req HTTP请求对象
     * @param res HTTP响应对象
     */
    void handleGetDocumentVersion(const httplib::Request& req, httplib::Response& res);

    // 评论管理相关路由
    /**
     * @brief 处理创建评论请求
     * @param req HTTP请求对象
     * @param res HTTP响应对象
     */
    void handleCreateComment(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 处理获取文档评论列表请求
     * @param req HTTP请求对象
     * @param res HTTP响应对象
     */
    void handleGetComments(const httplib::Request& req, httplib::Response& res);

    // 统计与健康检查相关路由
    /**
     * @brief 处理获取统计信息请求
     * @param req HTTP请求对象
     * @param res HTTP响应对象
     */
    void handleGetMetrics(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 处理健康检查请求
     * @param req HTTP请求对象
     * @param res HTTP响应对象
     */
    void handleHealthCheck(const httplib::Request& req, httplib::Response& res);

    // 辅助方法
    /**
     * @brief 发送成功响应
     * @param res HTTP响应对象
     * @param data 响应数据（JSON格式）
     */
    void sendSuccessResponse(httplib::Response& res, const std::string& data);

    /**
     * @brief 发送错误响应
     * @param res HTTP响应对象
     * @param error_code 错误码
     * @param message 错误信息
     * @param status_code HTTP状态码（默认为400）
     */
    void sendErrorResponse(httplib::Response& res, int error_code, const std::string& message, int status_code = 400);

private:
    std::shared_ptr<Service> service_; ///< 业务逻辑服务对象指针
};

#endif // ROUTER_H