#ifndef JOB_SERVICE_JOB_H
#define JOB_SERVICE_JOB_H

#include <string>
#include <chrono>
#include <optional>
#include <nlohmann/json.hpp>
#include <memory>

namespace job_service {

// 任务状态枚举
enum class JobStatus {
    QUEUED,
    RUNNING,
    DONE,
    FAILED,
    CANCELED
};

// 转换JobStatus到字符串
std::string job_status_to_string(JobStatus status);

// 从字符串转换到JobStatus
std::optional<JobStatus> string_to_job_status(const std::string& str);

// 任务类型
typedef std::string JobType;

// 任务优先级类型
typedef int JobPriority;

// 任务类
class Job {
private:
    std::string job_id_;
    JobType type_;
    nlohmann::json payload_;
    JobPriority priority_;
    JobStatus status_;
    std::chrono::system_clock::time_point created_at_;
    std::optional<std::chrono::system_clock::time_point> started_at_;
    std::optional<std::chrono::system_clock::time_point> finished_at_;
    std::optional<nlohmann::json> result_;
    std::optional<std::string> error_;
    bool cancel_requested_;

public:
    // 构造函数
    Job(const std::string& job_id, const JobType& type, const nlohmann::json& payload, JobPriority priority);
    
    // 禁止拷贝构造和拷贝赋值
    Job(const Job&) = delete;
    Job& operator=(const Job&) = delete;
    
    // 允许移动构造和移动赋值
    Job(Job&&) = default;
    Job& operator=(Job&&) = default;
    
    // 获取Job ID
    const std::string& get_job_id() const;
    
    // 获取任务类型
    const JobType& get_type() const;
    
    // 获取任务参数
    const nlohmann::json& get_payload() const;
    
    // 获取优先级
    JobPriority get_priority() const;
    
    // 获取状态
    JobStatus get_status() const;
    
    // 设置状态
    void set_status(JobStatus status);
    
    // 获取创建时间
    const std::chrono::system_clock::time_point& get_created_at() const;
    
    // 获取开始时间
    const std::optional<std::chrono::system_clock::time_point>& get_started_at() const;
    
    // 设置开始时间
    void set_started_at(const std::chrono::system_clock::time_point& time);
    
    // 获取完成时间
    const std::optional<std::chrono::system_clock::time_point>& get_finished_at() const;
    
    // 设置完成时间
    void set_finished_at(const std::chrono::system_clock::time_point& time);
    
    // 获取结果
    const std::optional<nlohmann::json>& get_result() const;
    
    // 设置结果
    void set_result(const nlohmann::json& result);
    
    // 获取错误信息
    const std::optional<std::string>& get_error() const;
    
    // 设置错误信息
    void set_error(const std::string& error);
    
    // 请求取消
    void request_cancel();
    
    // 检查是否被取消
    bool is_cancel_requested() const;
    
    // 转换为JSON
    nlohmann::json to_json() const;
    
    // 从JSON创建Job
    static Job from_json(const nlohmann::json& json);
};

using JobPtr = std::shared_ptr<Job>;

} // namespace job_service

#endif // JOB_SERVICE_JOB_H
