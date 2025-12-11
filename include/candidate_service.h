#ifndef CANDIDATE_SERVICE_H
#define CANDIDATE_SERVICE_H

#include <string>
#include <optional>
#include <vector>
#include "candidate.h"

namespace recruitment {

/**
 * @brief 候选人服务类
 */
class CandidateService {
public:
    /**
     * @brief 构造函数
     * @param candidate_dao 候选人DAO
     */
    explicit CandidateService(std::shared_ptr<CandidateDAO> candidate_dao)
        : candidate_dao_(std::move(candidate_dao)) {}

    /**
     * @brief 析构函数
     */
    virtual ~CandidateService() = default;

protected:
    std::shared_ptr<CandidateDAO> candidate_dao_; ///< 候选人DAO对象

    /**
     * @brief 创建候选人档案
     * @param candidate 候选人实体
     * @return 创建成功返回候选人ID，否则返回-1
     */
    virtual long long createCandidate(const Candidate& candidate) = 0;

    /**
     * @brief 根据ID获取候选人
     * @param id 候选人ID
     * @return 候选人实体，如果不存在则返回空
     */
    virtual std::optional<Candidate> getCandidateById(long long id) = 0;

    /**
     * @brief 更新候选人信息
     * @param candidate 候选人实体
     * @return 更新成功返回true，否则返回false
     */
    virtual bool updateCandidate(const Candidate& candidate) = 0;

    /**
     * @brief 根据ID删除候选人
     * @param id 候选人ID
     * @return 删除成功返回true，否则返回false
     */
    virtual bool deleteCandidateById(long long id) = 0;

    /**
     * @brief 获取所有候选人
     * @return 候选人实体列表
     */
    virtual std::vector<Candidate> getAllCandidates() = 0;

    /**
     * @brief 根据条件查询候选人
     * @param skills 技能，可选
     * @param years_of_experience 工作经验年限，可选
     * @param page 页码，默认1
     * @param page_size 每页数量，默认10
     * @return 候选人实体列表
     */
    virtual std::vector<Candidate> findCandidatesByCondition(const std::optional<std::string>& skills = std::nullopt,
                                                                 const std::optional<int>& years_of_experience = std::nullopt,
                                                                 int page = 1,
                                                                 int page_size = 10) = 0;
};

/**
 * @brief 候选人服务实现类
 */
class CandidateServiceImpl : public CandidateService {
public:
    /**
     * @brief 构造函数
     * @param candidate_dao 候选人DAO
     */
    explicit CandidateServiceImpl(std::shared_ptr<CandidateDAO> candidate_dao);

    /**
     * @brief 析构函数
     */
    virtual ~CandidateServiceImpl() = default;

    /**
     * @brief 创建候选人档案
     * @param candidate 候选人实体
     * @return 创建成功返回候选人ID，否则返回-1
     */
    long long createCandidate(const Candidate& candidate) override;

    /**
     * @brief 根据ID获取候选人
     * @param id 候选人ID
     * @return 候选人实体，如果不存在则返回空
     */
    std::optional<Candidate> getCandidateById(long long id) override;

    /**
     * @brief 更新候选人信息
     * @param candidate 候选人实体
     * @return 更新成功返回true，否则返回false
     */
    bool updateCandidate(const Candidate& candidate) override;

    /**
     * @brief 根据ID删除候选人
     * @param id 候选人ID
     * @return 删除成功返回true，否则返回false
     */
    bool deleteCandidateById(long long id) override;

    /**
     * @brief 获取所有候选人
     * @return 候选人实体列表
     */
    std::vector<Candidate> getAllCandidates() override;

    /**
     * @brief 根据条件查询候选人
     * @param skills 技能，可选
     * @param years_of_experience 工作经验年限，可选
     * @param page 页码，默认1
     * @param page_size 每页数量，默认10
     * @return 候选人实体列表
     */
    std::vector<Candidate> findCandidatesByCondition(const std::optional<std::string>& skills = std::nullopt,
                                                         const std::optional<int>& years_of_experience = std::nullopt,
                                                         int page = 1,
                                                         int page_size = 10) override;

private:
};

} // namespace recruitment

#endif // CANDIDATE_SERVICE_H