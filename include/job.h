#ifndef JOB_H
#define JOB_H

#include <string>
#include <vector>
#include <optional>
#include "base_dao.h"

namespace recruitment {

/**
 * @brief 职位实体类
 */
class Job {
public:
    /**
     * @brief 默认构造函数
     */
    Job() = default;

    /**
     * @brief 构造函数
     * @param company_id 公司ID
     * @param title 职位名称
     * @param location 工作地点
     * @param salary_range 薪资范围
     * @param description 职位描述
     * @param required_skills 所需技能
     * @param is_open 是否开放招聘
     */
    Job(long long company_id,
        const std::string& title,
        const std::string& location,
        const std::string& salary_range,
        const std::string& description,
        const std::string& required_skills,
        bool is_open)
        : company_id_(company_id), title_(title), location_(location), salary_range_(salary_range),
          description_(description), required_skills_(required_skills), is_open_(is_open) {}

    /**
     * @brief 获取职位ID
     * @return 职位ID
     */
    long long getId() const { return id_; }

    /**
     * @brief 设置职位ID
     * @param id 职位ID
     */
    void setId(long long id) { id_ = id; }

    /**
     * @brief 获取公司ID
     * @return 公司ID
     */
    long long getCompanyId() const { return company_id_; }

    /**
     * @brief 设置公司ID
     * @param company_id 公司ID
     */
    void setCompanyId(long long company_id) { company_id_ = company_id; }

    /**
     * @brief 获取职位名称
     * @return 职位名称
     */
    const std::string& getTitle() const { return title_; }

    /**
     * @brief 设置职位名称
     * @param title 职位名称
     */
    void setTitle(const std::string& title) { title_ = title; }

    /**
     * @brief 获取工作地点
     * @return 工作地点
     */
    const std::string& getLocation() const { return location_; }

    /**
     * @brief 设置工作地点
     * @param location 工作地点
     */
    void setLocation(const std::string& location) { location_ = location; }

    /**
     * @brief 获取薪资范围
     * @return 薪资范围
     */
    const std::string& getSalaryRange() const { return salary_range_; }

    /**
     * @brief 设置薪资范围
     * @param salary_range 薪资范围
     */
    void setSalaryRange(const std::string& salary_range) { salary_range_ = salary_range; }

    /**
     * @brief 获取职位描述
     * @return 职位描述
     */
    const std::string& getDescription() const { return description_; }

    /**
     * @brief 设置职位描述
     * @param description 职位描述
     */
    void setDescription(const std::string& description) { description_ = description; }

    /**
     * @brief 获取所需技能
     * @return 所需技能
     */
    const std::string& getRequiredSkills() const { return required_skills_; }

    /**
     * @brief 设置所需技能
     * @param required_skills 所需技能
     */
    void setRequiredSkills(const std::string& required_skills) { required_skills_ = required_skills; }

    /**
     * @brief 获取是否开放招聘
     * @return 是否开放招聘
     */
    bool isOpen() const { return is_open_; }

    /**
     * @brief 设置是否开放招聘
     * @param is_open 是否开放招聘
     */
    void setIsOpen(bool is_open) { is_open_ = is_open; }

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

    /**
     * @brief 转换为JSON字符串
     * @return JSON字符串
     */
    std::string toJson() const;

private:
    long long id_ = 0; ///< 职位ID
    long long company_id_ = 0; ///< 公司ID
    std::string title_; ///< 职位名称
    std::string location_; ///< 工作地点
    std::string salary_range_; ///< 薪资范围
    std::string description_; ///< 职位描述
    std::string required_skills_; ///< 所需技能
    bool is_open_ = true; ///< 是否开放招聘
    std::string created_at_; ///< 创建时间
    std::string updated_at_; ///< 更新时间
};

/**
 * @brief 职位DAO类
 */
class JobDAO : public BaseDAO<Job> {
public:
    /**
     * @brief 创建职位
     * @param job 职位实体
     * @return 创建成功返回职位ID，否则返回-1
     */
    long long create(const Job& job) override;

    /**
     * @brief 根据ID获取职位
     * @param id 职位ID
     * @return 职位实体，如果不存在则返回空
     */
    std::optional<Job> getById(long long id) override;

    /**
     * @brief 更新职位
     * @param job 职位实体
     * @return 更新成功返回true，否则返回false
     */
    bool update(const Job& job) override;

    /**
     * @brief 根据ID删除职位
     * @param id 职位ID
     * @return 删除成功返回true，否则返回false
     */
    bool deleteById(long long id) override;

    /**
     * @brief 获取所有职位
     * @return 职位实体列表
     */
    std::vector<Job> getAll() override;

    /**
     * @brief 根据条件查询职位
     * @param company_id 公司ID，可选
     * @param location 工作地点，可选
     * @param required_skills 所需技能，可选
     * @param is_open 是否开放招聘，可选
     * @param page 页码，默认1
     * @param page_size 每页数量，默认10
     * @param sort_by 排序字段，默认"created_at"
     * @param sort_order 排序顺序，默认"DESC"
     * @return 职位实体列表
     */
    std::vector<Job> findByCondition(const std::optional<long long>& company_id = std::nullopt,
                                        const std::optional<std::string>& location = std::nullopt,
                                        const std::optional<std::string>& required_skills = std::nullopt,
                                        const std::optional<bool>& is_open = std::nullopt,
                                        int page = 1,
                                        int page_size = 10,
                                        const std::string& sort_by = "created_at",
                                        const std::string& sort_order = "DESC");

    /**
     * @brief 根据条件获取职位数量
     * @param company_id 公司ID，可选
     * @param location 工作地点，可选
     * @param required_skills 所需技能，可选
     * @param is_open 是否开放招聘，可选
     * @return 职位数量
     */
    int getJobCount(const std::optional<long long>& company_id = std::nullopt,
                     const std::optional<std::string>& location = std::nullopt,
                     const std::optional<std::string>& required_skills = std::nullopt,
                     const std::optional<bool>& is_open = std::nullopt);
};

} // namespace recruitment

#endif // JOB_H