#include "job_service/job_queue.h"
#include <stdexcept>

namespace job_service {

JobQueue::JobQueue(size_t max_size)
    : stopped_(false),
      max_size_(max_size) {
}

bool JobQueue::enqueue(JobPtr job) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (stopped_) {
        return false;
    }
    
    if (queue_.size() >= max_size_) {
        return false;
    }
    
    queue_.push(std::move(job));
    cv_.notify_one();
    return true;
}

std::optional<JobPtr> JobQueue::dequeue() {
    std::unique_lock<std::mutex> lock(mutex_);
    
    cv_.wait(lock, [this] { return stopped_ || !queue_.empty(); });
    
    if (stopped_ || queue_.empty()) {
        return std::nullopt;
    }
    
    JobPtr job = std::move(queue_.top());
    queue_.pop();
    return job;
}

std::optional<JobPtr> JobQueue::try_dequeue() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (stopped_ || queue_.empty()) {
        return std::nullopt;
    }
    
    JobPtr job = std::move(queue_.top());
    queue_.pop();
    return job;
}

size_t JobQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

bool JobQueue::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

void JobQueue::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    stopped_ = true;
    cv_.notify_all();
}

bool JobQueue::is_stopped() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stopped_;
}

void JobQueue::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!queue_.empty()) {
        queue_.pop();
    }
}

std::vector<JobPtr> JobQueue::get_all_jobs() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<JobPtr> jobs;
    auto temp_queue = queue_;
    
    while (!temp_queue.empty()) {
        jobs.push_back(temp_queue.top());
        temp_queue.pop();
    }
    
    return jobs;
}

} // namespace job_service
