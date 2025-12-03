#ifndef JOB_SERVICE_JOB_QUEUE_H
#define JOB_SERVICE_JOB_QUEUE_H

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include "job.h"

namespace job_service {

// 比较器：优先级高的任务先执行
struct JobPriorityComparator {
    bool operator()(const JobPtr& a, const JobPtr& b) const {
        return a->get_priority() < b->get_priority();
    }
};

// 带优先级的线程安全任务队列
class JobQueue {
private:
    std::priority_queue<JobPtr, std::vector<JobPtr>, JobPriorityComparator> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_;
    size_t max_size_;

public:
    // 构造函数
    explicit JobQueue(size_t max_size = 1000);
    
    // 禁止拷贝
    JobQueue(const JobQueue&) = delete;
    JobQueue& operator=(const JobQueue&) = delete;
    
    // 移动构造
    JobQueue(JobQueue&&) = default;
    JobQueue& operator=(JobQueue&&) = default;
    
    // 入队
    bool enqueue(JobPtr job);
    
    // 出队（阻塞等待直到有任务或队列停止）
    std::optional<JobPtr> dequeue();
    
    // 尝试出队（非阻塞）
    std::optional<JobPtr> try_dequeue();
    
    // 获取队列大小
    size_t size() const;
    
    // 检查队列是否为空
    bool empty() const;
    
    // 停止队列，唤醒所有等待的线程
    void stop();
    
    // 检查队列是否已停止
    bool is_stopped() const;
    
    // 清除队列
    void clear();
    
    // 获取所有任务（用于持久化）
    std::vector<JobPtr> get_all_jobs() const;
};

} // namespace job_service

#endif // JOB_SERVICE_JOB_QUEUE_H
