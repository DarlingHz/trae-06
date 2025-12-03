#include "router.h"
#include "response_util.h"
#include <iostream>
#include <stdexcept>

Router::Router(const std::string& base_url, uint16_t port)
    : base_url_(base_url), port_(port) {}

Router::~Router() {
    stop().wait();
}

std::string Router::get_base_url() const {
    return base_url_;
}

uint16_t Router::get_port() const {
    return port_;
}

void Router::set_auth_middleware(std::shared_ptr<AuthMiddleware> auth_middleware) {
    auth_middleware_ = auth_middleware;
}

void Router::set_user_controller(std::shared_ptr<UserController> user_controller) {
    user_controller_ = user_controller;
}

void Router::set_announcement_controller(std::shared_ptr<AnnouncementController> announcement_controller) {
    announcement_controller_ = announcement_controller;
}

void Router::setup_cors(http_request& message) {
    message.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    message.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, PUT, DELETE, OPTIONS"));
    message.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type, Authorization"));
    message.headers().add(U("Access-Control-Max-Age"), U("86400"));
}

void Router::handle_options(http_request message) {
    setup_cors(message);
    message.reply(status_codes::OK);
}

template <typename T>
void Router::apply_middleware(
    std::shared_ptr<T> listener,
    const std::vector<std::function<void(http_request, std::function<void()>)>>& middlewares,
    std::function<void(http_request)> handler
) {
    std::function<void(http_request)> current_handler = handler;
    
    // 反向应用中间件，使第一个中间件先执行
    for (auto it = middlewares.rbegin(); it != middlewares.rend(); ++it) {
        const auto& middleware = *it;
        auto next_handler = current_handler;
        current_handler = [middleware, next_handler](http_request message) {
            middleware(message, [next_handler, message]() mutable {
                next_handler(std::move(message));
            });
        };
    }
    
    // 设置最终处理函数
    *listener = *listener.support([current_handler](http_request message) mutable {
        current_handler(std::move(message));
    });
}

void Router::register_all_routes() {
    if (!auth_middleware_) {
        throw std::runtime_error("Auth middleware not set");
    }
    
    uri_builder base_uri(utility::conversions::to_string_t(base_url_));
    base_uri.set_port(port_);
    base_uri.append_path(U("/api/v1"));
    
    auto add_route = [this](http_listener listener) {
        auto shared_listener = std::make_shared<http_listener>(std::move(listener));
        listeners_.push_back(shared_listener);
    };
    
    // 注册用户控制器路由
    if (user_controller_) {
        user_controller_->register_routes(base_uri, add_route);
    }
    
    // 注册公告控制器路由
    if (announcement_controller_) {
        announcement_controller_->register_routes(base_uri, add_route);
    }
    
    // 为所有 listener 设置 CORS 和 OPTIONS 处理
    for (auto& listener : listeners_) {
        // 设置 OPTIONS 方法处理
        *listener = *listener.support(methods::OPTIONS, [this](http_request message) {
            this->handle_options(message);
        });
        
        // 应用中间件
        std::vector<std::function<void(http_request, std::function<void()>)>> middlewares = {
            [this](http_request message, std::function<void()> next) {
                this->setup_cors(message);
                next();
            }
        };
        
        // 需要认证的路由添加认证中间件
        auto path = listener->uri().path();
        if (path.find(U("/auth/login")) == std::string::npos &&
            path.find(U("/auth/refresh")) == std::string::npos &&
            path.find(U("/users")) == std::string::npos) {  // 用户注册不需要认证
            middlewares.push_back(std::bind(&AuthMiddleware::authenticate, auth_middleware_, std::placeholders::_1, std::placeholders::_2));
        }
        
        // 读取现有的处理函数并应用中间件
        auto existing_listener = std::make_shared<http_listener>(*listener);
        *listener = http_listener(listener->uri().to_string());
        
        // 重新注册所有方法的处理函数
        for (auto method : {methods::GET, methods::POST, methods::PUT, methods::DEL}) {
            if (existing_listener->supports(method)) {
                auto handler = [existing_listener, method](http_request message) {
                    existing_listener->handle(method, message);
                };
                
                auto wrapped_listener = std::make_shared<http_listener>(*listener);
                apply_middleware(wrapped_listener, middlewares, handler);
                *listener = *wrapped_listener;
            }
        }
    }
    
    std::cout << "All routes registered successfully" << std::endl;
}

pplx::task<void> Router::start() {
    std::cout << "Starting server on " << base_url_ << ":" << port_ << "/api/v1" << std::endl;
    
    std::vector<pplx::task<void>> start_tasks;
    for (auto& listener : listeners_) {
        start_tasks.push_back(listener->open());
    }
    
    return pplx::when_all(start_tasks.begin(), start_tasks.end()).then([this](pplx::task<void> t) {
        try {
            t.get();
            std::cout << "Server started successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to start server: " << e.what() << std::endl;
            throw;
        }
    });
}

pplx::task<void> Router::stop() {
    std::cout << "Stopping server..." << std::endl;
    
    std::vector<pplx::task<void>> stop_tasks;
    for (auto& listener : listeners_) {
        stop_tasks.push_back(listener->close());
    }
    
    return pplx::when_all(stop_tasks.begin(), stop_tasks.end()).then([this](pplx::task<void> t) {
        try {
            t.get();
            listeners_.clear();
            std::cout << "Server stopped successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error stopping server: " << e.what() << std::endl;
            throw;
        }
    });
}

// 显式实例化模板函数
template void Router::apply_middleware<http_listener>(
    std::shared_ptr<http_listener>,
    const std::vector<std::function<void(http_request, std::function<void()>)>>&,
    std::function<void(http_request)>);
