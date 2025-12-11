#include "job_service.h"
#include "job.h"
#include "log.h"
#include <algorithm>

namespace recruitment {

// JobService 实现

JobService::JobService(std::shared_ptr<JobDAO> job_dao)
    : job_dao_(std::move(job_dao)) {
    LOG_DEBUG("JobService initialized");
}

// JobServiceImpl 实现

JobServiceImpl::JobServiceImpl(std::shared_ptr<JobDAO> job_dao)
    : JobService(std::move(job_dao)) {
    LOG_DEBUG("JobServiceImpl initialized");
}

long long JobServiceImpl::createJob(const Job& job) {
    LOG_DEBUG("Creating job: " + job.getTitle() + " for company ID: " + std::to_string(job.getCompanyId()));

    // 验证职位标题是否为空
    if (job.getTitle().empty()) {
        LOG_ERROR("Job title cannot be empty");
        throw std::invalid_argument("Job title cannot be empty");
    }

    // 验证公司ID是否有效
    if (job.getCompanyId() <= 0) {
        LOG_ERROR("Invalid company ID: " + std::to_string(job.getCompanyId()));
        throw std::invalid_argument("Invalid company ID");
    }

    try {
        long long job_id = job_dao_->create(job);
        LOG_INFO("Job created successfully with ID: " + std::to_string(job_id));
        return job_id;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create job: " + std::string(e.what()));
        throw;
    }
}

std::optional<Job> JobServiceImpl::getJobById(long long id) {
    LOG_DEBUG("Getting job by ID: " + std::to_string(id));

    try {
        std::optional<Job> job = job_dao_->getById(id);
        if (job) {
            LOG_DEBUG("Job found: " + job->getTitle());
        } else {
            LOG_DEBUG("Job not found with ID: " + std::to_string(id));
        }
        return job;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get job by ID: " + std::string(e.what()));
        throw;
    }
}

bool JobServiceImpl::updateJob(const Job& job) {
    LOG_DEBUG("Updating job: ID " + std::to_string(job.getId()));

    // 验证职位ID是否有效
    if (job.getId() <= 0) {
        LOG_ERROR("Invalid job ID: " + std::to_string(job.getId()));
        throw std::invalid_argument("Invalid job ID");
    }

    // 验证职位标题是否为空
    if (job.getTitle().empty()) {
        LOG_ERROR("Job title cannot be empty");
        throw std::invalid_argument("Job title cannot be empty");
    }

    // 验证公司ID是否有效
    if (job.getCompanyId() <= 0) {
        LOG_ERROR("Invalid company ID: " + std::to_string(job.getCompanyId()));
        throw std::invalid_argument("Invalid company ID");
    }

    try {
        bool success = job_dao_->update(job);
        if (success) {
            LOG_INFO("Job updated successfully: ID " + std::to_string(job.getId()));
        } else {
            LOG_DEBUG("Job not found for update: ID " + std::to_string(job.getId()));
        }
        return success;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update job: " + std::string(e.what()));
        throw;
    }
}

bool JobServiceImpl::deleteJobById(long long id) {
    LOG_DEBUG("Deleting job by ID: " + std::to_string(id));

    // 验证职位ID是否有效
    if (id <= 0) {
        LOG_ERROR("Invalid job ID: " + std::to_string(id));
        throw std::invalid_argument("Invalid job ID");
    }

    try {
        bool success = job_dao_->deleteById(id);
        if (success) {
            LOG_INFO("Job deleted successfully: ID " + std::to_string(id));
        } else {
            LOG_DEBUG("Job not found for deletion: ID " + std::to_string(id));
        }
        return success;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to delete job by ID: " + std::string(e.what()));
        throw;
    }
}

std::vector<Job> JobServiceImpl::getAllJobs(int page, int page_size) {
    LOG_DEBUG("Getting all jobs, page: " + std::to_string(page) + ", page size: " + std::to_string(page_size));

    // 验证分页参数
    if (page <= 0) {
        LOG_WARN("Invalid page number, using default: 1");
        page = 1;
    }

    if (page_size <= 0 || page_size > 100) {
        LOG_WARN("Invalid page size, using default: 20");
        page_size = 20;
    }

    try {
        std::vector<Job> jobs = job_dao_->getAll();

        // 分页处理
        size_t start_index = (page - 1) * page_size;
        size_t end_index = start_index + page_size;

        if (start_index >= jobs.size()) {
            LOG_DEBUG("No jobs found for the specified page");
            return {};
        }

        if (end_index > jobs.size()) {
            end_index = jobs.size();
        }

        std::vector<Job> paginated_jobs(jobs.begin() + start_index, jobs.begin() + end_index);
        LOG_DEBUG("Found " + std::to_string(paginated_jobs.size()) + " jobs for page " + std::to_string(page));
        return paginated_jobs;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get all jobs: " + std::string(e.what()));
        throw;
    }
}

std::vector<Job> JobServiceImpl::findJobsByCondition(const std::map<std::string, std::string>& conditions, int page, int page_size) {
    LOG_DEBUG("Finding jobs by condition");

    // 验证分页参数
    if (page <= 0) {
        LOG_WARN("Invalid page number, using default: 1");
        page = 1;
    }

    if (page_size <= 0 || page_size > 100) {
        LOG_WARN("Invalid page size, using default: 20");
        page_size = 20;
    }

    // 提取查询条件
    std::optional<long long> company_id_opt;
    auto company_id_it = conditions.find("company_id");
    if (company_id_it != conditions.end()) {
        try {
            company_id_opt = std::stoll(company_id_it->second);
        } catch (const std::exception& e) {
            LOG_ERROR("Invalid company_id: " + company_id_it->second);
            company_id_opt = std::nullopt;
        }
    }

    std::optional<std::string> location_opt;
    auto location_it = conditions.find("location");
    if (location_it != conditions.end()) {
        location_opt = location_it->second;
    }

    std::optional<std::string> required_skills_opt;
    auto required_skills_it = conditions.find("required_skills");
    if (required_skills_it != conditions.end()) {
        required_skills_opt = required_skills_it->second;
    }

    std::optional<bool> is_open_opt;
    auto is_open_it = conditions.find("is_open");
    if (is_open_it != conditions.end()) {
        is_open_opt = (is_open_it->second == "true" || is_open_it->second == "1");
    }

    try {
        std::vector<Job> jobs = job_dao_->findByCondition(company_id_opt, location_opt, required_skills_opt, is_open_opt, page, page_size, "created_at", "DESC");
        LOG_DEBUG("Found " + std::to_string(jobs.size()) + " jobs matching condition");
        return jobs;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to find jobs by condition: " + std::string(e.what()));
        throw;
    }
}

bool JobServiceImpl::openJob(long long id) {
    LOG_DEBUG("Opening job: ID " + std::to_string(id));

    // 验证职位ID是否有效
    if (id <= 0) {
        LOG_ERROR("Invalid job ID: " + std::to_string(id));
        throw std::invalid_argument("Invalid job ID");
    }

    try {
        // 获取职位信息
        std::optional<Job> job_opt = job_dao_->getById(id);
        if (!job_opt) {
            LOG_DEBUG("Job not found for opening: ID " + std::to_string(id));
            return false;
        }

        Job job = job_opt.value();
        if (job.isOpen()) {
            LOG_DEBUG("Job is already open: ID " + std::to_string(id));
            return true; // 已经开放，视为成功
        }

        // 更新职位状态为开放
        job.setIsOpen(true);
        bool success = job_dao_->update(job);
        if (success) {
            LOG_INFO("Job opened successfully: ID " + std::to_string(id));
        }
        return success;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to open job: " + std::string(e.what()));
        throw;
    }
}

bool JobServiceImpl::closeJob(long long id) {
    LOG_DEBUG("Closing job: ID " + std::to_string(id));

    // 验证职位ID是否有效
    if (id <= 0) {
        LOG_ERROR("Invalid job ID: " + std::to_string(id));
        throw std::invalid_argument("Invalid job ID");
    }

    try {
        // 获取职位信息
        std::optional<Job> job_opt = job_dao_->getById(id);
        if (!job_opt) {
            LOG_DEBUG("Job not found for closing: ID " + std::to_string(id));
            return false;
        }

        Job job = job_opt.value();
        if (!job.isOpen()) {
            LOG_DEBUG("Job is already closed: ID " + std::to_string(id));
            return true; // 已经关闭，视为成功
        }

        // 更新职位状态为关闭
        job.setIsOpen(false);
        bool success = job_dao_->update(job);
        if (success) {
            LOG_INFO("Job closed successfully: ID " + std::to_string(id));
        }
        return success;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to close job: " + std::string(e.what()));
        throw;
    }
}

int JobServiceImpl::getJobCount(const std::optional<long long>& company_id,
                             const std::optional<std::string>& location,
                             const std::optional<std::string>& required_skills,
                             const std::optional<bool>& is_open) {
    LOG_DEBUG("Getting job count by condition");

    try {
        int count = job_dao_->getJobCount(company_id, location, required_skills, is_open);
        LOG_DEBUG("Found " + std::to_string(count) + " jobs matching condition");
        return count;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get job count by condition: " + std::string(e.what()));
        throw;
    }
}

} // namespace recruitment