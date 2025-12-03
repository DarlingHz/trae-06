#include "job_service/job_service.h"
#include <iostream>
#include <signal.h>
#include <thread>
#include <chrono>

using namespace job_service;

// 全局标志，用于优雅退出
std::atomic_bool running = true;

// 信号处理函数
void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        std::cout << "\nReceived termination signal, shutting down gracefully..." << std::endl;
        running = false;
    }
}

// 注册信号处理
void register_signal_handlers() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}

int main(int argc, char* argv[]) {
    try {
        // 注册信号处理
        register_signal_handlers();
        
        // 初始化日志
        global_logger.set_log_level(LogLevel::INFO);
        global_logger.info("Starting Job Service...");
        
        // 加载配置
        Config config;
        std::string config_path = "config.json";
        if (argc > 1) {
            config_path = argv[1];
        }
        
        if (!config.load_from_file(config_path)) {
            global_logger.error("Failed to load config from ", config_path);
            global_logger.info("Using default configuration");
            // 使用默认配置
            config.set_port(8080);
            config.set_thread_pool_size(4);
            config.set_storage_path("./data");
            config.set_log_level(LogLevel::INFO);
        }
        
        // 验证配置
        if (!config.validate()) {
            global_logger.error("Invalid configuration");
            return 1;
        }
        
        // 配置日志级别
        global_logger.set_log_level(config.get_log_level());
        
        // 初始化存储
        global_logger.info("Initializing storage at ", config.get_storage_path());
        auto storage = std::make_shared<FileStorage>(config.get_storage_path());
        
        // 恢复之前的任务
        global_logger.info("Recovering jobs from storage...");
        auto jobs = storage->get_jobs(std::nullopt, std::nullopt, 1000, 0);
        
        size_t recovered_count = 0;
        size_t running_jobs_recovered = 0;
        
        for (const auto& job : jobs) {
            if (job->get_status() == JobStatus::RUNNING) {
                // 服务重启时，将之前运行中的任务标记为失败或重新排队
                // 这里选择标记为失败，并记录原因
                job->set_status(JobStatus::FAILED);
                job->set_error("Service restarted while job was running");
                storage->update_job(job);
                running_jobs_recovered++;
            }
            recovered_count++;
        }
        
        global_logger.info("Recovered ", recovered_count, " jobs from storage");
        if (running_jobs_recovered > 0) {
            global_logger.info(running_jobs_recovered, " running jobs were marked as failed due to service restart");
        }
        
        // 创建任务队列和执行器
        
        // 注册示例任务类型
        auto task_factory = std::make_shared<TaskFactory>();
        register_example_tasks(task_factory);
        
        auto task_executor = std::make_unique<TaskExecutor>(config.get_thread_pool_size(), task_factory, storage);
        
        // 创建并启动HTTP服务器
        global_logger.info("Starting HTTP server on port ", config.get_port());
        HttpServer server(config.get_port(), std::move(task_executor), storage);
        
        if (!server.start()) {
            global_logger.error("Failed to start HTTP server");
            return 1;
        }
        
        global_logger.info("Job Service started successfully on port ", config.get_port());
        global_logger.info("API endpoints available:");
        global_logger.info("  POST /api/jobs - Submit a new job");
        global_logger.info("  GET /api/jobs/{job_id} - Get job status and result");
        global_logger.info("  GET /api/jobs - List jobs with filters");
        global_logger.info("  POST /api/jobs/{job_id}/cancel - Cancel a job");
        global_logger.info("  GET /health - Health check");
        
        // 等待退出信号
        while (running && server.is_running()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        // 优雅关闭
        global_logger.info("Shutting down HTTP server...");
        server.stop();
        
        global_logger.info("Job Service stopped successfully");
        
        return 0;
        
    } catch (const std::exception& e) {
        global_logger.error("Fatal error: ", e.what());
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        global_logger.error("Unknown fatal error");
        std::cerr << "Unknown fatal error" << std::endl;
        return 1;
    }
}
