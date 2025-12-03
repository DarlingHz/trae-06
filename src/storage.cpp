#include "job_service/storage.h"
#include "job_service/utils.h"
#include "job_service/logging.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace job_service {

namespace fs = std::filesystem;

// FileStorage 实现
FileStorage::FileStorage(const std::string& storage_path)
    : storage_path_(storage_path) {
    // 创建存储目录
    if (!fs::exists(storage_path_)) {
        fs::create_directories(storage_path_);
    }
}

bool FileStorage::save_job(const std::shared_ptr<Job>& job) {
    try {
        std::string filename = storage_path_ + "/" + job->get_job_id() + ".json";
        std::ofstream file(filename);
        
        if (!file.is_open()) {
            return false;
        }
        
        nlohmann::json job_json = job->to_json();
        file << job_json.dump(2);
        
        return true;
    } catch (...) {
        return false;
    }
}

bool FileStorage::update_job(const std::shared_ptr<Job>& job) {
    return save_job(job);
}

std::optional<std::shared_ptr<Job>> FileStorage::get_job(const std::string& job_id) {
    try {
        std::string filename = storage_path_ + "/" + job_id + ".json";
        
        if (!fs::exists(filename)) {
            return std::nullopt;
        }
        
        std::ifstream file(filename);
        if (!file.is_open()) {
            return std::nullopt;
        }
        
        nlohmann::json job_json;
        file >> job_json;
        
        auto job = std::make_shared<Job>(Job::from_json(job_json));
        return job;
    } catch (...) {
        return std::nullopt;
    }
}

bool FileStorage::delete_job(const std::string& job_id) {
    try {
        std::string filename = storage_path_ + "/" + job_id + ".json";
        return fs::remove(filename);
    } catch (...) {
        return false;
    }
}

std::vector<std::shared_ptr<Job>> FileStorage::get_jobs(
    const std::optional<JobStatus>& status_filter,
    const std::optional<JobType>& type_filter,
    size_t limit,
    size_t offset) {
    try {
        std::vector<std::shared_ptr<Job>> jobs;
        
        // 加载所有任务
        auto all_jobs = load_all_jobs();
        
        // 应用过滤器
        for (const auto& job : all_jobs) {
            bool should_include = true;
            
            if (status_filter && job->get_status() != *status_filter) {
                should_include = false;
            }
            
            if (type_filter && job->get_type() != *type_filter) {
                should_include = false;
            }
            
            if (should_include) {
                jobs.push_back(job);
            }
        }
        
        // 按创建时间排序（降序）
        std::sort(jobs.begin(), jobs.end(), [](const std::shared_ptr<Job>& a, const std::shared_ptr<Job>& b) {
            return a->get_created_at() > b->get_created_at();
        });
        
        // 应用分页
        if (offset >= jobs.size()) {
            return {};
        }
        
        size_t take = std::min(limit, jobs.size() - offset);
        return std::vector<std::shared_ptr<Job>>(jobs.begin() + offset, jobs.begin() + offset + take);
    } catch (...) {
        // 查询失败，返回空列表
        return {};
    }
}


} // namespace job_service
