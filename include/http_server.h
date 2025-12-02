#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <future>

namespace recruitment {

/**
 * @brief HTTP请求方法枚举
 */
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS
};

/**
 * @brief HTTP请求数据结构
 */
struct HttpRequest {
    HttpMethod method; ///< 请求方法
    std::string path; ///< 请求路径
    std::unordered_map<std::string, std::string> headers; ///< 请求头部
    std::unordered_map<std::string, std::string> query_params; ///< 查询参数
    std::string body; ///< 请求体
};

/**
 * @brief HTTP响应数据结构
 */
struct HttpResponse {
    int status_code; ///< 状态码
    std::unordered_map<std::string, std::string> headers; ///< 响应头部
    std::string body; ///< 响应体

    /**
     * @brief 默认构造函数
     */
    HttpResponse() : status_code(200) {
        headers["Content-Type"] = "application/json; charset=utf-8";
    }

    /**
     * @brief 构造函数
     * @param status_code 状态码
     * @param body 响应体
     */
    HttpResponse(int status_code, const std::string& body) : status_code(status_code), body(body) {
        headers["Content-Type"] = "application/json; charset=utf-8";
    }
};

/**
 * @brief HTTP请求处理函数类型
 */
typedef std::function<HttpResponse(const HttpRequest&)> HttpHandler;

/**
 * @brief 线程池类
 */
class ThreadPool {
public:
    /**
     * @brief 构造函数
     * @param thread_count 线程数量
     */
    explicit ThreadPool(int thread_count = std::thread::hardware_concurrency());

    /**
     * @brief 析构函数
     */
    ~ThreadPool();

    /**
     * @brief 禁止拷贝构造
     */
    ThreadPool(const ThreadPool&) = delete;

    /**
     * @brief 禁止赋值操作
     */
    ThreadPool& operator=(const ThreadPool&) = delete;

    /**
     * @brief 提交任务
     * @tparam F 任务函数类型
     * @tparam Args 任务函数参数类型
     * @param f 任务函数
     * @param args 任务函数参数
     * @return 任务执行结果的未来对象
     */
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            // 如果线程池已经停止，抛出异常
            if (stop_) {
                throw std::runtime_error("ThreadPool is stopped");
            }

            // 添加任务到队列
            tasks_.emplace([task]() { (*task)(); });
        }

        // 通知一个等待的线程
        condition_.notify_one();
        return res;
    }

private:
    /**
     * @brief 工作线程函数
     */
    void worker();

    std::vector<std::thread> workers_; ///< 工作线程列表
    std::queue<std::function<void()>> tasks_; ///< 任务队列
    std::mutex queue_mutex_; ///< 队列互斥锁
    std::condition_variable condition_; ///< 条件变量
    bool stop_; ///< 线程池是否停止
};

/**
 * @brief HTTP服务器类
 */
class HttpServer {
public:
    /**
     * @brief 构造函数
     * @param port 监听端口
     * @param thread_count 线程数量
     */
    explicit HttpServer(int port = 8080, int thread_count = std::thread::hardware_concurrency());

    /**
     * @brief 析构函数
     */
    ~HttpServer();

    /**
     * @brief 禁止拷贝构造
     */
    HttpServer(const HttpServer&) = delete;

    /**
     * @brief 禁止赋值操作
     */
    HttpServer& operator=(const HttpServer&) = delete;

    /**
     * @brief 启动服务器
     * @return 启动成功返回true，否则返回false
     */
    bool start();

    /**
     * @brief 停止服务器
     */
    void stop();

    /**
     * @brief 注册路由
     * @param method HTTP请求方法
     * @param path 请求路径
     * @param handler 请求处理函数
     */
    void registerRoute(HttpMethod method, const std::string& path, const HttpHandler& handler);

    /**
     * @brief 注册GET路由
     * @param path 请求路径
     * @param handler 请求处理函数
     */
    void get(const std::string& path, const HttpHandler& handler) {
        registerRoute(HttpMethod::GET, path, handler);
    }

    /**
     * @brief 注册POST路由
     * @param path 请求路径
     * @param handler 请求处理函数
     */
    void post(const std::string& path, const HttpHandler& handler) {
        registerRoute(HttpMethod::POST, path, handler);
    }

    /**
     * @brief 注册PUT路由
     * @param path 请求路径
     * @param handler 请求处理函数
     */
    void put(const std::string& path, const HttpHandler& handler) {
        registerRoute(HttpMethod::PUT, path, handler);
    }

    /**
     * @brief 注册DELETE路由
     * @param path 请求路径
     * @param handler 请求处理函数
     */
    void del(const std::string& path, const HttpHandler& handler) {
        registerRoute(HttpMethod::DELETE, path, handler);
    }

private:
    /**
     * @brief 处理客户端连接
     * @param client_socket 客户端套接字
     */
    void handleClient(int client_socket);

    /**
     * @brief 解析HTTP请求
     * @param request_data 请求数据
     * @return 解析后的HTTP请求
     */
    HttpRequest parseRequest(const std::string& request_data);

    /**
     * @brief 格式化HTTP响应
     * @param response HTTP响应
     * @return 格式化后的响应数据
     */
    std::string formatResponse(const HttpResponse& response);

    /**
     * @brief 查找路由处理函数
     * @param method HTTP请求方法
     * @param path 请求路径
     * @return 路由处理函数，如果找不到返回空
     */
    std::optional<HttpHandler> findRoute(HttpMethod method, const std::string& path);

    int port_; ///< 监听端口
    int server_socket_; ///< 服务器套接字
    bool running_; ///< 服务器是否运行
    std::thread accept_thread_; ///< 接受连接线程
    std::shared_ptr<ThreadPool> thread_pool_; ///< 线程池

    // 路由表：方法 -> 路径 -> 处理函数
    std::unordered_map<HttpMethod, std::unordered_map<std::string, HttpHandler>> routes_;
    std::mutex routes_mutex_; ///< 路由表互斥锁

    /**
     * @brief HTTP方法转字符串
     * @param method HTTP方法
     * @return 方法字符串
     */
    std::string httpMethodToString(HttpMethod method);

    /**
     * @brief HTTP状态码转文本
     * @param status_code 状态码
     * @return 状态码文本
     */
    std::string getHttpStatusCodeText(int status_code);
};

} // namespace recruitment

#endif // HTTP_SERVER_H