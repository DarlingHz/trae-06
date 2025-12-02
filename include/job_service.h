#ifndef JOB_SERVICE_H
#define JOB_SERVICE_H

#include <string>
#include <optional>
#include <vector>
#include "job.h"

namespace recruitment {

/**
 * @brief 职位服务类
 */
class JobService {
public:
    /**
     * @brief 构造函数
     * @param job_dao 职位DAO对象
     */
    JobService(std::shared_ptr<JobDAO> job_dao);

    /**
     * @brief 析构函数
     */
    virtual ~JobService() = default;

    /**
     * @brief 根据条件获取职位数量
     * @param company_id 公司ID，可选
     * @param location 工作地点，可选
     * @param required_skills 所需技能，可选
     * @param is_open 是否开放招聘，可选
     * @return 职位数量
     */
    virtual int getJobCount(const std::optional<long long>& company_id = std::nullopt,
                             const std::optional<std::string>& location = std::nullopt,
                             const std::optional<std::string>& required_skills = std::nullopt,
                             const std::optional<bool>& is_open = std::nullopt) = 0;

protected:
    std::shared_ptr<JobDAO> job_dao_; ///< 职位DAO对象

    /**
     * @brief 创建职位
     * @param job 职位实体
     * @return 创建成功返回职位ID，否则返回-1
     */
    virtual long long createJob(const Job& job) = 0;

    /**
     * @brief 根据ID获取职位
     * @param id 职位ID
     * @return 职位实体，如果不存在则返回空
     */
    virtual std::optional<Job> getJobById(long long id) = 0;

    /**
     * @brief 更新职位信息
     * @param job 职位实体
     * @return 更新成功返回true，否则返回false
     */
    virtual bool updateJob(const Job& job) = 0;

    /**
     * @brief 根据ID删除职位
     * @param id 职位ID
     * @return 删除成功返回true，否则返回false
     */
    virtual bool deleteJobById(long long id) = 0;

    /**
     * @brief 获取所有职位
     * @param page 页码
     * @param page_size 每页数量
     * @return 职位实体列表
     */
    virtual std::vector<Job> getAllJobs(int page, int page_size) = 0;

    /**
     * @brief 根据条件查询职位
     * @param conditions 查询条件
     * @param page 页码
     * @param page_size 每页大小
     * @return 职位列表
     */
    virtual std::vector<Job> findJobsByCondition(const std::map<std::string, std::string>& conditions, int page = 1, int page_size = 20) = 0;

    /**
     * @brief 开放职位招聘
     * @param job_id 职位ID
     * @return 操作成功返回true，否则返回false
     */
    virtual bool openJob(long long job_id) = 0;

    /**
     * @brief 关闭职位招聘
     * @param job_id 职位ID
     * @return 操作成功返回true，否则返回false
     */
    virtual bool closeJob(long long job_id) = 0;
};

/**
 * @brief 职位服务实现类
 */
class JobServiceImpl : public JobService {
public:
    /**
     * @brief 构造函数
     * @param job_dao 职位DAO对象
     */
    JobServiceImpl(std::shared_ptr<JobDAO> job_dao);

    /**
     * @brief 析构函数
     */
    virtual ~JobServiceImpl() = default;

    /**
     * @brief 创建职位
     * @param job 职位实体
     * @return 创建成功返回职位ID，否则返回-1
     */
    long long createJob(const Job& job) override;

    /**
     * @brief 根据ID获取职位
     * @param id 职位ID
     * @return 职位实体，如果不存在则返回空
     */
    std::optional<Job> getJobById(long long id) override;

    /**
     * @brief 更新职位信息
     * @param job 职位实体
     * @return 更新成功返回true，否则返回false
     */
    bool updateJob(const Job& job) override;

    /**
     * @brief 根据ID删除职位
     * @param id 职位ID
     * @return 删除成功返回true，否则返回false
     */
    bool deleteJobById(long long id) override;

    /**
     * @brief 获取所有职位
     * @param page 页码
     * @param page_size 每页数量
     * @return 职位实体列表
     */
    std::vector<Job> getAllJobs(int page, int page_size) override;

    /**
     * @brief 根据条件查询职位
     * @param conditions 查询条件
     * @param page 页码
     * @param page_size 每页大小
     * @return 职位列表
     */
    std::vector<Job> findJobsByCondition(const std::map<std::string, std::string>& conditions, int page = 1, int page_size = 20) override;

    /**
     * @brief 开放职位招聘
     * @param job_id 职位ID
     * @return 操作成功返回true，否则返回false
     */
    bool openJob(long long job_id) override;

    /**
     * @brief 关闭职位招聘
     * @param job_id 职位ID
     * @return 操作成功返回true，否则返回false
     */
    bool closeJob(long long job_id) override;

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
                     const std::optional<bool>& is_open = std::nullopt) override;

};


} // namespace recruitment

#endif // JOB_SERVICE_H