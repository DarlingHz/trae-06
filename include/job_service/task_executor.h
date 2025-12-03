#ifndef JOB_SERVICE_TASK_EXECUTOR_H
#define JOB_SERVICE_TASK_EXECUTOR_H

#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <memory>
#include "job.h"
#include "job_queue.h"
#include "task_factory.h"

namespace job_service {

class Storage;

// 任务执行器（线程池）
class TaskExecutor {
private:
    std::vector<std::thread> workers_;
    std::unique_ptr<JobQueue> job_queue_;
    std::shared_ptr<TaskFactory> task_factory_;
    std::shared_ptr<Storage> storage_;
    std::atomic<bool> running_;
    size_t thread_count_;

    // 工作线程函数
    void worker_loop();
    
    // 执行单个任务
    void execute_job(JobPtr job);

public:
    // 构造函数
    TaskExecutor(size_t thread_count, std::shared_ptr<TaskFactory> task_factory, std::shared_ptr<Storage> storage);
    
    // 析构函数
    ~TaskExecutor();
    
    // 禁止拷贝
    TaskExecutor(const TaskExecutor&) = delete;
    TaskExecutor& operator=(const TaskExecutor&) = delete;
    
    // 移动构造（禁用，因为包含线程）
    TaskExecutor(TaskExecutor&&) = delete;
    TaskExecutor& operator=(TaskExecutor&&) = delete;
    
    // 启动执行器
    void start();
    
    // 停止执行器
    void stop();
    
    // 提交任务
    bool submit_job(JobPtr job);
    
    // 获取队列大小
    size_t queue_size() const;
    
    // 获取运行中的线程数
    size_t thread_count() const;
};

} // namespace job_service

#endif // JOB_SERVICE_TASK_EXECUTOR_H
