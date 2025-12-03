#ifndef JOB_SERVICE_TASK_FACTORY_H
#define JOB_SERVICE_TASK_FACTORY_H

#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "job.h"

namespace job_service {

// 任务接口
class Task {
public:
    virtual ~Task() = default;
    virtual nlohmann::json execute(JobPtr job) = 0;
};

using TaskPtr = std::unique_ptr<Task>;
using TaskCreator = std::function<TaskPtr()>;

// 任务工厂：负责创建不同类型的任务
class TaskFactory {
private:
    std::unordered_map<JobType, TaskCreator> creators_;
    mutable std::mutex mutex_;

public:
    // 注册任务类型
    void register_task_type(const JobType& type, TaskCreator creator);
    
    // 创建任务实例
    TaskPtr create_task(const JobType& type) const;
    
    // 检查任务类型是否存在
    bool has_task_type(const JobType& type) const;
    
    // 获取所有支持的任务类型
    std::vector<JobType> get_supported_types() const;
};

// 任务不存在异常
class TaskNotFoundException : public std::runtime_error {
public:
    explicit TaskNotFoundException(const JobType& type);
};

// 任务参数无效异常
class InvalidTaskParametersException : public std::runtime_error {
public:
    explicit InvalidTaskParametersException(const std::string& message);
};

// 注册任务类型的辅助函数
#define REGISTER_TASK(FACTORY, TYPE, TASK_CLASS) \
    FACTORY->register_task_type(TYPE, []() -> TaskPtr { return std::make_unique<TASK_CLASS>(); })

} // namespace job_service

#endif // JOB_SERVICE_TASK_FACTORY_H
