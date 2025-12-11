#include "application_service.h"
#include "log.h"
#include <sstream>
#include <unordered_map>
#include <set>

namespace recruitment {

ApplicationServiceImpl::ApplicationServiceImpl(std::shared_ptr<ApplicationDAO> application_dao)
    : ApplicationService(std::move(application_dao)) {
    // 构造函数初始化
    LOG_INFO("ApplicationServiceImpl initialized");
}

long long ApplicationServiceImpl::createApplication(const Application& application) {
    try {
        // 参数验证
        if (application.getJobId() <= 0) {
            std::stringstream ss1;
            ss1 << "Invalid job ID: " << application.getJobId();
            LOG_ERROR(ss1.str());
            return -1;
        }
        if (application.getCandidateId() <= 0) {
            std::stringstream ss2;
            ss2 << "Invalid candidate ID: " << application.getCandidateId();
            LOG_ERROR(ss2.str());
            return -1;
        }
        if (application.getStatus().empty()) {
            LOG_ERROR("Application status cannot be empty");
            return -1;
        }

        // 调用DAO创建投递
        long long application_id = application_dao_->create(application);
        if (application_id > 0) {
            std::stringstream ss3;
            ss3 << "Application created successfully, ID: " << application_id;
            LOG_INFO(ss3.str());
            return application_id;
        } else {
            LOG_ERROR("Failed to create application");
            return -1;
        }
    } catch (const std::exception& e) {
        std::stringstream ss4;
        ss4 << "Exception occurred while creating application: " << e.what();
        LOG_ERROR(ss4.str());
        return -1;
    }
}

std::optional<Application> ApplicationServiceImpl::getApplicationById(long long id) {
    try {
        // 参数验证
        if (id <= 0) {
            std::stringstream ss1;
            ss1 << "Invalid application ID: " << id;
            LOG_ERROR(ss1.str());
            return std::nullopt;
        }

        // 调用DAO获取投递
        std::optional<Application> application = application_dao_->getById(id);
        if (application) {
            std::stringstream ss2;
            ss2 << "Application retrieved successfully, ID: " << id;
            LOG_INFO(ss2.str());
        } else {
            std::stringstream ss3;
            ss3 << "Application not found, ID: " << id;
            LOG_WARN(ss3.str());
        }
        return application;
    } catch (const std::exception& e) {
        std::stringstream ss4;
        ss4 << "Exception occurred while retrieving application: " << e.what();
        LOG_ERROR(ss4.str());
        return std::nullopt;
    }
}

bool ApplicationServiceImpl::updateApplication(const Application& application) {
    try {
        // 参数验证
        if (application.getId() <= 0) {
            std::stringstream ss1;
            ss1 << "Invalid application ID: " << application.getId();
            LOG_ERROR(ss1.str());
            return false;
        }
        if (application.getJobId() <= 0) {
            std::stringstream ss2;
            ss2 << "Invalid job ID: " << application.getJobId();
            LOG_ERROR(ss2.str());
            return false;
        }
        if (application.getCandidateId() <= 0) {
            std::stringstream ss3;
            ss3 << "Invalid candidate ID: " << application.getCandidateId();
            LOG_ERROR(ss3.str());
            return false;
        }
        if (application.getStatus().empty()) {
            LOG_ERROR("Application status cannot be empty");
            return false;
        }

        // 调用DAO更新投递
        bool success = application_dao_->update(application);
        if (success) {
            std::stringstream ss4;
            ss4 << "Application updated successfully, ID: " << application.getId();
            LOG_INFO(ss4.str());
        } else {
            std::stringstream ss5;
            ss5 << "Failed to update application, ID: " << application.getId();
            LOG_ERROR(ss5.str());
        }
        return success;
    } catch (const std::exception& e) {
        std::stringstream ss6;
        ss6 << "Exception occurred while updating application: " << e.what();
        LOG_ERROR(ss6.str());
        return false;
    }
}

bool ApplicationServiceImpl::deleteApplicationById(long long id) {
    try {
        // 参数验证
        if (id <= 0) {
            std::stringstream ss1;
            ss1 << "Invalid application ID: " << id;
            LOG_ERROR(ss1.str());
            return false;
        }

        // 调用DAO删除投递
        bool success = application_dao_->deleteById(id);
        if (success) {
            std::stringstream ss2;
            ss2 << "Application deleted successfully, ID: " << id;
            LOG_INFO(ss2.str());
        } else {
            std::stringstream ss3;
            ss3 << "Failed to delete application, ID: " << id;
            LOG_ERROR(ss3.str());
        }
        return success;
    } catch (const std::exception& e) {
        std::stringstream ss4;
        ss4 << "Exception occurred while deleting application: " << e.what();
        LOG_ERROR(ss4.str());
        return false;
    }
}

std::vector<Application> ApplicationServiceImpl::getAllApplications() {
    try {
        // 调用DAO获取所有投递
        std::vector<Application> applications = application_dao_->getAll();
        std::stringstream ss1;
        ss1 << "Retrieved all applications, count: " << applications.size();
        LOG_INFO(ss1.str());
        return applications;
    } catch (const std::exception& e) {
        std::stringstream ss2;
        ss2 << "Exception occurred while retrieving all applications: " << e.what();
        LOG_ERROR(ss2.str());
        return {};
    }
}

std::vector<Application> ApplicationServiceImpl::findApplicationsByCondition(
    const std::optional<long long>& job_id,
    const std::optional<long long>& candidate_id,
    const std::optional<std::string>& status,
    int page,
    int page_size) {
    try {
        // 参数验证
        if (page < 1) {
            std::stringstream ss1;
            ss1 << "Invalid page number: " << page << ", using default 1";
            LOG_WARN(ss1.str());
            page = 1;
        }
        if (page_size < 1 || page_size > 100) {
            std::stringstream ss2;
            ss2 << "Invalid page size: " << page_size << ", using default 10";
            LOG_WARN(ss2.str());
            page_size = 10;
        }

        // 构建查询条件日志
        std::ostringstream oss;
        oss << "Finding applications with condition: ";
        if (job_id) {
            oss << "job_id=" << *job_id << ", ";
        }
        if (candidate_id) {
            oss << "candidate_id=" << *candidate_id << ", ";
        }
        if (status) {
            oss << "status=" << *status << ", ";
        }
        oss << "page=" << page << ", page_size=" << page_size;
        LOG_INFO(oss.str());

        // 调用DAO查询投递
        std::vector<Application> applications = application_dao_->findByCondition(
            job_id, candidate_id, status, page, page_size);
        std::stringstream ss1;
        ss1 << "Found applications, count: " << applications.size();
        LOG_INFO(ss1.str());
        return applications;
    } catch (const std::exception& e) {
        std::stringstream ss2;
        ss2 << "Exception occurred while finding applications: " << e.what();
        LOG_ERROR(ss2.str());
        return {};
    }
}

bool ApplicationServiceImpl::updateApplicationStatus(long long application_id, const std::string& new_status) {
    try {
        // 参数验证
        if (application_id <= 0) {
            std::stringstream ss1;
            ss1 << "Invalid application ID: " << application_id;
            LOG_ERROR(ss1.str());
            return false;
        }
        if (new_status.empty()) {
            LOG_ERROR("New status cannot be empty");
            return false;
        }

        // 获取当前投递信息
        std::optional<Application> current_application = application_dao_->getById(application_id);
        if (!current_application) {
            std::stringstream ss2;
            ss2 << "Application not found, ID: " << application_id;
            LOG_ERROR(ss2.str());
            return false;
        }

        const std::string& old_status = current_application->getStatus();
        if (old_status == new_status) {
            std::stringstream ss3;
            ss3 << "Application status is already " << new_status << ", no update needed";
            LOG_WARN(ss3.str());
            return true;
        }

        // 验证状态流转是否合法
        if (!isStatusTransitionValid(old_status, new_status)) {
            std::stringstream ss4;
            ss4 << "Invalid status transition from " << old_status << " to " << new_status;
            LOG_ERROR(ss4.str());
            return false;
        }

        // 调用DAO更新状态
        bool success = application_dao_->updateStatus(application_id, new_status);
        if (success) {
            std::stringstream ss5;
            ss5 << "Application status updated successfully, ID: " << application_id 
                     << ", from " << old_status << " to " << new_status;
            LOG_INFO(ss5.str());
            
            // 添加状态变更历史
            bool history_success = application_dao_->addStatusHistory(application_id, old_status, new_status);
            if (history_success) {
                std::stringstream ss6;
                ss6 << "Application status history added successfully, ID: " << application_id;
                LOG_INFO(ss6.str());
            } else {
                std::stringstream ss7;
                ss7 << "Failed to add application status history, ID: " << application_id;
                LOG_WARN(ss7.str());
            }
        } else {
            std::stringstream ss8;
            ss8 << "Failed to update application status, ID: " << application_id;
            LOG_ERROR(ss8.str());
        }
        return success;
    } catch (const std::exception& e) {
        std::stringstream ss9;
        ss9 << "Exception occurred while updating application status: " << e.what();
        LOG_ERROR(ss9.str());
        return false;
    }
}

std::vector<ApplicationStatusHistory> ApplicationServiceImpl::getApplicationStatusHistory(long long application_id) {
    try {
        // 参数验证
        if (application_id <= 0) {
            std::stringstream ss1;
            ss1 << "Invalid application ID: " << application_id;
            LOG_ERROR(ss1.str());
            return {};
        }

        // 调用DAO获取状态变更历史
        std::vector<ApplicationStatusHistory> history = application_dao_->getStatusHistory(application_id);
        std::stringstream ss2;
        ss2 << "Retrieved application status history, ID: " << application_id << ", count: " << history.size();
        LOG_INFO(ss2.str());
        return history;
    } catch (const std::exception& e) {
        std::stringstream ss3;
        ss3 << "Exception occurred while retrieving application status history: " << e.what();
        LOG_ERROR(ss3.str());
        return {};
    }
}

bool ApplicationServiceImpl::isStatusTransitionValid(const std::string& old_status, const std::string& new_status) {
    // 定义合法的状态流转
    static const std::unordered_map<std::string, std::set<std::string>> valid_transitions = {
        {"applied", {"screening", "rejected"}},
        {"screening", {"interviewing", "rejected"}},
        {"interviewing", {"offered", "rejected"}},
        {"offered", {"accepted", "rejected"}},
        {"accepted", {"hired", "rejected"}},
        {"rejected", {}},
        {"hired", {}}
    };

    // 检查旧状态是否存在于映射中
    auto it = valid_transitions.find(old_status);
    if (it == valid_transitions.end()) {
        std::stringstream ss1;
        ss1 << "Invalid old status: " << old_status;
        LOG_ERROR(ss1.str());
        return false;
    }

    // 检查新状态是否存在于旧状态的合法流转集合中
    if (it->second.find(new_status) == it->second.end()) {
        std::stringstream ss2;
        ss2 << "Invalid transition from " << old_status << " to " << new_status;
        LOG_ERROR(ss2.str());
        return false;
    }

    return true;
}

} // namespace recruitment
