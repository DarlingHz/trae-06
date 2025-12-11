#include "candidate_service.h"
#include "log.h"
#include <sstream>

namespace recruitment {

CandidateServiceImpl::CandidateServiceImpl(std::shared_ptr<CandidateDAO> candidate_dao)
    : CandidateService(std::move(candidate_dao)) {
    // 构造函数初始化
    LOG_INFO("CandidateServiceImpl initialized");
}

long long CandidateServiceImpl::createCandidate(const Candidate& candidate) {
    try {
        // 参数验证
        if (candidate.getName().empty()) {
            LOG_ERROR("Candidate name cannot be empty");
            return -1;
        }
        if (candidate.getContact().empty()) {
            LOG_ERROR("Candidate contact cannot be empty");
            return -1;
        }

        // 调用DAO创建候选人
        long long candidate_id = candidate_dao_->create(candidate);
        if (candidate_id > 0) {
            std::stringstream ss;
            ss << "Candidate created successfully, ID: " << candidate_id;
            LOG_INFO(ss.str());
            return candidate_id;
        } else {
            LOG_ERROR("Failed to create candidate");
            return -1;
        }
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Exception occurred while creating candidate: " << e.what();
        LOG_ERROR(ss.str());
        return -1;
    }
}

std::optional<Candidate> CandidateServiceImpl::getCandidateById(long long id) {
    try {
        // 参数验证
        if (id <= 0) {
            std::stringstream ss;
            ss << "Invalid candidate ID: " << id;
            LOG_ERROR(ss.str());
            return std::nullopt;
        }

        // 调用DAO获取候选人
        std::optional<Candidate> candidate = candidate_dao_->getById(id);
        if (candidate) {
            std::stringstream ss;
            ss << "Candidate retrieved successfully, ID: " << id;
            LOG_INFO(ss.str());
        } else {
            std::stringstream ss;
            ss << "Candidate not found, ID: " << id;
            LOG_WARN(ss.str());
        }
        return candidate;
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Exception occurred while retrieving candidate: " << e.what();
        LOG_ERROR(ss.str());
        return std::nullopt;
    }
}

bool CandidateServiceImpl::updateCandidate(const Candidate& candidate) {
    try {
        // 参数验证
        if (candidate.getId() <= 0) {
            std::stringstream ss;
            ss << "Invalid candidate ID: " << candidate.getId();
            LOG_ERROR(ss.str());
            return false;
        }
        if (candidate.getName().empty()) {
            LOG_ERROR("Candidate name cannot be empty");
            return false;
        }
        if (candidate.getContact().empty()) {
            LOG_ERROR("Candidate contact cannot be empty");
            return false;
        }

        // 调用DAO更新候选人
        bool success = candidate_dao_->update(candidate);
        if (success) {
            std::stringstream ss;
            ss << "Candidate updated successfully, ID: " << candidate.getId();
            LOG_INFO(ss.str());
        } else {
            std::stringstream ss;
            ss << "Failed to update candidate, ID: " << candidate.getId();
            LOG_ERROR(ss.str());
        }
        return success;
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Exception occurred while updating candidate: " << e.what();
        LOG_ERROR(ss.str());
        return false;
    }
}

bool CandidateServiceImpl::deleteCandidateById(long long id) {
    try {
        // 参数验证
        if (id <= 0) {
            std::stringstream ss;
            ss << "Invalid candidate ID: " << id;
            LOG_ERROR(ss.str());
            return false;
        }

        // 调用DAO删除候选人
        bool success = candidate_dao_->deleteById(id);
        if (success) {
            std::stringstream ss;
            ss << "Candidate deleted successfully, ID: " << id;
            LOG_INFO(ss.str());
        } else {
            std::stringstream ss;
            ss << "Failed to delete candidate, ID: " << id;
            LOG_ERROR(ss.str());
        }
        return success;
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Exception occurred while deleting candidate: " << e.what();
        LOG_ERROR(ss.str());
        return false;
    }
}

std::vector<Candidate> CandidateServiceImpl::getAllCandidates() {
    try {
        // 调用DAO获取所有候选人
        std::vector<Candidate> candidates = candidate_dao_->getAll();
        std::stringstream ss;
        ss << "Retrieved all candidates, count: " << candidates.size();
        LOG_INFO(ss.str());
        return candidates;
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Exception occurred while retrieving all candidates: " << e.what();
        LOG_ERROR(ss.str());
        return {};
    }
}

std::vector<Candidate> CandidateServiceImpl::findCandidatesByCondition(
    const std::optional<std::string>& skills,
    const std::optional<int>& years_of_experience,
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
        oss << "Finding candidates with condition: ";
        if (skills) {
            oss << "skills=" << *skills << ", ";
        }
        if (years_of_experience) {
            oss << "years_of_experience=" << *years_of_experience << ", ";
        }
        oss << "page=" << page << ", page_size=" << page_size;
        LOG_INFO(oss.str());

        // 调用DAO查询候选人
        std::vector<Candidate> candidates = candidate_dao_->findByCondition(
            skills, years_of_experience, page, page_size);
        std::stringstream ss3;
        ss3 << "Found candidates, count: " << candidates.size();
        LOG_INFO(ss3.str());
        return candidates;
    } catch (const std::exception& e) {
        std::stringstream ss4;
        ss4 << "Exception occurred while finding candidates: " << e.what();
        LOG_ERROR(ss4.str());
        return {};
    }
}

} // namespace recruitment
