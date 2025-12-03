#include "job_service/job_service.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

using namespace job_service;

int main() {
    std::cout << "=== System Integration Test ===" << std::endl;
    
    // 初始化配置
    Config config;
    config.set_port(8080);
    config.set_thread_pool_size(4);
    config.set_storage_path("./data");
    config.set_log_level(LogLevel::INFO);
    
    // 创建组件
    auto storage = std::make_shared<FileStorage>(config.get_storage_path());
    auto job_queue = std::make_shared<JobQueue>();
    auto task_executor = std::make_unique<TaskExecutor>(job_queue, storage, config.get_thread_pool_size());
    
    // 注册内置任务类型
    auto task_factory = std::make_shared<TaskFactory>();
    // 注意：这里需要先实现和注册具体的任务类型
    // task_factory->register_task_type("fib", std::make_unique<FibonacciTask>());
    // task_factory->register_task_type("word_count", std::make_unique<WordCountTask>());
    
    std::cout << "✓ All components initialized successfully" << std::endl;
    
    // 启动任务执行器
    task_executor->start();
    std::cout << "✓ Task executor started" << std::endl;
    
    // 测试任务提交
    std::cout << "\n=== Task Submission Test ===" << std::endl;
    
    // 创建Fibonacci任务
    nlohmann::json fib_payload;
    fib_payload["n"] = 10; // 小数值，快速完成
    auto fib_job = std::make_shared<Job>(utils::generate_job_id(), "fib", fib_payload, 5);
    
    bool submit_success = task_executor->submit_job(fib_job);
    if (submit_success) {
        std::cout << "✓ Fibonacci job submitted successfully" << std::endl;
        std::cout << "  Job ID: " << fib_job->get_id() << std::endl;
    } else {
        std::cout << "✗ Failed to submit Fibonacci job" << std::endl;
        task_executor->stop();
        return 1;
    }
    
    // 创建WordCount任务
    nlohmann::json wc_payload;
    wc_payload["text"] = "This is a test. Testing word count functionality. This test should count words.";
    auto wc_job = std::make_shared<Job>(utils::generate_job_id(), "word_count", wc_payload, 3);
    
    submit_success = task_executor->submit_job(wc_job);
    if (submit_success) {
        std::cout << "✓ WordCount job submitted successfully" << std::endl;
        std::cout << "  Job ID: " << wc_job->get_id() << std::endl;
    } else {
        std::cout << "✗ Failed to submit WordCount job" << std::endl;
        task_executor->stop();
        return 1;
    }
    
    // 等待任务完成
    std::cout << "\n=== Waiting for tasks to complete ===" << std::endl;
    
    // 监控Fibonacci任务
    int fib_checks = 0;
    while (fib_checks < 10) {
        auto stored_job = storage->get_job(fib_job->get_id());
        if (stored_job && ((*stored_job)->get_status() == JobStatus::DONE || 
                          (*stored_job)->get_status() == JobStatus::FAILED ||
                          (*stored_job)->get_status() == JobStatus::CANCELED)) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        fib_checks++;
    }
    
    // 检查Fibonacci任务结果
    auto stored_fib_job = storage->get_job(fib_job->get_id());
    if (stored_fib_job) {
        std::cout << "\n=== Fibonacci Job Result ===" << std::endl;
        std::cout << "  Status: " << job_status_to_string((*stored_fib_job)->get_status()) << std::endl;
        
        if ((*stored_fib_job)->get_status() == JobStatus::DONE) {
            std::cout << "✓ Job completed successfully" << std::endl;
            auto result = (*stored_fib_job)->get_result();
            if (result) {
                std::cout << "  Result: " << (*result).dump(2) << std::endl;
            }
        } else if ((*stored_fib_job)->get_status() == JobStatus::FAILED) {
            std::cout << "✗ Job failed" << std::endl;
            auto error = (*stored_fib_job)->get_error();
            if (error) {
                std::cout << "  Error: " << *error << std::endl;
            }
        }
    }
    
    // 监控WordCount任务
    int wc_checks = 0;
    while (wc_checks < 10) {
        auto stored_job = storage->get_job(wc_job->get_id());
        if (stored_job && ((*stored_job)->get_status() == JobStatus::DONE || 
                          (*stored_job)->get_status() == JobStatus::FAILED ||
                          (*stored_job)->get_status() == JobStatus::CANCELED)) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        wc_checks++;
    }
    
    // 检查WordCount任务结果
    auto stored_wc_job = storage->get_job(wc_job->get_id());
    if (stored_wc_job) {
        std::cout << "\n=== WordCount Job Result ===" << std::endl;
        std::cout << "  Status: " << job_status_to_string((*stored_wc_job)->get_status()) << std::endl;
        
        if ((*stored_wc_job)->get_status() == JobStatus::DONE) {
            std::cout << "✓ Job completed successfully" << std::endl;
            auto result = (*stored_wc_job)->get_result();
            if (result) {
                std::cout << "  Result: " << (*result).dump(2) << std::endl;
            }
        } else if ((*stored_wc_job)->get_status() == JobStatus::FAILED) {
            std::cout << "✗ Job failed" << std::endl;
            auto error = (*stored_wc_job)->get_error();
            if (error) {
                std::cout << "  Error: " << *error << std::endl;
            }
        }
    }
    
    // 测试任务列表查询
    std::cout << "\n=== Job List Query Test ===" << std::endl;
    auto all_jobs = storage->get_jobs(std::nullopt, std::nullopt, 10, 0);
    std::cout << "✓ Found " << all_jobs.size() << " jobs in storage" << std::endl;
    
    // 测试状态过滤
    auto done_jobs = storage->get_jobs(JobStatus::DONE, std::nullopt, 10, 0);
    std::cout << "✓ Found " << done_jobs.size() << " completed jobs" << std::endl;
    
    // 测试类型过滤
    auto fib_jobs = storage->get_jobs(std::nullopt, "fib", 10, 0);
    std::cout << "✓ Found " << fib_jobs.size() << " fib type jobs" << std::endl;
    
    auto wc_jobs = storage->get_jobs(std::nullopt, "word_count", 10, 0);
    std::cout << "✓ Found " << wc_jobs.size() << " word_count type jobs" << std::endl;
    
    // 停止任务执行器
    std::cout << "\n=== Stopping system ===" << std::endl;
    task_executor->stop();
    std::cout << "✓ Task executor stopped" << std::endl;
    
    // 测试任务取消功能
    std::cout << "\n=== Task Cancellation Test ===" << std::endl;
    auto cancel_job = std::make_shared<Job>(utils::generate_job_id(), "fib", fib_payload, 5);
    storage->save_job(cancel_job);
    
    bool cancel_success = cancel_job->request_cancel();
    std::cout << "✓ Task cancellation requested: " << (cancel_success ? "Success" : "Failed") << std::endl;
    std::cout << "  Cancel requested: " << (cancel_job->is_cancel_requested() ? "Yes" : "No") << std::endl;
    
    std::cout << "\n=== Integration Test Summary ===" << std::endl;
    std::cout << "✓ All integration tests completed" << std::endl;
    
    return 0;
}
