#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <optional>
#include <vector>
#include "base_dao.h"

namespace recruitment {

/**
 * @brief 投递实体类
 */
class Application {
public:
    /**
     * @brief 默认构造函数
     */
    Application() = default;

    /**
     * @brief 构造函数
     * @param job_id 职位ID
     * @param candidate_id 候选人ID
     * @param status 投递状态
     */
    Application(long long job_id,
                long long candidate_id,
                const std::string& status = "applied")
        : job_id_(job_id), candidate_id_(candidate_id), status_(status) {}

    /**
     * @brief 获取投递ID
     * @return 投递ID
     */
    long long getId() const { return id_; }

    /**
     * @brief 设置投递ID
     * @param id 投递ID
     */
    void setId(long long id) { id_ = id; }

    /**
     * @brief 获取职位ID
     * @return 职位ID
     */
    long long getJobId() const { return job_id_; }

    /**
     * @brief 设置职位ID
     * @param job_id 职位ID
     */
    void setJobId(long long job_id) { job_id_ = job_id; }

    /**
     * @brief 获取候选人ID
     * @return 候选人ID
     */
    long long getCandidateId() const { return candidate_id_; }

    /**
     * @brief 设置候选人ID
     * @param candidate_id 候选人ID
     */
    void setCandidateId(long long candidate_id) { candidate_id_ = candidate_id; }

    /**
     * @brief 获取投递状态
     * @return 投递状态
     */
    const std::string& getStatus() const { return status_; }

    /**
     * @brief 设置投递状态
     * @param status 投递状态
     */
    void setStatus(const std::string& status) { status_ = status; }

    /**
     * @brief 获取投递时间
     * @return 投递时间
     */
    const std::string& getAppliedAt() const { return applied_at_; }

    /**
     * @brief 设置投递时间
     * @param applied_at 投递时间
     */
    void setAppliedAt(const std::string& applied_at) { applied_at_ = applied_at; }

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
    long long id_ = 0; ///< 投递ID
    long long job_id_ = 0; ///< 职位ID
    long long candidate_id_ = 0; ///< 候选人ID
    std::string status_ = "applied"; ///< 投递状态
    std::string applied_at_; ///< 投递时间
    std::string created_at_; ///< 创建时间
    std::string updated_at_; ///< 更新时间
};

/**
 * @brief 投递状态变更历史实体类
 */
class ApplicationStatusHistory {
public:
    /**
     * @brief 默认构造函数
     */
    ApplicationStatusHistory() = default;

    /**
     * @brief 构造函数
     * @param application_id 投递ID
     * @param from_status 从状态
     * @param to_status 到状态
     */
    ApplicationStatusHistory(long long application_id,
                             const std::string& from_status,
                             const std::string& to_status)
        : application_id_(application_id), from_status_(from_status), to_status_(to_status) {}

    /**
     * @brief 获取状态变更历史ID
     * @return 状态变更历史ID
     */
    long long getId() const { return id_; }

    /**
     * @brief 设置状态变更历史ID
     * @param id 状态变更历史ID
     */
    void setId(long long id) { id_ = id; }

    /**
     * @brief 获取投递ID
     * @return 投递ID
     */
    long long getApplicationId() const { return application_id_; }

    /**
     * @brief 设置投递ID
     * @param application_id 投递ID
     */
    void setApplicationId(long long application_id) { application_id_ = application_id; }

    /**
     * @brief 获取从状态
     * @return 从状态
     */
    const std::string& getFromStatus() const { return from_status_; }

    /**
     * @brief 设置从状态
     * @param from_status 从状态
     */
    void setFromStatus(const std::string& from_status) { from_status_ = from_status; }

    /**
     * @brief 获取到状态
     * @return 到状态
     */
    const std::string& getToStatus() const { return to_status_; }

    /**
     * @brief 设置到状态
     * @param to_status 到状态
     */
    void setToStatus(const std::string& to_status) { to_status_ = to_status; }

    /**
     * @brief 获取变更时间
     * @return 变更时间
     */
    const std::string& getChangedAt() const { return changed_at_; }

    /**
     * @brief 设置变更时间
     * @param changed_at 变更时间
     */
    void setChangedAt(const std::string& changed_at) { changed_at_ = changed_at; }

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
    long long id_ = 0; ///< 状态变更历史ID
    long long application_id_ = 0; ///< 投递ID
    std::string from_status_; ///< 从状态
    std::string to_status_; ///< 到状态
    std::string changed_at_; ///< 变更时间
    std::string created_at_; ///< 创建时间
    std::string updated_at_; ///< 更新时间
};

/**
 * @brief 投递DAO类
 */
class ApplicationDAO : public BaseDAO<Application> {
public:
    /**
     * @brief 构造函数
     */
    ApplicationDAO();

    /**
     * @brief 析构函数
     */
    ~ApplicationDAO() override;

    /**
     * @brief 创建投递
     * @param application 投递实体
     * @return 创建成功返回投递ID，否则返回-1
     */
    long long create(const Application& application) override;

    /**
     * @brief 根据ID获取投递
     * @param id 投递ID
     * @return 投递实体，如果不存在则返回空
     */
    std::optional<Application> getById(long long id) override;

    /**
     * @brief 更新投递
     * @param application 投递实体
     * @return 更新成功返回true，否则返回false
     */
    bool update(const Application& application) override;

    /**
     * @brief 根据ID删除投递
     * @param id 投递ID
     * @return 删除成功返回true，否则返回false
     */
    bool deleteById(long long id) override;

    /**
     * @brief 获取所有投递
     * @return 投递实体列表
     */
    std::vector<Application> getAll() override;

    /**
     * @brief 根据候选人ID查询投递
     * @param candidate_id 候选人ID
     * @param status 投递状态，可选
     * @param page 页码，默认1
     * @param page_size 每页数量，默认10
     * @return 投递实体列表
     */
    std::vector<Application> findByCandidateId(long long candidate_id,
                                                  const std::optional<std::string>& status = std::nullopt,
                                                  int page = 1,
                                                  int page_size = 10);

    /**
     * @brief 根据职位ID查询投递
     * @param job_id 职位ID
     * @param status 投递状态，可选
     * @param page 页码，默认1
     * @param page_size 每页数量，默认10
     * @return 投递实体列表
     */
    std::vector<Application> findByJobId(long long job_id,
                                            const std::optional<std::string>& status = std::nullopt,
                                            int page = 1,
                                            int page_size = 10);

    /**
     * @brief 更新投递状态
     * @param application_id 投递ID
     * @param new_status 新状态
     * @return 更新成功返回true，否则返回false
     */
    bool updateStatus(long long application_id, const std::string& new_status);

    /**
     * @brief 获取投递状态变更历史
     * @param application_id 投递ID
     * @return 状态变更历史列表
     */
    std::vector<ApplicationStatusHistory> getStatusHistory(long long application_id);

    /**
     * @brief 根据条件查询投递
     * @param job_id 职位ID（可选）
     * @param candidate_id 候选人ID（可选）
     * @param status 投递状态（可选）
     * @param page 页码（可选，默认1）
     * @param page_size 每页数量（可选，默认10）
     * @return 投递对象列表
     */
    std::vector<Application> findByCondition(const std::optional<long long>& job_id,
                                               const std::optional<long long>& candidate_id,
                                               const std::optional<std::string>& status,
                                               int page = 1,
                                               int page_size = 10);

    /**
     * @brief 添加投递状态变更历史
     * @param application_id 投递ID
     * @param from_status 从状态
     * @param to_status 到状态
     * @return 添加成功返回历史记录ID，否则返回-1
     */
    long long addStatusHistory(long long application_id, const std::string& from_status, const std::string& to_status);
};

} // namespace recruitment

#endif // APPLICATION_H