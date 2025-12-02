#include "statistics_dao.h"
#include "database.h"
#include "log.h"

namespace recruitment {

/**
 * @brief 获取职位状态统计
 * @param job_id 职位ID
 * @return 职位状态统计数据，如果职位不存在则返回空
 */
std::optional<JobStatusStatistics> StatisticsDAO::getJobStatusStatistics(long long job_id) {
    LOG_DEBUG("Getting job status statistics for job ID: " + std::to_string(job_id));

    try {
        auto conn = Database::getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return std::nullopt;
        }

        // 查询职位基本信息
        std::string job_query = "SELECT id, title FROM jobs WHERE id = ?";
        std::vector<QueryParameter> job_parameters;
        job_parameters.emplace_back(job_id);
        QueryResult job_result = conn->executeQuery(job_query, job_parameters);

        if (job_result.rows.empty()) {
            LOG_DEBUG("Job not found for ID: " + std::to_string(job_id));
            return std::nullopt;
        }

        JobStatusStatistics stats;
        stats.job_id = job_result.rows[0].columns["id"].int_value;
        stats.job_title = job_result.rows[0].columns["title"].text_value;

        // 查询职位总投递数
        std::string total_applications_query = "SELECT COUNT(*) FROM applications WHERE job_id = ?";
        std::vector<QueryParameter> total_applications_parameters;
        total_applications_parameters.emplace_back(job_id);
        QueryResult total_applications_result = conn->executeQuery(total_applications_query, total_applications_parameters);
        if (!total_applications_result.rows.empty()) {
            auto it = total_applications_result.rows[0].columns.find("COUNT(*)");
            stats.total_applications = (it != total_applications_result.rows[0].columns.end()) ? it->second.int_value : 0;
        }

        // 查询各状态的投递数
        std::string status_query = "SELECT status, COUNT(*) FROM applications WHERE job_id = ? GROUP BY status";
        std::vector<QueryParameter> status_parameters;
        status_parameters.emplace_back(job_id);
        QueryResult status_result = conn->executeQuery(status_query, status_parameters);

        for (const auto& row : status_result.rows) {
            auto status_it = row.columns.find("status");
            std::string status = (status_it != row.columns.end()) ? status_it->second.text_value : "";
            auto count_it = row.columns.find("COUNT(*)");
            int count = (count_it != row.columns.end()) ? count_it->second.int_value : 0;
            
            if (status == "applied") {
                stats.applied_count = count;
            } else if (status == "screening") {
                stats.screening_count = count;
            } else if (status == "interviewing") {
                stats.interviewing_count = count;
            } else if (status == "offered") {
                stats.offered_count = count;
            } else if (status == "rejected") {
                stats.rejected_count = count;
            }
        }

        LOG_DEBUG("Got job status statistics for job ID: " + std::to_string(job_id) + ", total applications: " + std::to_string(stats.total_applications));
        return stats;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get job status statistics: " + std::string(e.what()));
        return std::nullopt;
    }
}

/**
 * @brief 获取时间维度统计
 * @param start_date 开始日期（格式：YYYY-MM-DD）
 * @param end_date 结束日期（格式：YYYY-MM-DD）
 * @return 时间维度统计数据
 */
TimeDimensionStatistics StatisticsDAO::getTimeDimensionStatistics(const std::string& start_date,
                                                                     const std::string& end_date) {
    LOG_DEBUG("Getting time dimension statistics from " + start_date + " to " + end_date);

    TimeDimensionStatistics stats;
    stats.start_date = start_date;
    stats.end_date = end_date;

    try {
        auto conn = Database::getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return stats;
        }

        // 查询新增投递数量
        std::string new_applications_query = "SELECT COUNT(*) FROM applications WHERE created_at BETWEEN ? AND ?";
        std::vector<QueryParameter> new_applications_parameters;
        new_applications_parameters.emplace_back(start_date);
        new_applications_parameters.emplace_back(end_date + " 23:59:59");
        QueryResult new_applications_result = conn->executeQuery(new_applications_query, new_applications_parameters);
        if (!new_applications_result.rows.empty()) {
            auto it = new_applications_result.rows[0].columns.find("COUNT(*)");
            stats.new_applications = (it != new_applications_result.rows[0].columns.end()) ? it->second.int_value : 0;
        }

        // 查询完成面试数量
        std::string completed_interviews_query = "SELECT COUNT(*) FROM interviews WHERE interview_date BETWEEN ? AND ? AND status = 'completed'";
        std::vector<QueryParameter> completed_interviews_parameters;
        completed_interviews_parameters.emplace_back(start_date);
        completed_interviews_parameters.emplace_back(end_date);
        QueryResult completed_interviews_result = conn->executeQuery(completed_interviews_query, completed_interviews_parameters);
        if (!completed_interviews_result.rows.empty()) {
            auto it = completed_interviews_result.rows[0].columns.find("COUNT(*)");
            stats.completed_interviews = (it != completed_interviews_result.rows[0].columns.end()) ? it->second.int_value : 0;
        }

        // 查询发出Offer数量
        std::string offered_count_query = "SELECT COUNT(*) FROM applications WHERE status = 'offered' AND updated_at BETWEEN ? AND ?";
        std::vector<QueryParameter> offered_count_parameters;
        offered_count_parameters.emplace_back(start_date);
        offered_count_parameters.emplace_back(end_date + " 23:59:59");
        QueryResult offered_count_result = conn->executeQuery(offered_count_query, offered_count_parameters);
        if (!offered_count_result.rows.empty()) {
            auto it = offered_count_result.rows[0].columns.find("COUNT(*)");
            stats.offered_count = (it != offered_count_result.rows[0].columns.end()) ? it->second.int_value : 0;
        }

        LOG_DEBUG("Got time dimension statistics from " + start_date + " to " + end_date + ", new applications: " + std::to_string(stats.new_applications) + ", completed interviews: " + std::to_string(stats.completed_interviews) + ", offered count: " + std::to_string(stats.offered_count));
        return stats;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get time dimension statistics: " + std::string(e.what()));
        return stats;
    }
}

/**
 * @brief 获取候选人画像
 * @param candidate_id 候选人ID
 * @return 候选人画像数据，如果候选人不存在则返回空
 */
std::optional<CandidateProfile> StatisticsDAO::getCandidateProfile(long long candidate_id) {
    LOG_DEBUG("Getting candidate profile for candidate ID: " + std::to_string(candidate_id));

    try {
        auto conn = Database::getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return std::nullopt;
        }

        // 查询候选人基本信息
        std::string candidate_query = "SELECT id, name FROM candidates WHERE id = ?";
        std::vector<QueryParameter> candidate_parameters;
        candidate_parameters.emplace_back(candidate_id);
        QueryResult candidate_result = conn->executeQuery(candidate_query, candidate_parameters);

        if (candidate_result.rows.empty()) {
            LOG_DEBUG("Candidate not found for ID: " + std::to_string(candidate_id));
            return std::nullopt;
        }

        CandidateProfile profile;
        auto id_it = candidate_result.rows[0].columns.find("id");
        profile.candidate_id = (id_it != candidate_result.rows[0].columns.end()) ? id_it->second.int_value : 0;
        auto name_it = candidate_result.rows[0].columns.find("name");
        profile.candidate_name = (name_it != candidate_result.rows[0].columns.end()) ? name_it->second.text_value : "";

        // 查询总投递数
        std::string total_applications_query = "SELECT COUNT(*) FROM applications WHERE candidate_id = ?";
        std::vector<QueryParameter> total_applications_parameters;
        total_applications_parameters.emplace_back(candidate_id);
        QueryResult total_applications_result = conn->executeQuery(total_applications_query, total_applications_parameters);
        if (!total_applications_result.rows.empty()) {
            auto it = total_applications_result.rows[0].columns.find("COUNT(*)");
            profile.total_applications = (it != total_applications_result.rows[0].columns.end()) ? it->second.int_value : 0;
        }

        // 查询面试次数
        std::string interview_count_query = "SELECT COUNT(*) FROM interviews WHERE candidate_id = ?";
        std::vector<QueryParameter> interview_count_parameters;
        interview_count_parameters.emplace_back(candidate_id);
        QueryResult interview_count_result = conn->executeQuery(interview_count_query, interview_count_parameters);
        if (!interview_count_result.rows.empty()) {
            auto it = interview_count_result.rows[0].columns.find("COUNT(*)");
            profile.interview_count = (it != interview_count_result.rows[0].columns.end()) ? it->second.int_value : 0;
        }

        // 查询平均评分
        std::string average_score_query = "SELECT AVG(score) FROM interviews WHERE candidate_id = ? AND score IS NOT NULL";
        std::vector<QueryParameter> average_score_parameters;
        average_score_parameters.emplace_back(candidate_id);
        QueryResult average_score_result = conn->executeQuery(average_score_query, average_score_parameters);
        if (!average_score_result.rows.empty() && !average_score_result.rows[0].columns["AVG(score)"].is_null) {
            profile.average_score = average_score_result.rows[0].columns["AVG(score)"].real_value;
        }

        // 查询技能匹配度（这里只是示例，实际实现可能更复杂）
        profile.skill_match["C++"] = 90;
        profile.skill_match["Python"] = 85;
        profile.skill_match["Java"] = 70;

        // 查询投递历史（这里只是示例，实际实现可能更复杂）
        profile.application_history["Software Engineer"] = 3;
        profile.application_history["Data Scientist"] = 2;
        profile.application_history["Product Manager"] = 1;

        // 计算综合匹配度
        profile.match_score = (profile.average_score * 0.4) + (profile.interview_count * 5) + (profile.total_applications * 2);
        if (profile.match_score > 100) {
            profile.match_score = 100;
        }

        LOG_DEBUG("Got candidate profile for candidate ID: " + std::to_string(candidate_id) + ", match score: " + std::to_string(profile.match_score));
        return profile;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get candidate profile: " + std::string(e.what()));
        return std::nullopt;
    }
}

/**
 * @brief 获取公司招聘统计
 * @param company_id 公司ID
 * @param start_date 开始日期（可选）
 * @param end_date 结束日期（可选）
 * @return 公司招聘统计数据，按职位分组
 */
std::vector<JobStatusStatistics> StatisticsDAO::getCompanyRecruitmentStatistics(long long company_id,
                                                                                       const std::optional<std::string>& start_date,
                                                                                       const std::optional<std::string>& end_date) {
    LOG_DEBUG("Getting company recruitment statistics for company ID: " + std::to_string(company_id));

    std::vector<JobStatusStatistics> stats_list;

    try {
        auto conn = Database::getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return stats_list;
        }

        // 构建查询条件
        std::string query = "SELECT j.id, j.title, COUNT(a.id) as total_applications, "
                            "SUM(CASE WHEN a.status = 'applied' THEN 1 ELSE 0 END) as applied_count, "
                            "SUM(CASE WHEN a.status = 'screening' THEN 1 ELSE 0 END) as screening_count, "
                            "SUM(CASE WHEN a.status = 'interviewing' THEN 1 ELSE 0 END) as interviewing_count, "
                            "SUM(CASE WHEN a.status = 'offered' THEN 1 ELSE 0 END) as offered_count, "
                            "SUM(CASE WHEN a.status = 'rejected' THEN 1 ELSE 0 END) as rejected_count "
                            "FROM jobs j "
                            "LEFT JOIN applications a ON j.id = a.job_id "
                            "WHERE j.company_id = ?";

        if (start_date && end_date) {
            query += " AND a.created_at BETWEEN ? AND ?";
        }

        query += " GROUP BY j.id, j.title ORDER BY j.id";

        std::vector<QueryParameter> parameters;
        parameters.emplace_back(company_id);

        if (start_date && end_date) {
            parameters.emplace_back(start_date.value());
            parameters.emplace_back(end_date.value() + " 23:59:59");
        }

        QueryResult result = conn->executeQuery(query, parameters);

        for (const auto& row : result.rows) {
            JobStatusStatistics stats;
            auto id_it = row.columns.find("id");
            stats.job_id = (id_it != row.columns.end()) ? id_it->second.int_value : 0;
            auto title_it = row.columns.find("title");
            stats.job_title = (title_it != row.columns.end()) ? title_it->second.text_value : "";
            auto total_applications_it = row.columns.find("total_applications");
            stats.total_applications = (total_applications_it != row.columns.end()) ? total_applications_it->second.int_value : 0;
            auto applied_count_it = row.columns.find("applied_count");
            stats.applied_count = (applied_count_it != row.columns.end()) ? applied_count_it->second.int_value : 0;
            auto screening_count_it = row.columns.find("screening_count");
            stats.screening_count = (screening_count_it != row.columns.end()) ? screening_count_it->second.int_value : 0;
            auto interviewing_count_it = row.columns.find("interviewing_count");
            stats.interviewing_count = (interviewing_count_it != row.columns.end()) ? interviewing_count_it->second.int_value : 0;
            auto offered_count_it = row.columns.find("offered_count");
            stats.offered_count = (offered_count_it != row.columns.end()) ? offered_count_it->second.int_value : 0;
            auto rejected_count_it = row.columns.find("rejected_count");
            stats.rejected_count = (rejected_count_it != row.columns.end()) ? rejected_count_it->second.int_value : 0;

            stats_list.push_back(stats);
        }

        LOG_DEBUG("Got company recruitment statistics for company ID: " + std::to_string(company_id) + ", found " + std::to_string(stats_list.size()) + " jobs");
        return stats_list;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get company recruitment statistics: " + std::string(e.what()));
        return stats_list;
    }
}

} // namespace recruitment
