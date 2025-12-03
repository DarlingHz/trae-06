#ifndef JOB_SERVICE_STORAGE_H
#define JOB_SERVICE_STORAGE_H

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <nlohmann/json.hpp>
#include "job.h"

namespace job_service {

// 存储接口
class Storage {
public:
    virtual ~Storage() = default;
    
    // 保存单个任务
    virtual bool save_job(const JobPtr& job) = 0;
    
    // 更新任务
    virtual bool update_job(const JobPtr& job) = 0;
    
    // 根据ID获取任务
    virtual std::optional<JobPtr> get_job(const std::string& job_id) = 0;
    
    // 查询任务列表
    virtual std::vector<JobPtr> get_jobs(
        const std::optional<JobStatus>& status = std::nullopt,
        const std::optional<JobType>& type = std::nullopt,
        size_t limit = 20,
        size_t offset = 0
    ) = 0;
    
    // 获取所有任务
    virtual std::vector<JobPtr> get_all_jobs() = 0;
    
    // 删除任务
    virtual bool delete_job(const std::string& job_id) = 0;
    
    // 初始化存储
    virtual bool init() = 0;
};

// 文件存储实现（JSON格式）
class FileStorage : public Storage {
private:
    std::string storage_path_;
    mutable std::mutex mutex_;
    
    // 获取存储文件路径
    std::string get_file_path() const;
    
    // 加载所有任务
    std::vector<JobPtr> load_all_jobs() const;
    
    // 保存所有任务
    bool save_all_jobs(const std::vector<JobPtr>& jobs) const;

public:
    explicit FileStorage(const std::string& storage_path);
    
    bool save_job(const JobPtr& job) override;
    bool update_job(const JobPtr& job) override;
    std::optional<JobPtr> get_job(const std::string& job_id) override;
    std::vector<JobPtr> get_jobs(
        const std::optional<JobStatus>& status = std::nullopt,
        const std::optional<JobType>& type = std::nullopt,
        size_t limit = 20,
        size_t offset = 0
    ) override;
    std::vector<JobPtr> get_all_jobs() override;
    bool delete_job(const std::string& job_id) override;
    bool init() override;
};

// 创建存储实例
std::shared_ptr<Storage> create_storage(const std::string& type, const std::string& path);

} // namespace job_service

#endif // JOB_SERVICE_STORAGE_H
