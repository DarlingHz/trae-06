#include "interview_service.h"
#include "interview.h"
#include "log.h"

namespace recruitment {

/**
 * @brief 构造函数
 * @param interview_dao 面试DAO
 */
InterviewServiceImpl::InterviewServiceImpl(std::shared_ptr<InterviewDAO> interview_dao)
    : InterviewService(std::move(interview_dao)) {
    LOG_DEBUG("InterviewServiceImpl constructor called");
}

/**
 * @brief 为投递创建面试
 * @param interview 面试实体
 * @return 创建成功返回面试ID，否则返回-1
 */
long long InterviewServiceImpl::createInterview(const Interview& interview) {
    LOG_DEBUG("InterviewServiceImpl::createInterview called");

    try {
        long long interview_id = interview_dao_->create(interview);
        if (interview_id > 0) {
            LOG_INFO("Created interview successfully, ID: " + std::to_string(interview_id));
        } else {
            LOG_ERROR("Failed to create interview");
        }
        return interview_id;
    } catch (const std::exception& e) {
        LOG_ERROR("Error creating interview: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 根据ID获取面试
 * @param id 面试ID
 * @return 面试实体，如果不存在则返回空
 */
std::optional<Interview> InterviewServiceImpl::getInterviewById(long long id) {
    LOG_DEBUG("InterviewServiceImpl::getInterviewById called, ID: " + std::to_string(id));

    try {
        std::optional<Interview> interview = interview_dao_->getById(id);
        if (interview) {
            LOG_DEBUG("Found interview by ID: " + std::to_string(id));
        } else {
            LOG_DEBUG("No interview found by ID: " + std::to_string(id));
        }
        return interview;
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting interview by ID: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 更新面试信息
 * @param interview 面试实体
 * @return 更新成功返回true，否则返回false
 */
bool InterviewServiceImpl::updateInterview(const Interview& interview) {
    LOG_DEBUG("InterviewServiceImpl::updateInterview called, ID: " + std::to_string(interview.getId()));

    try {
        bool success = interview_dao_->update(interview);
        if (success) {
            LOG_INFO("Updated interview successfully, ID: " + std::to_string(interview.getId()));
        } else {
            LOG_ERROR("Failed to update interview, ID: " + std::to_string(interview.getId()));
        }
        return success;
    } catch (const std::exception& e) {
        LOG_ERROR("Error updating interview: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 根据ID删除面试
 * @param id 面试ID
 * @return 删除成功返回true，否则返回false
 */
bool InterviewServiceImpl::deleteInterviewById(long long id) {
    LOG_DEBUG("InterviewServiceImpl::deleteInterviewById called, ID: " + std::to_string(id));

    try {
        bool success = interview_dao_->deleteById(id);
        if (success) {
            LOG_INFO("Deleted interview successfully, ID: " + std::to_string(id));
        } else {
            LOG_ERROR("Failed to delete interview, ID: " + std::to_string(id));
        }
        return success;
    } catch (const std::exception& e) {
        LOG_ERROR("Error deleting interview: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 获取所有面试
 * @return 面试实体列表
 */
std::vector<Interview> InterviewServiceImpl::getAllInterviews() {
    LOG_DEBUG("InterviewServiceImpl::getAllInterviews called");

    try {
        std::vector<Interview> interviews = interview_dao_->getAll();
        LOG_DEBUG("Found " + std::to_string(interviews.size()) + " interviews");
        return interviews;
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting all interviews: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 根据条件查询面试
 * @param application_id 投递ID，可选
 * @param candidate_id 候选人ID，可选
 * @param status 面试状态，可选
 * @param page 页码，默认1
 * @param page_size 每页数量，默认10
 * @return 面试实体列表
 */
std::vector<Interview> InterviewServiceImpl::findInterviewsByCondition(const std::optional<long long>& application_id,
                                                                             const std::optional<long long>& candidate_id,
                                                                             const std::optional<std::string>& status,
                                                                             int page,
                                                                                int page_size) {
    LOG_DEBUG("InterviewServiceImpl::findInterviewsByCondition called");

    try {
        std::vector<Interview> interviews = interview_dao_->findByCondition(
            application_id, candidate_id, status, page, page_size
        );
        LOG_DEBUG("Found " + std::to_string(interviews.size()) + " interviews matching conditions");
        return interviews;
    } catch (const std::exception& e) {
        LOG_ERROR("Error finding interviews by condition: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 取消面试
 * @param interview_id 面试ID
 * @return 操作成功返回true，否则返回false
 */
bool InterviewServiceImpl::cancelInterview(long long interview_id) {
    LOG_DEBUG("InterviewServiceImpl::cancelInterview called, ID: " + std::to_string(interview_id));

    try {
        // 获取面试信息
        std::optional<Interview> interview = interview_dao_->getById(interview_id);
        if (!interview) {
            LOG_ERROR("Interview not found, ID: " + std::to_string(interview_id));
            return false;
        }

        // 更新面试状态为取消
        Interview updated_interview = *interview;
        updated_interview.setStatus("cancelled");

        bool success = interview_dao_->update(updated_interview);
        if (success) {
            LOG_INFO("Cancelled interview successfully, ID: " + std::to_string(interview_id));
        } else {
            LOG_ERROR("Failed to cancel interview, ID: " + std::to_string(interview_id));
        }
        return success;
    } catch (const std::exception& e) {
        LOG_ERROR("Error cancelling interview: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 完成面试
 * @param interview_id 面试ID
 * @return 操作成功返回true，否则返回false
 */
bool InterviewServiceImpl::completeInterview(long long interview_id) {
    LOG_DEBUG("InterviewServiceImpl::completeInterview called, ID: " + std::to_string(interview_id));

    try {
        // 获取面试信息
        std::optional<Interview> interview = interview_dao_->getById(interview_id);
        if (!interview) {
            LOG_ERROR("Interview not found, ID: " + std::to_string(interview_id));
            return false;
        }

        // 更新面试状态为完成
        Interview updated_interview = *interview;
        updated_interview.setStatus("completed");

        bool success = interview_dao_->update(updated_interview);
        if (success) {
            LOG_INFO("Completed interview successfully, ID: " + std::to_string(interview_id));
        } else {
            LOG_ERROR("Failed to complete interview, ID: " + std::to_string(interview_id));
        }
        return success;
    } catch (const std::exception& e) {
        LOG_ERROR("Error completing interview: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 为面试或投递添加评价
 * @param evaluation 评价实体
 * @return 创建成功返回评价ID，否则返回-1
 */
long long InterviewServiceImpl::createEvaluation(const Evaluation& evaluation) {
    LOG_DEBUG("InterviewServiceImpl::createEvaluation called");

    try {
        long long evaluation_id = interview_dao_->createEvaluation(evaluation);
        if (evaluation_id > 0) {
            LOG_INFO("Created evaluation successfully, ID: " + std::to_string(evaluation_id));
        } else {
            LOG_ERROR("Failed to create evaluation");
        }
        return evaluation_id;
    } catch (const std::exception& e) {
        LOG_ERROR("Error creating evaluation: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 根据ID获取评价
 * @param id 评价ID
 * @return 评价实体，如果不存在则返回空
 */
std::optional<Evaluation> InterviewServiceImpl::getEvaluationById(long long id) {
    LOG_DEBUG("InterviewServiceImpl::getEvaluationById called, ID: " + std::to_string(id));

    try {
        std::optional<Evaluation> evaluation = interview_dao_->getEvaluationById(id);
        if (evaluation) {
            LOG_DEBUG("Found evaluation by ID: " + std::to_string(id));
        } else {
            LOG_DEBUG("No evaluation found by ID: " + std::to_string(id));
        }
        return evaluation;
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting evaluation by ID: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 更新评价信息
 * @param evaluation 评价实体
 * @return 更新成功返回true，否则返回false
 */
bool InterviewServiceImpl::updateEvaluation(const Evaluation& evaluation) {
    LOG_DEBUG("InterviewServiceImpl::updateEvaluation called, ID: " + std::to_string(evaluation.getId()));

    try {
        bool success = interview_dao_->updateEvaluation(evaluation);
        if (success) {
            LOG_INFO("Updated evaluation successfully, ID: " + std::to_string(evaluation.getId()));
        } else {
            LOG_ERROR("Failed to update evaluation, ID: " + std::to_string(evaluation.getId()));
        }
        return success;
    } catch (const std::exception& e) {
        LOG_ERROR("Error updating evaluation: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 根据ID删除评价
 * @param id 评价ID
 * @return 删除成功返回true，否则返回false
 */
bool InterviewServiceImpl::deleteEvaluationById(long long id) {
    LOG_DEBUG("InterviewServiceImpl::deleteEvaluationById called, ID: " + std::to_string(id));

    try {
        bool success = interview_dao_->deleteEvaluationById(id);
        if (success) {
            LOG_INFO("Deleted evaluation successfully, ID: " + std::to_string(id));
        } else {
            LOG_ERROR("Failed to delete evaluation, ID: " + std::to_string(id));
        }
        return success;
    } catch (const std::exception& e) {
        LOG_ERROR("Error deleting evaluation: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 获取所有评价
 * @return 评价实体列表
 */
std::vector<Evaluation> InterviewServiceImpl::getAllEvaluations() {
    LOG_DEBUG("InterviewServiceImpl::getAllEvaluations called");

    try {
        // TODO: Implement getAllEvaluations method in InterviewDAO
        std::vector<Evaluation> evaluations;
        LOG_DEBUG("Found " + std::to_string(evaluations.size()) + " evaluations");
        return evaluations;
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting all evaluations: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 根据条件查询评价
 * @param application_id 投递ID，可选
 * @param interview_id 面试ID，可选
 * @param candidate_id 候选人ID，可选
 * @param page 页码，默认1
 * @param page_size 每页数量，默认10
 * @return 评价实体列表
 */
std::vector<Evaluation> InterviewServiceImpl::findEvaluationsByCondition(const std::optional<long long>& application_id,
                                                                               const std::optional<long long>& interview_id,
                                                                               const std::optional<long long>& candidate_id,
                                                                               int page,
                                                                               int page_size) {
    LOG_DEBUG("InterviewServiceImpl::findEvaluationsByCondition called");

    try {
        std::vector<Evaluation> evaluations;
        
        // 根据条件查询评价
        if (application_id) {
            evaluations = interview_dao_->findEvaluationsByApplicationId(*application_id);
        } else if (interview_id) {
            evaluations = interview_dao_->findEvaluationsByInterviewId(*interview_id);
        } else if (candidate_id) {
            // TODO: Implement findEvaluationsByCandidateId method in InterviewDAO
        }
        
        // 分页查询结果
        size_t start_index = (page - 1) * page_size;
        size_t end_index = start_index + page_size;
        if (start_index >= evaluations.size()) {
            evaluations.clear();
        } else if (end_index >= evaluations.size()) {
            evaluations.erase(evaluations.begin(), evaluations.begin() + start_index);
        } else {
            evaluations.erase(evaluations.begin(), evaluations.begin() + start_index);
            evaluations.erase(evaluations.begin() + page_size, evaluations.end());
        }

        LOG_DEBUG("Found " + std::to_string(evaluations.size()) + " evaluations matching conditions");
        return evaluations;
    } catch (const std::exception& e) {
        LOG_ERROR("Error finding evaluations by condition: " + std::string(e.what()));
        throw;
    }
}

} // namespace recruitment
