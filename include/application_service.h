#ifndef APPLICATION_SERVICE_H
#define APPLICATION_SERVICE_H

#include <string>
#include <optional>
#include <vector>
#include "application.h"

namespace recruitment {

/**
 * @brief 投递服务类
 */
class ApplicationService {
public:
    /**
     * @brief 构造函数
     * @param application_dao 投递DAO
     */
    explicit ApplicationService(std::shared_ptr<ApplicationDAO> application_dao)
        : application_dao_(std::move(application_dao)) {}

    /**
     * @brief 析构函数
     */
    virtual ~ApplicationService() = default;

protected:
    std::shared_ptr<ApplicationDAO> application_dao_; ///< 投递DAO对象

    /**
     * @brief 创建投递
     * @param application 投递实体
     * @return 创建成功返回投递ID，否则返回-1
     */
    virtual long long createApplication(const Application& application) = 0;

    /**
     * @brief 根据ID获取投递
     * @param id 投递ID
     * @return 投递实体，如果不存在则返回空
     */
    virtual std::optional<Application> getApplicationById(long long id) = 0;

    /**
     * @brief 更新投递信息
     * @param application 投递实体
     * @return 更新成功返回true，否则返回false
     */
    virtual bool updateApplication(const Application& application) = 0;

    /**
     * @brief 根据ID删除投递
     * @param id 投递ID
     * @return 删除成功返回true，否则返回false
     */
    virtual bool deleteApplicationById(long long id) = 0;

    /**
     * @brief 获取所有投递
     * @return 投递实体列表
     */
    virtual std::vector<Application> getAllApplications() = 0;

    /**
     * @brief 根据条件查询投递
     * @param job_id 职位ID，可选
     * @param candidate_id 候选人ID，可选
     * @param status 投递状态，可选
     * @param page 页码，默认1
     * @param page_size 每页数量，默认10
     * @return 投递实体列表
     */
    virtual std::vector<Application> findApplicationsByCondition(const std::optional<long long>& job_id = std::nullopt,
                                                                     const std::optional<long long>& candidate_id = std::nullopt,
                                                                     const std::optional<std::string>& status = std::nullopt,
                                                                     int page = 1,
                                                                     int page_size = 10) = 0;

    /**
     * @brief 更新投递状态
     * @param application_id 投递ID
     * @param new_status 新状态
     * @return 更新成功返回true，否则返回false
     */
    virtual bool updateApplicationStatus(long long application_id, const std::string& new_status) = 0;

    /**
     * @brief 获取投递状态变更历史
     * @param application_id 投递ID
     * @return 状态变更历史列表
     */
    virtual std::vector<ApplicationStatusHistory> getApplicationStatusHistory(long long application_id) = 0;

    /**
     * @brief 检查状态流转是否合法
     * @param old_status 旧状态
     * @param new_status 新状态
     * @return 合法返回true，否则返回false
     */
    virtual bool isStatusTransitionValid(const std::string& old_status, const std::string& new_status) = 0;
};

/**
 * @brief 投递服务实现类
 */
class ApplicationServiceImpl : public ApplicationService {
public:
    /**
     * @brief 构造函数
     * @param application_dao 投递DAO
     */
    explicit ApplicationServiceImpl(std::shared_ptr<ApplicationDAO> application_dao);

    /**
     * @brief 析构函数
     */
    virtual ~ApplicationServiceImpl() = default;

    /**
     * @brief 创建投递
     * @param application 投递实体
     * @return 创建成功返回投递ID，否则返回-1
     */
    long long createApplication(const Application& application) override;

    /**
     * @brief 根据ID获取投递
     * @param id 投递ID
     * @return 投递实体，如果不存在则返回空
     */
    std::optional<Application> getApplicationById(long long id) override;

    /**
     * @brief 更新投递信息
     * @param application 投递实体
     * @return 更新成功返回true，否则返回false
     */
    bool updateApplication(const Application& application) override;

    /**
     * @brief 根据ID删除投递
     * @param id 投递ID
     * @return 删除成功返回true，否则返回false
     */
    bool deleteApplicationById(long long id) override;

    /**
     * @brief 获取所有投递
     * @return 投递实体列表
     */
    std::vector<Application> getAllApplications() override;

    /**
     * @brief 根据条件查询投递
     * @param job_id 职位ID，可选
     * @param candidate_id 候选人ID，可选
     * @param status 投递状态，可选
     * @param page 页码，默认1
     * @param page_size 每页数量，默认10
     * @return 投递实体列表
     */
    std::vector<Application> findApplicationsByCondition(const std::optional<long long>& job_id = std::nullopt,
                                                             const std::optional<long long>& candidate_id = std::nullopt,
                                                             const std::optional<std::string>& status = std::nullopt,
                                                             int page = 1,
                                                             int page_size = 10) override;

    /**
     * @brief 更新投递状态
     * @param application_id 投递ID
     * @param new_status 新状态
     * @return 更新成功返回true，否则返回false
     */
    bool updateApplicationStatus(long long application_id, const std::string& new_status) override;

    /**
     * @brief 获取投递状态变更历史
     * @param application_id 投递ID
     * @return 状态变更历史列表
     */
    std::vector<ApplicationStatusHistory> getApplicationStatusHistory(long long application_id) override;

    /**
     * @brief 检查状态流转是否合法
     * @param old_status 旧状态
     * @param new_status 新状态
     * @return 合法返回true，否则返回false
     */
    bool isStatusTransitionValid(const std::string& old_status, const std::string& new_status) override;

private:
};

} // namespace recruitment

#endif // APPLICATION_SERVICE_H