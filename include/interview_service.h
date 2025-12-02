#ifndef INTERVIEW_SERVICE_H
#define INTERVIEW_SERVICE_H

#include <string>
#include <optional>
#include <vector>
#include "interview.h"

namespace recruitment {

/**
 * @brief 面试服务类
 */
class InterviewService {
public:
    /**
     * @brief 构造函数
     * @param interview_dao 面试DAO
     */
    explicit InterviewService(std::shared_ptr<InterviewDAO> interview_dao)
        : interview_dao_(std::move(interview_dao)) {}

    /**
     * @brief 析构函数
     */
    virtual ~InterviewService() = default;

protected:
    std::shared_ptr<InterviewDAO> interview_dao_; ///< 面试DAO对象

    /**
     * @brief 为投递创建面试
     * @param interview 面试实体
     * @return 创建成功返回面试ID，否则返回-1
     */
    virtual long long createInterview(const Interview& interview) = 0;

    /**
     * @brief 根据ID获取面试
     * @param id 面试ID
     * @return 面试实体，如果不存在则返回空
     */
    virtual std::optional<Interview> getInterviewById(long long id) = 0;

    /**
     * @brief 更新面试信息
     * @param interview 面试实体
     * @return 更新成功返回true，否则返回false
     */
    virtual bool updateInterview(const Interview& interview) = 0;

    /**
     * @brief 根据ID删除面试
     * @param id 面试ID
     * @return 删除成功返回true，否则返回false
     */
    virtual bool deleteInterviewById(long long id) = 0;

    /**
     * @brief 获取所有面试
     * @return 面试实体列表
     */
    virtual std::vector<Interview> getAllInterviews() = 0;

    /**
     * @brief 根据条件查询面试
     * @param application_id 投递ID，可选
     * @param candidate_id 候选人ID，可选
     * @param status 面试状态，可选
     * @param page 页码，默认1
     * @param page_size 每页数量，默认10
     * @return 面试实体列表
     */
    virtual std::vector<Interview> findInterviewsByCondition(const std::optional<long long>& application_id = std::nullopt,
                                                                 const std::optional<long long>& candidate_id = std::nullopt,
                                                                 const std::optional<std::string>& status = std::nullopt,
                                                                 int page = 1,
                                                                 int page_size = 10) = 0;

    /**
     * @brief 取消面试
     * @param interview_id 面试ID
     * @return 操作成功返回true，否则返回false
     */
    virtual bool cancelInterview(long long interview_id) = 0;

    /**
     * @brief 完成面试
     * @param interview_id 面试ID
     * @return 操作成功返回true，否则返回false
     */
    virtual bool completeInterview(long long interview_id) = 0;

    /**
     * @brief 为面试或投递添加评价
     * @param evaluation 评价实体
     * @return 创建成功返回评价ID，否则返回-1
     */
    virtual long long createEvaluation(const Evaluation& evaluation) = 0;

    /**
     * @brief 根据ID获取评价
     * @param id 评价ID
     * @return 评价实体，如果不存在则返回空
     */
    virtual std::optional<Evaluation> getEvaluationById(long long id) = 0;

    /**
     * @brief 更新评价信息
     * @param evaluation 评价实体
     * @return 更新成功返回true，否则返回false
     */
    virtual bool updateEvaluation(const Evaluation& evaluation) = 0;

    /**
     * @brief 根据ID删除评价
     * @param id 评价ID
     * @return 删除成功返回true，否则返回false
     */
    virtual bool deleteEvaluationById(long long id) = 0;

    /**
     * @brief 获取所有评价
     * @return 评价实体列表
     */
    virtual std::vector<Evaluation> getAllEvaluations() = 0;

    /**
     * @brief 根据条件查询评价
     * @param application_id 投递ID，可选
     * @param interview_id 面试ID，可选
     * @param candidate_id 候选人ID，可选
     * @param page 页码，默认1
     * @param page_size 每页数量，默认10
     * @return 评价实体列表
     */
    virtual std::vector<Evaluation> findEvaluationsByCondition(const std::optional<long long>& application_id = std::nullopt,
                                                                   const std::optional<long long>& interview_id = std::nullopt,
                                                                   const std::optional<long long>& candidate_id = std::nullopt,
                                                                   int page = 1,
                                                                   int page_size = 10) = 0;
};

/**
 * @brief 面试服务实现类
 */
class InterviewServiceImpl : public InterviewService {
public:
    /**
     * @brief 构造函数
     * @param interview_dao 面试DAO
     */
    explicit InterviewServiceImpl(std::shared_ptr<InterviewDAO> interview_dao);

    /**
     * @brief 析构函数
     */
    virtual ~InterviewServiceImpl() = default;

    /**
     * @brief 为投递创建面试
     * @param interview 面试实体
     * @return 创建成功返回面试ID，否则返回-1
     */
    long long createInterview(const Interview& interview) override;

    /**
     * @brief 根据ID获取面试
     * @param id 面试ID
     * @return 面试实体，如果不存在则返回空
     */
    std::optional<Interview> getInterviewById(long long id) override;

    /**
     * @brief 更新面试信息
     * @param interview 面试实体
     * @return 更新成功返回true，否则返回false
     */
    bool updateInterview(const Interview& interview) override;

    /**
     * @brief 根据ID删除面试
     * @param id 面试ID
     * @return 删除成功返回true，否则返回false
     */
    bool deleteInterviewById(long long id) override;

    /**
     * @brief 获取所有面试
     * @return 面试实体列表
     */
    std::vector<Interview> getAllInterviews() override;

    /**
     * @brief 根据条件查询面试
     * @param application_id 投递ID，可选
     * @param candidate_id 候选人ID，可选
     * @param status 面试状态，可选
     * @param page 页码，默认1
     * @param page_size 每页数量，默认10
     * @return 面试实体列表
     */
    std::vector<Interview> findInterviewsByCondition(const std::optional<long long>& application_id = std::nullopt,
                                                         const std::optional<long long>& candidate_id = std::nullopt,
                                                         const std::optional<std::string>& status = std::nullopt,
                                                         int page = 1,
                                                         int page_size = 10) override;

    /**
     * @brief 取消面试
     * @param interview_id 面试ID
     * @return 操作成功返回true，否则返回false
     */
    bool cancelInterview(long long interview_id) override;

    /**
     * @brief 完成面试
     * @param interview_id 面试ID
     * @return 操作成功返回true，否则返回false
     */
    bool completeInterview(long long interview_id) override;

    /**
     * @brief 为面试或投递添加评价
     * @param evaluation 评价实体
     * @return 创建成功返回评价ID，否则返回-1
     */
    long long createEvaluation(const Evaluation& evaluation) override;

    /**
     * @brief 根据ID获取评价
     * @param id 评价ID
     * @return 评价实体，如果不存在则返回空
     */
    std::optional<Evaluation> getEvaluationById(long long id) override;

    /**
     * @brief 更新评价信息
     * @param evaluation 评价实体
     * @return 更新成功返回true，否则返回false
     */
    bool updateEvaluation(const Evaluation& evaluation) override;

    /**
     * @brief 根据ID删除评价
     * @param id 评价ID
     * @return 删除成功返回true，否则返回false
     */
    bool deleteEvaluationById(long long id) override;

    /**
     * @brief 获取所有评价
     * @return 评价实体列表
     */
    std::vector<Evaluation> getAllEvaluations() override;

    /**
     * @brief 根据条件查询评价
     * @param application_id 投递ID，可选
     * @param interview_id 面试ID，可选
     * @param candidate_id 候选人ID，可选
     * @param page 页码，默认1
     * @param page_size 每页数量，默认10
     * @return 评价实体列表
     */
    std::vector<Evaluation> findEvaluationsByCondition(const std::optional<long long>& application_id = std::nullopt,
                                                           const std::optional<long long>& interview_id = std::nullopt,
                                                           const std::optional<long long>& candidate_id = std::nullopt,
                                                           int page = 1,
                                                           int page_size = 10) override;

private:
};

} // namespace recruitment

#endif // INTERVIEW_SERVICE_H