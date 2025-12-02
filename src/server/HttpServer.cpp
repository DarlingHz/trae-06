#include "server/HttpServer.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>

namespace server {

HttpServer::HttpServer(asio::io_context& io_context, const std::string& address, const std::string& port)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(asio::ip::make_address(address), static_cast<unsigned short>(std::stoi(port)))) {
}

void HttpServer::registerHandler(const std::string& method, const std::string& path, RequestHandler handler) {
    std::string key = method + " " + path;
    handlers_[key] = std::move(handler);
}

void HttpServer::start() {
    accept();
    std::cout << "Server started successfully" << std::endl;
}

void HttpServer::accept() {
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            if (ec) {
                std::cerr << "Error accepting connection: " << ec.message() << std::endl;
            } else {
                // 处理新连接
                handleConnection(std::move(socket));
            }
            // 继续接受新连接
            accept();
        });
}

void HttpServer::handleConnection(tcp::socket socket) {
    // 创建 HTTP 流
    auto stream = std::make_shared<beast::tcp_stream>(std::move(socket));
    // 设置超时
    stream->expires_after(std::chrono::seconds(30));
    // 创建缓冲区
    auto buffer = std::make_shared<beast::flat_buffer>();
    // 创建请求对象
    auto request = std::make_shared<http::request<http::string_body>>();
    // 读取请求
    readRequest(stream, buffer, request, stream);
}

} // namespace server