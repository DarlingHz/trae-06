#ifndef STATISTICS_SERVICE_H
#define STATISTICS_SERVICE_H

#include <string>
#include <optional>
#include <vector>
#include <memory>
#include "statistics.h"
#include "statistics_dao.h"

namespace recruitment {

/**
 * @brief 统计服务实现类
 */
class StatisticsServiceImpl : public StatisticsService {
public:
    /**
     * @brief 默认构造函数
     */
    StatisticsServiceImpl();

    /**
     * @brief 构造函数
     * @param statistics_dao 统计数据访问对象
     */
    explicit StatisticsServiceImpl(std::shared_ptr<StatisticsDAO> statistics_dao);

    /**
     * @brief 析构函数
     */
    ~StatisticsServiceImpl() override;

    /**
     * @brief 获取职位状态统计
     * @param job_id 职位ID
     * @return 职位状态统计数据，如果职位不存在则返回空
     */
    std::optional<JobStatusStatistics> getJobStatusStatistics(long long job_id) override;

    /**
     * @brief 获取时间维度统计
     * @param start_date 开始日期（格式：YYYY-MM-DD）
     * @param end_date 结束日期（格式：YYYY-MM-DD）
     * @return 时间维度统计数据
     */
    TimeDimensionStatistics getTimeDimensionStatistics(const std::string& start_date,
                                                         const std::string& end_date) override;

    /**
     * @brief 获取候选人画像
     * @param candidate_id 候选人ID
     * @return 候选人画像数据，如果候选人不存在则返回空
     */
    std::optional<CandidateProfile> getCandidateProfile(long long candidate_id) override;

    /**
     * @brief 获取公司招聘统计
     * @param company_id 公司ID
     * @param start_date 开始日期（可选）
     * @param end_date 结束日期（可选）
     * @return 公司招聘统计数据，按职位分组
     */
    std::vector<JobStatusStatistics> getCompanyRecruitmentStatistics(long long company_id,
                                                                           const std::optional<std::string>& start_date = std::nullopt,
                                                                           const std::optional<std::string>& end_date = std::nullopt) override;

    /**
     * @brief 计算综合匹配度
     * @param candidate_id 候选人ID
     * @return 综合匹配度（0-100）
     */
    double calculateMatchScore(long long candidate_id) override;

    /**
     * @brief 分析技能匹配度
     * @param candidate_id 候选人ID
     * @return 技能匹配度映射
     */
    std::unordered_map<std::string, int> analyzeSkillMatch(long long candidate_id) override;

    /**
     * @brief 分析投递历史
     * @param candidate_id 候选人ID
     * @return 投递历史映射
     */
    std::unordered_map<std::string, int> analyzeApplicationHistory(long long candidate_id) override;

private:
    std::shared_ptr<StatisticsDAO> statistics_dao_; ///< 统计数据访问对象
};

} // namespace recruitment

#endif // STATISTICS_SERVICE_H