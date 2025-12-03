#include "job_service/task_executor.h"
#include "job_service/storage.h"
#include "job_service/logging.h"
#include <stdexcept>
#include <chrono>

namespace job_service {

TaskExecutor::TaskExecutor(size_t thread_count, std::shared_ptr<TaskFactory> task_factory, std::shared_ptr<Storage> storage)
    : job_queue_(std::make_unique<JobQueue>(1000)),
      task_factory_(std::move(task_factory)),
      storage_(std::move(storage)),
      running_(false),
      thread_count_(thread_count) {
}

TaskExecutor::~TaskExecutor() {
    stop();
}

void TaskExecutor::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    
    // 启动工作线程
    for (size_t i = 0; i < thread_count_; ++i) {
        workers_.emplace_back(&TaskExecutor::worker_loop, this);
    }
    
    global_logger.info("TaskExecutor started with ", thread_count_, " threads");
}

void TaskExecutor::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    job_queue_->stop();
    
    // 等待所有工作线程完成
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    workers_.clear();
    
    global_logger.info("TaskExecutor stopped");
}

bool TaskExecutor::submit_job(JobPtr job) {
    if (!running_) {
        return false;
    }
    
    // 保存到存储
    if (!storage_->save_job(job)) {
        global_logger.error("Failed to save job ", job->get_job_id());
        return false;
    }
    
    if (job_queue_->enqueue(std::move(job))) {
        global_logger.info("Job submitted to queue: ", job->get_job_id());
        return true;
    }
    
    global_logger.warn("Job queue is full, cannot submit job: ", job->get_job_id());
    return false;
}

size_t TaskExecutor::queue_size() const {
    return job_queue_->size();
}

size_t TaskExecutor::thread_count() const {
    return thread_count_;
}

void TaskExecutor::worker_loop() {
    while (running_) {
        auto job_opt = job_queue_->dequeue();
        
        if (!job_opt) {
            continue;
        }
        
        auto job = std::move(*job_opt);
        
        // 检查是否被取消
        if (job->is_cancel_requested()) {
            job->set_status(JobStatus::CANCELED);
            storage_->update_job(job);
            global_logger.info("Job canceled before execution: ", job->get_job_id());
            continue;
        }
        
        execute_job(std::move(job));
    }
}

void TaskExecutor::execute_job(JobPtr job) {
    try {
        global_logger.info("Starting job execution: ", job->get_job_id());
        
        // 更新任务状态为运行中
        job->set_status(JobStatus::RUNNING);
        job->set_started_at(std::chrono::system_clock::now());
        storage_->update_job(job);
        
        // 创建任务实例
        auto task = task_factory_->create_task(job->get_type());
        
        // 执行任务
        auto result = task->execute(job);
        
        // 检查是否被取消
        if (job->is_cancel_requested()) {
            job->set_status(JobStatus::CANCELED);
            job->set_error("Job was canceled during execution");
            global_logger.info("Job canceled during execution: ", job->get_job_id());
        } else {
            job->set_status(JobStatus::DONE);
            job->set_result(result);
            global_logger.info("Job execution completed: ", job->get_job_id());
        }
        
    } catch (const std::exception& e) {
        job->set_status(JobStatus::FAILED);
        job->set_error(std::string("Execution failed: ") + e.what());
        global_logger.error("Job execution failed ", job->get_job_id(), ": ", e.what());
    } catch (...) {
        job->set_status(JobStatus::FAILED);
        job->set_error("Unknown error occurred during execution");
        global_logger.error("Job execution failed with unknown error: ", job->get_job_id());
    }
    
    // 更新完成时间和存储
    job->set_finished_at(std::chrono::system_clock::now());
    storage_->update_job(job);
}

} // namespace job_service
