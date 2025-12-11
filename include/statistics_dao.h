#ifndef STATISTICS_DAO_H
#define STATISTICS_DAO_H

#include <optional>
#include "statistics.h"
#include "base_dao.h"

namespace recruitment {

/**
 * @brief 统计数据访问对象类
 */
class StatisticsDAO {
public:
    /**
     * @brief 默认构造函数
     */
    StatisticsDAO() = default;

    /**
     * @brief 析构函数
     */
    virtual ~StatisticsDAO() = default;

    /**
     * @brief 获取职位状态统计
     * @param job_id 职位ID
     * @return 职位状态统计数据，如果职位不存在则返回空
     */
    virtual std::optional<JobStatusStatistics> getJobStatusStatistics(long long job_id);

    /**
     * @brief 获取时间维度统计
     * @param start_date 开始日期（格式：YYYY-MM-DD）
     * @param end_date 结束日期（格式：YYYY-MM-DD）
     * @return 时间维度统计数据
     */
    virtual TimeDimensionStatistics getTimeDimensionStatistics(const std::string& start_date,
                                                                 const std::string& end_date);

    /**
     * @brief 获取候选人画像
     * @param candidate_id 候选人ID
     * @return 候选人画像数据，如果候选人不存在则返回空
     */
    virtual std::optional<CandidateProfile> getCandidateProfile(long long candidate_id);

    /**
     * @brief 获取公司招聘统计
     * @param company_id 公司ID
     * @param start_date 开始日期（可选）
     * @param end_date 结束日期（可选）
     * @return 公司招聘统计数据，按职位分组
     */
    virtual std::vector<JobStatusStatistics> getCompanyRecruitmentStatistics(long long company_id,
                                                                                   const std::optional<std::string>& start_date = std::nullopt,
                                                                                   const std::optional<std::string>& end_date = std::nullopt);
};

} // namespace recruitment

#endif // STATISTICS_DAO_H
