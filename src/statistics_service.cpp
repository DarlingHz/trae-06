#include "statistics_dao.h"
#include "log.h"
#include "statistics_service.h"

namespace recruitment {

/**
 * @brief 默认构造函数
 */
StatisticsServiceImpl::StatisticsServiceImpl() : statistics_dao_(std::make_shared<StatisticsDAO>()) {
    LOG_DEBUG("StatisticsServiceImpl default constructor called");
}

/**
 * @brief 构造函数
 * @param statistics_dao 统计数据访问对象
 */
StatisticsServiceImpl::StatisticsServiceImpl(std::shared_ptr<StatisticsDAO> statistics_dao) : statistics_dao_(std::move(statistics_dao)) {
    LOG_DEBUG("StatisticsServiceImpl constructor called with StatisticsDAO");
}

/**
 * @brief 析构函数
 */
StatisticsServiceImpl::~StatisticsServiceImpl() {
    LOG_DEBUG("StatisticsServiceImpl destructor called");
}

/**
 * @brief 获取职位状态统计
 * @param job_id 职位ID
 * @return 职位状态统计数据，如果职位不存在则返回空
 */
std::optional<JobStatusStatistics> StatisticsServiceImpl::getJobStatusStatistics(long long job_id) {
    LOG_DEBUG("StatisticsServiceImpl::getJobStatusStatistics called for job ID: " + std::to_string(job_id));

    try {
        auto stats = statistics_dao_->getJobStatusStatistics(job_id);
        if (stats) {
            LOG_DEBUG("Got job status statistics for job ID: " + std::to_string(job_id));
        } else {
            LOG_DEBUG("No job status statistics found for job ID: " + std::to_string(job_id));
        }
        return stats;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get job status statistics: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 获取时间维度统计
 * @param start_date 开始日期（格式：YYYY-MM-DD）
 * @param end_date 结束日期（格式：YYYY-MM-DD）
 * @return 时间维度统计数据
 */
TimeDimensionStatistics StatisticsServiceImpl::getTimeDimensionStatistics(const std::string& start_date,
                                                                             const std::string& end_date) {
    LOG_DEBUG("StatisticsServiceImpl::getTimeDimensionStatistics called from " + start_date + " to " + end_date);

    try {
        auto stats = statistics_dao_->getTimeDimensionStatistics(start_date, end_date);
        LOG_DEBUG("Got time dimension statistics from " + start_date + " to " + end_date);
        return stats;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get time dimension statistics: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 获取候选人画像
 * @param candidate_id 候选人ID
 * @return 候选人画像数据，如果候选人不存在则返回空
 */
std::optional<CandidateProfile> StatisticsServiceImpl::getCandidateProfile(long long candidate_id) {
    LOG_DEBUG("StatisticsServiceImpl::getCandidateProfile called for candidate ID: " + std::to_string(candidate_id));

    try {
        auto profile = statistics_dao_->getCandidateProfile(candidate_id);
        if (profile) {
            LOG_DEBUG("Got candidate profile for candidate ID: " + std::to_string(candidate_id));
        } else {
            LOG_DEBUG("No candidate profile found for candidate ID: " + std::to_string(candidate_id));
        }
        return profile;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get candidate profile: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 获取公司招聘统计
 * @param company_id 公司ID
 * @param start_date 开始日期（可选）
 * @param end_date 结束日期（可选）
 * @return 公司招聘统计数据，按职位分组
 */
std::vector<JobStatusStatistics> StatisticsServiceImpl::getCompanyRecruitmentStatistics(long long company_id,
                                                                                               const std::optional<std::string>& start_date,
                                                                                               const std::optional<std::string>& end_date) {
    LOG_DEBUG("StatisticsServiceImpl::getCompanyRecruitmentStatistics called for company ID: " + std::to_string(company_id));

    try {
        auto stats_list = statistics_dao_->getCompanyRecruitmentStatistics(company_id, start_date, end_date);
        LOG_DEBUG("Got company recruitment statistics for company ID: " + std::to_string(company_id) + ", found " + std::to_string(stats_list.size()) + " jobs");
        return stats_list;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get company recruitment statistics: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 计算综合匹配度
 * @param candidate_id 候选人ID
 * @return 综合匹配度（0-100）
 */
double StatisticsServiceImpl::calculateMatchScore(long long candidate_id) {
    LOG_DEBUG("StatisticsServiceImpl::calculateMatchScore called for candidate ID: " + std::to_string(candidate_id));

    try {
        // 实现计算综合匹配度的逻辑
        // 这里只是一个示例，实际实现可能需要更复杂的逻辑
        auto profile = statistics_dao_->getCandidateProfile(candidate_id);
        if (!profile) {
            LOG_DEBUG("No candidate profile found for candidate ID: " + std::to_string(candidate_id));
            return 0.0;
        }

        // 简单的匹配度计算：平均评分 * 投递成功率
        double average_score = profile->average_score;
        int total_applications = profile->total_applications;
        int interview_count = profile->interview_count;

        double application_success_rate = total_applications > 0 ? static_cast<double>(interview_count) / total_applications : 0.0;
        double match_score = average_score * 0.7 + application_success_rate * 100 * 0.3;

        // 确保匹配度在0-100之间
        match_score = std::max(0.0, std::min(100.0, match_score));

        LOG_DEBUG("Calculated match score for candidate ID: " + std::to_string(candidate_id) + ", score: " + std::to_string(match_score));
        return match_score;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to calculate match score: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 分析技能匹配度
 * @param candidate_id 候选人ID
 * @return 技能匹配度映射
 */
std::unordered_map<std::string, int> StatisticsServiceImpl::analyzeSkillMatch(long long candidate_id) {
    LOG_DEBUG("StatisticsServiceImpl::analyzeSkillMatch called for candidate ID: " + std::to_string(candidate_id));

    try {
        // 实现分析技能匹配度的逻辑
        // 这里只是一个示例，实际实现可能需要更复杂的逻辑
        auto profile = statistics_dao_->getCandidateProfile(candidate_id);
        if (!profile) {
            LOG_DEBUG("No candidate profile found for candidate ID: " + std::to_string(candidate_id));
            return {};
        }

        // 返回候选人的技能匹配度
        LOG_DEBUG("Got skill match for candidate ID: " + std::to_string(candidate_id) + ", found " + std::to_string(profile->skill_match.size()) + " skills");
        return profile->skill_match;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to analyze skill match: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief 分析投递历史
 * @param candidate_id 候选人ID
 * @return 投递历史映射
 */
std::unordered_map<std::string, int> StatisticsServiceImpl::analyzeApplicationHistory(long long candidate_id) {
    LOG_DEBUG("StatisticsServiceImpl::analyzeApplicationHistory called for candidate ID: " + std::to_string(candidate_id));

    try {
        // 实现分析投递历史的逻辑
        // 这里只是一个示例，实际实现可能需要更复杂的逻辑
        auto profile = statistics_dao_->getCandidateProfile(candidate_id);
        if (!profile) {
            LOG_DEBUG("No candidate profile found for candidate ID: " + std::to_string(candidate_id));
            return {};
        }

        // 返回候选人的投递历史
        LOG_DEBUG("Got application history for candidate ID: " + std::to_string(candidate_id) + ", found " + std::to_string(profile->application_history.size()) + " entries");
        return profile->application_history;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to analyze application history: " + std::string(e.what()));
        throw;
    }
}

} // namespace recruitment
