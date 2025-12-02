#ifndef INTERVIEW_H
#define INTERVIEW_H

#include <string>
#include <optional>
#include <vector>
#include "base_dao.h"

namespace recruitment {

/**
 * @brief 面试实体类
 */
class Interview {
public:
    /**
     * @brief 默认构造函数
     */
    Interview() = default;

    /**
     * @brief 构造函数
     * @param application_id 投递ID
     * @param scheduled_time 面试时间
     * @param interviewer_name 面试官姓名
     * @param mode 面试模式
     * @param location_or_link 面试地点或链接
     * @param note 面试备注
     * @param status 面试状态
     */
    Interview(long long application_id,
              const std::string& scheduled_time,
              const std::string& interviewer_name,
              const std::string& mode = "online",
              const std::string& location_or_link = "",
              const std::string& note = "",
              const std::string& status = "scheduled")
        : application_id_(application_id), scheduled_time_(scheduled_time),
          interviewer_name_(interviewer_name), mode_(mode),
          location_or_link_(location_or_link), note_(note), status_(status) {}

    /**
     * @brief 获取面试ID
     * @return 面试ID
     */
    long long getId() const { return id_; }

    /**
     * @brief 设置面试ID
     * @param id 面试ID
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
     * @brief 获取面试时间
     * @return 面试时间
     */
    const std::string& getScheduledTime() const { return scheduled_time_; }

    /**
     * @brief 设置面试时间
     * @param scheduled_time 面试时间
     */
    void setScheduledTime(const std::string& scheduled_time) { scheduled_time_ = scheduled_time; }

    /**
     * @brief 获取面试官姓名
     * @return 面试官姓名
     */
    const std::string& getInterviewerName() const { return interviewer_name_; }

    /**
     * @brief 设置面试官姓名
     * @param interviewer_name 面试官姓名
     */
    void setInterviewerName(const std::string& interviewer_name) { interviewer_name_ = interviewer_name; }

    /**
     * @brief 获取面试模式
     * @return 面试模式
     */
    const std::string& getMode() const { return mode_; }

    /**
     * @brief 设置面试模式
     * @param mode 面试模式
     */
    void setMode(const std::string& mode) { mode_ = mode; }

    /**
     * @brief 获取面试地点或链接
     * @return 面试地点或链接
     */
    const std::string& getLocationOrLink() const { return location_or_link_; }

    /**
     * @brief 设置面试地点或链接
     * @param location_or_link 面试地点或链接
     */
    void setLocationOrLink(const std::string& location_or_link) { location_or_link_ = location_or_link; }

    /**
     * @brief 获取面试备注
     * @return 面试备注
     */
    const std::string& getNote() const { return note_; }

    /**
     * @brief 设置面试备注
     * @param note 面试备注
     */
    void setNote(const std::string& note) { note_ = note; }

    /**
     * @brief 获取面试状态
     * @return 面试状态
     */
    const std::string& getStatus() const { return status_; }

    /**
     * @brief 设置面试状态
     * @param status 面试状态
     */
    void setStatus(const std::string& status) { status_ = status; }

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
    long long id_ = 0; ///< 面试ID
    long long application_id_ = 0; ///< 投递ID
    std::string scheduled_time_; ///< 面试时间
    std::string interviewer_name_; ///< 面试官姓名
    std::string mode_ = "online"; ///< 面试模式
    std::string location_or_link_; ///< 面试地点或链接
    std::string note_; ///< 面试备注
    std::string status_ = "scheduled"; ///< 面试状态
    std::string created_at_; ///< 创建时间
    std::string updated_at_; ///< 更新时间
};

/**
 * @brief 评价实体类
 */
class Evaluation {
public:
    /**
     * @brief 默认构造函数
     */
    Evaluation() = default;

    /**
     * @brief 构造函数
     * @param application_id 投递ID
     * @param interview_id 面试ID
     * @param score 评分
     * @param comment 评价内容
     * @param evaluator 评价人
     */
    Evaluation(long long application_id,
              long long interview_id,
              int score,
              const std::string& comment,
              const std::string& evaluator)
        : application_id_(application_id), interview_id_(interview_id),
          score_(score), comment_(comment), evaluator_(evaluator) {}

    /**
     * @brief 获取评价ID
     * @return 评价ID
     */
    long long getId() const { return id_; }

    /**
     * @brief 设置评价ID
     * @param id 评价ID
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
     * @brief 获取面试ID
     * @return 面试ID
     */
    long long getInterviewId() const { return interview_id_; }

    /**
     * @brief 设置面试ID
     * @param interview_id 面试ID
     */
    void setInterviewId(long long interview_id) { interview_id_ = interview_id; }

    /**
     * @brief 获取评分
     * @return 评分
     */
    int getScore() const { return score_; }

    /**
     * @brief 设置评分
     * @param score 评分
     */
    void setScore(int score) { score_ = score; }

    /**
     * @brief 获取评价内容
     * @return 评价内容
     */
    const std::string& getComment() const { return comment_; }

    /**
     * @brief 设置评价内容
     * @param comment 评价内容
     */
    void setComment(const std::string& comment) { comment_ = comment; }

    /**
     * @brief 获取评价人
     * @return 评价人
     */
    const std::string& getEvaluator() const { return evaluator_; }

    /**
     * @brief 设置评价人
     * @param evaluator 评价人
     */
    void setEvaluator(const std::string& evaluator) { evaluator_ = evaluator; }

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

private:
    long long id_ = 0; ///< 评价ID
    long long application_id_ = 0; ///< 投递ID
    long long interview_id_ = 0; ///< 面试ID
    int score_ = 0; ///< 评分
    std::string comment_; ///< 评价内容
    std::string evaluator_; ///< 评价人
    std::string created_at_; ///< 创建时间
};

/**
 * @brief 面试DAO类
 */
class InterviewDAO : public BaseDAO<Interview> {
public:
    /**
     * @brief 创建面试
     * @param interview 面试实体
     * @return 创建成功返回面试ID，否则返回-1
     */
    long long create(const Interview& interview) override;

    /**
     * @brief 根据ID获取面试
     * @param id 面试ID
     * @return 面试实体，如果不存在则返回空
     */
    std::optional<Interview> getById(long long id) override;

    /**
     * @brief 更新面试
     * @param interview 面试实体
     * @return 更新成功返回true，否则返回false
     */
    bool update(const Interview& interview) override;

    /**
     * @brief 根据ID删除面试
     * @param id 面试ID
     * @return 删除成功返回true，否则返回false
     */
    bool deleteById(long long id) override;

    /**
     * @brief 获取所有面试
     * @return 面试实体列表
     */
    std::vector<Interview> getAll() override;

    /**
     * @brief 根据条件查询面试
     * @param application_id 投递ID，可选
     * @param candidate_id 候选人ID，可选
     * @param status 面试状态，可选
     * @param page 页码，可选（0表示不分页）
     * @param page_size 每页大小，可选
     * @return 面试实体列表
     */
    std::vector<Interview> findByCondition(const std::optional<long long>& application_id,
                                             const std::optional<long long>& candidate_id,
                                             const std::optional<std::string>& status,
                                             int page = 0,
                                             int page_size = 0);

    /**
     * @brief 根据投递ID查询面试
     * @param application_id 投递ID
     * @param status 面试状态，可选
     * @return 面试实体列表
     */
    std::vector<Interview> findByApplicationId(long long application_id,
                                                  const std::optional<std::string>& status = std::nullopt);

    /**
     * @brief 根据候选人ID查询面试
     * @param candidate_id 候选人ID
     * @param status 面试状态，可选
     * @return 面试实体列表
     */
    std::vector<Interview> findByCandidateId(long long candidate_id,
                                                const std::optional<std::string>& status = std::nullopt);

    /**
     * @brief 创建评价
     * @param evaluation 评价实体
     * @return 创建成功返回评价ID，否则返回-1
     */
    long long createEvaluation(const Evaluation& evaluation);

    /**
     * @brief 根据ID获取评价
     * @param id 评价ID
     * @return 评价实体，如果不存在则返回空
     */
    std::optional<Evaluation> getEvaluationById(long long id);

    /**
     * @brief 根据投递ID查询评价
     * @param application_id 投递ID
     * @return 评价实体列表
     */
    std::vector<Evaluation> findEvaluationsByApplicationId(long long application_id);

    /**
     * @brief 根据面试ID查询评价
     * @param interview_id 面试ID
     * @return 评价实体列表
     */
    std::vector<Evaluation> findEvaluationsByInterviewId(long long interview_id);

    /**
     * @brief 更新评价
     * @param evaluation 评价实体
     * @return 更新成功返回true，否则返回false
     */
    bool updateEvaluation(const Evaluation& evaluation);

    /**
     * @brief 根据ID删除评价
     * @param id 评价ID
     * @return 删除成功返回true，否则返回false
     */
    bool deleteEvaluationById(long long id);
};

} // namespace recruitment

#endif // INTERVIEW_H