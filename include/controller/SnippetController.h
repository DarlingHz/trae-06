#ifndef SNIPPET_CONTROLLER_H
#define SNIPPET_CONTROLLER_H

#include <memory>
#include <string>
#include "server/HttpServer.h"
#include "service/SnippetService.h"
#include "service/UserService.h"
#include "model/User.h"

namespace controller {

class SnippetController {
public:
    SnippetController(std::shared_ptr<service::SnippetService> snippet_service, 
                       std::shared_ptr<service::UserService> user_service,
                       std::shared_ptr<server::HttpServer> http_server);

    // 禁止拷贝构造函数和赋值运算符
    SnippetController(const SnippetController&) = delete;
    SnippetController& operator=(const SnippetController&) = delete;

    // 允许移动构造函数和赋值运算符
    SnippetController(SnippetController&&) = default;
    SnippetController& operator=(SnippetController&&) = default;

    // 注册所有 API 端点
    void registerEndpoints();

private:
    // 处理创建代码片段的请求
    void handleCreateSnippet(const server::http::request<server::http::string_body>& request, 
                               server::http::response<server::http::string_body>& response);

    // 处理获取单个代码片段详情的请求
    void handleGetSnippetById(const server::http::request<server::http::string_body>& request, 
                                server::http::response<server::http::string_body>& response);

    // 处理更新代码片段的请求
    void handleUpdateSnippet(const server::http::request<server::http::string_body>& request, 
                               server::http::response<server::http::string_body>& response);

    // 处理删除代码片段的请求
    void handleDeleteSnippet(const server::http::request<server::http::string_body>& request, 
                               server::http::response<server::http::string_body>& response);

    // 处理搜索代码片段的请求
    void handleSearchSnippets(const server::http::request<server::http::string_body>& request, 
                                server::http::response<server::http::string_body>& response);

    // 处理收藏代码片段的请求
    void handleStarSnippet(const server::http::request<server::http::string_body>& request, 
                            server::http::response<server::http::string_body>& response);

    // 处理取消收藏代码片段的请求
    void handleUnstarSnippet(const server::http::request<server::http::string_body>& request, 
                              server::http::response<server::http::string_body>& response);

    // 处理列出某个用户的代码片段的请求
    void handleGetUserSnippets(const server::http::request<server::http::string_body>& request, 
                                 server::http::response<server::http::string_body>& response);

    // 从请求头中提取 token 并验证用户身份
    std::optional<model::User> authenticateUser(const server::http::request<server::http::string_body>& request);

    // 依赖注入的服务
    std::shared_ptr<service::SnippetService> snippet_service_;
    std::shared_ptr<service::UserService> user_service_;
    std::shared_ptr<server::HttpServer> http_server_;
};

} // namespace controller

#endif // SNIPPET_CONTROLLER_H