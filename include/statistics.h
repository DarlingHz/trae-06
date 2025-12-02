#ifndef STATISTICS_H
#define STATISTICS_H

#include <string>
#include <optional>
#include <vector>
#include <unordered_map>

namespace recruitment {

/**
 * @brief 职位状态统计数据结构
 */
struct JobStatusStatistics {
    long long job_id; ///< 职位ID
    std::string job_title; ///< 职位名称
    int total_applications; ///< 总投递数
    int applied_count; ///< 已申请数量
    int screening_count; ///< 筛选中数量
    int interviewing_count; ///< 面试中数量
    int offered_count; ///< 已录用数量
    int rejected_count; ///< 已拒绝数量
};

/**
 * @brief 时间维度统计数据结构
 */
struct TimeDimensionStatistics {
    std::string start_date; ///< 开始日期
    std::string end_date; ///< 结束日期
    int new_applications; ///< 新增投递数量
    int completed_interviews; ///< 完成面试数量
    int offered_count; ///< 发出Offer数量
};

/**
 * @brief 候选人画像数据结构
 */
struct CandidateProfile {
    long long candidate_id; ///< 候选人ID
    std::string candidate_name; ///< 候选人姓名
    double match_score; ///< 综合匹配度（0-100）
    int total_applications; ///< 总投递数
    int interview_count; ///< 面试次数
    double average_score; ///< 平均评分
    std::unordered_map<std::string, int> skill_match; ///< 技能匹配度
    std::unordered_map<std::string, int> application_history; ///< 投递历史
};

/**
 * @brief 统计分析服务类
 */
class StatisticsService {
public:
    /**
     * @brief 默认构造函数
     */
    StatisticsService() = default;

    /**
     * @brief 析构函数
     */
    virtual ~StatisticsService() = default;

    /**
     * @brief 获取职位状态统计
     * @param job_id 职位ID
     * @return 职位状态统计数据，如果职位不存在则返回空
     */
    virtual std::optional<JobStatusStatistics> getJobStatusStatistics(long long job_id) = 0;

    /**
     * @brief 获取时间维度统计
     * @param start_date 开始日期（格式：YYYY-MM-DD）
     * @param end_date 结束日期（格式：YYYY-MM-DD）
     * @return 时间维度统计数据
     */
    virtual TimeDimensionStatistics getTimeDimensionStatistics(const std::string& start_date,
                                                                 const std::string& end_date) = 0;

    /**
     * @brief 获取候选人画像
     * @param candidate_id 候选人ID
     * @return 候选人画像数据，如果候选人不存在则返回空
     */
    virtual std::optional<CandidateProfile> getCandidateProfile(long long candidate_id) = 0;

    /**
     * @brief 获取公司招聘统计
     * @param company_id 公司ID
     * @param start_date 开始日期（可选）
     * @param end_date 结束日期（可选）
     * @return 公司招聘统计数据，按职位分组
     */
    virtual std::vector<JobStatusStatistics> getCompanyRecruitmentStatistics(long long company_id,
                                                                                   const std::optional<std::string>& start_date = std::nullopt,
                                                                                   const std::optional<std::string>& end_date = std::nullopt) = 0;

protected:
    /**
     * @brief 计算综合匹配度
     * @param candidate_id 候选人ID
     * @return 综合匹配度（0-100）
     */
    virtual double calculateMatchScore(long long candidate_id) = 0;

    /**
     * @brief 分析技能匹配度
     * @param candidate_id 候选人ID
     * @return 技能匹配度映射
     */
    virtual std::unordered_map<std::string, int> analyzeSkillMatch(long long candidate_id) = 0;

    /**
     * @brief 分析投递历史
     * @param candidate_id 候选人ID
     * @return 投递历史映射
     */
    virtual std::unordered_map<std::string, int> analyzeApplicationHistory(long long candidate_id) = 0;
};

} // namespace recruitment

#endif // STATISTICS_H