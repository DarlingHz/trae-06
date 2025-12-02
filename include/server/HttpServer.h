#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <iostream>

namespace server {

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
using tcp = asio::ip::tcp;

class HttpServer {
public:
    using RequestHandler = std::function<void(const http::request<http::string_body>&, 
                                                 http::response<http::string_body>&)>;

    HttpServer(asio::io_context& io_context, const std::string& address, const std::string& port);
    ~HttpServer() = default;

    // 禁止拷贝构造函数和赋值运算符
    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    // 禁止移动构造函数和赋值运算符，因为 std::io_context& 是不可移动的
    HttpServer(HttpServer&&) noexcept = delete;
    HttpServer& operator=(HttpServer&&) noexcept = delete;

    /**
     * @brief 注册请求处理函数
     * @param method HTTP方法（GET、POST、PUT、DELETE等）
     * @param path 请求路径
     * @param handler 处理函数
     */
    void registerHandler(const std::string& method, const std::string& path, RequestHandler handler);

    /**
     * @brief 启动服务器
     */
    void start();

private:
    /**
     * @brief 接受新的连接
     */
    void accept();

    /**
     * @brief 处理连接
     * @param socket TCP套接字
     */
    void handleConnection(tcp::socket socket);

    /**
     * @brief 读取请求
     * @param stream HTTP流
     * @param buffer 缓冲区
     * @param request 请求对象
     * @param self 共享指针
     */
    template <typename Stream>
    void readRequest(std::shared_ptr<Stream> stream, 
                     std::shared_ptr<beast::flat_buffer> buffer, 
                     std::shared_ptr<http::request<http::string_body>> request, 
                     std::shared_ptr<void> self) {
        auto& socket = stream->socket();
        http::async_read(*stream, *buffer, *request, 
            [this, stream, buffer, request, self](beast::error_code ec, std::size_t bytes_transferred) {
                boost::ignore_unused(bytes_transferred);
                if (ec) {
                    if (ec != http::error::end_of_stream) {
                        std::cerr << "Error reading request: " << ec.message() << std::endl;
                    }
                    return;
                }
                handleRequest(*stream, *request);
                readRequest(stream, buffer, std::make_shared<http::request<http::string_body>>(), self);
            });
    }

    /**
     * @brief 处理请求
     * @param stream HTTP流
     * @param request 请求对象
     */
    template <typename Stream>
    void handleRequest(Stream& stream, const http::request<http::string_body>& request) {
        http::response<http::string_body> response;
        response.version(request.version());
        response.keep_alive(false);

        // 查找处理函数
        std::string key = std::string(request.method_string()) + " " + std::string(request.target());
        auto it = handlers_.find(key);

        if (it != handlers_.end()) {
            try {
                // 调用处理函数
                it->second(request, response);
            } catch (const std::exception& e) {
                std::cerr << "Error handling request: " << e.what() << std::endl;
                response.result(http::status::internal_server_error);
                response.set(http::field::content_type, "application/json");
                response.body() = R"({
                    "error": "INTERNAL_SERVER_ERROR",
                    "message": "An internal server error occurred"
                })";
            }
        } else {
            // 未找到处理函数
            response.result(http::status::not_found);
            response.set(http::field::content_type, "application/json");
            response.body() = R"({
                "error": "NOT_FOUND",
                "message": "The requested resource was not found"
            })";
        }

        // 设置Content-Length
        response.prepare_payload();

        // 发送响应
        http::async_write(stream, response, 
            [this](beast::error_code ec, std::size_t bytes_transferred) {
                boost::ignore_unused(bytes_transferred);
                if (ec) {
                    std::cerr << "Error writing response: " << ec.message() << std::endl;
                }
            });
    }

private:
    asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    std::unordered_map<std::string, RequestHandler> handlers_; // 处理函数映射
};

} // namespace server

#endif // HTTP_SERVER_H