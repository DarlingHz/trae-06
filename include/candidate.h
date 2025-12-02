#ifndef CANDIDATE_H
#define CANDIDATE_H

#include <string>
#include <optional>
#include <vector>
#include "base_dao.h"

namespace recruitment {

/**
 * @brief 候选人实体类
 */
class Candidate {
public:
    /**
     * @brief 默认构造函数
     */
    Candidate() = default;

    /**
     * @brief 构造函数
     * @param name 候选人姓名
     * @param contact 联系方式
     * @param resume 简历
     * @param skills 技能
     * @param years_of_experience 工作经验年限
     */
    Candidate(const std::string& name,
              const std::string& contact,
              const std::string& resume,
              const std::string& skills,
              int years_of_experience)
        : name_(name), contact_(contact), resume_(resume), skills_(skills),
          years_of_experience_(years_of_experience) {}

    /**
     * @brief 获取候选人ID
     * @return 候选人ID
     */
    long long getId() const { return id_; }

    /**
     * @brief 设置候选人ID
     * @param id 候选人ID
     */
    void setId(long long id) { id_ = id; }

    /**
     * @brief 获取候选人姓名
     * @return 候选人姓名
     */
    const std::string& getName() const { return name_; }

    /**
     * @brief 设置候选人姓名
     * @param name 候选人姓名
     */
    void setName(const std::string& name) { name_ = name; }

    /**
     * @brief 获取联系方式
     * @return 联系方式
     */
    const std::string& getContact() const { return contact_; }

    /**
     * @brief 设置联系方式
     * @param contact 联系方式
     */
    void setContact(const std::string& contact) { contact_ = contact; }

    /**
     * @brief 获取简历
     * @return 简历
     */
    const std::string& getResume() const { return resume_; }

    /**
     * @brief 设置简历
     * @param resume 简历
     */
    void setResume(const std::string& resume) { resume_ = resume; }

    /**
     * @brief 获取技能
     * @return 技能
     */
    const std::string& getSkills() const { return skills_; }

    /**
     * @brief 设置技能
     * @param skills 技能
     */
    void setSkills(const std::string& skills) { skills_ = skills; }

    /**
     * @brief 获取工作经验年限
     * @return 工作经验年限
     */
    int getYearsOfExperience() const { return years_of_experience_; }

    /**
     * @brief 设置工作经验年限
     * @param years_of_experience 工作经验年限
     */
    void setYearsOfExperience(int years_of_experience) { years_of_experience_ = years_of_experience; }

    /**
     * @brief 获取创建时间
     * @return 创建时间
     */
    const std::string& getCreatedAt() const { return created_at_; }

    /**
     * @brief 设置创建时间
     * @param created_at 创建时间
     */
    void setCreatedAt(const std::string& created_at) { created_at_ = created_at; }

    /**
     * @brief 获取更新时间
     * @return 更新时间
     */
    const std::string& getUpdatedAt() const { return updated_at_; }

    /**
     * @brief 设置更新时间
     * @param updated_at 更新时间
     */
    void setUpdatedAt(const std::string& updated_at) { updated_at_ = updated_at; }

private:
    long long id_ = 0; ///< 候选人ID
    std::string name_; ///< 候选人姓名
    std::string contact_; ///< 联系方式
    std::string resume_; ///< 简历
    std::string skills_; ///< 技能
    int years_of_experience_ = 0; ///< 工作经验年限
    std::string created_at_; ///< 创建时间
    std::string updated_at_; ///< 更新时间
};

/**
 * @brief 候选人DAO类
 */
class CandidateDAO : public BaseDAO<Candidate> {
public:
    /**
     * @brief 创建候选人
     * @param candidate 候选人实体
     * @return 创建成功返回候选人ID，否则返回-1
     */
    long long create(const Candidate& candidate) override;

    /**
     * @brief 根据ID获取候选人
     * @param id 候选人ID
     * @return 候选人实体，如果不存在则返回空
     */
    std::optional<Candidate> getById(long long id) override;

    /**
     * @brief 更新候选人
     * @param candidate 候选人实体
     * @return 更新成功返回true，否则返回false
     */
    bool update(const Candidate& candidate) override;

    /**
     * @brief 根据ID删除候选人
     * @param id 候选人ID
     * @return 删除成功返回true，否则返回false
     */
    bool deleteById(long long id) override;

    /**
     * @brief 获取所有候选人
     * @return 候选人实体列表
     */
    std::vector<Candidate> getAll() override;

    /**
     * @brief 根据条件查询候选人
     * @param skills 技能，可选
     * @param years_of_experience 工作经验年限，可选
     * @param page 页码，默认1
     * @param page_size 每页数量，默认10
     * @return 候选人实体列表
     */
    std::vector<Candidate> findByCondition(const std::optional<std::string>& skills = std::nullopt,
                                              const std::optional<int>& years_of_experience = std::nullopt,
                                              int page = 1,
                                              int page_size = 10);

    /**
     * @brief 析构函数
     */
    ~CandidateDAO() override;
};

} // namespace recruitment

#endif // CANDIDATE_H