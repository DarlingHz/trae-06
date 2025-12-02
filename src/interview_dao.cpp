#include "interview.h"
#include "database.h"
#include "log.h"
#include <sstream>
#include <vector>

namespace recruitment {

// InterviewDAO 实现

long long InterviewDAO::create(const Interview& interview) {
    LOG_DEBUG("Creating interview for application ID: " + std::to_string(interview.getApplicationId()));

    std::string sql = "INSERT INTO interviews (application_id, scheduled_time, interviewer_name, mode, location, note, status, created_at, updated_at) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, datetime('now'), datetime('now'));";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(interview.getApplicationId()));
    parameters.push_back(QueryParameter(interview.getScheduledTime()));
    parameters.push_back(QueryParameter(interview.getInterviewerName()));
    parameters.push_back(QueryParameter(interview.getMode()));
    parameters.push_back(QueryParameter(interview.getLocationOrLink()));
    parameters.push_back(QueryParameter(interview.getNote()));
    parameters.push_back(QueryParameter(interview.getStatus()));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        long long interview_id = result.last_insert_id;
        LOG_INFO("Interview created successfully with ID: " + std::to_string(interview_id));
        return interview_id;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create interview: " + std::string(e.what()));
        throw;
    }
}

std::optional<Interview> InterviewDAO::getById(long long id) {
    LOG_DEBUG("Getting interview by ID: " + std::to_string(id));

    std::string sql = "SELECT * FROM interviews WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(id));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        if (result.rows.empty()) {
            LOG_DEBUG("Interview not found with ID: " + std::to_string(id));
            return std::nullopt;
        }

        const QueryRow& row = result.rows[0];
        Interview interview;
        interview.setId(row["id"].int_value);
        interview.setApplicationId(row["application_id"].int_value);
        interview.setScheduledTime(row["scheduled_time"].text_value);
        interview.setInterviewerName(row["interviewer_name"].text_value);
        interview.setMode(row["mode"].text_value);
        interview.setLocationOrLink(row["location"].text_value);
        interview.setNote(row["note"].text_value);
        interview.setStatus(row["status"].text_value);
        interview.setCreatedAt(row["created_at"].text_value);
        interview.setUpdatedAt(row["updated_at"].text_value);

        LOG_DEBUG("Interview found: ID " + std::to_string(interview.getId()));
        return interview;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get interview by ID: " + std::string(e.what()));
        throw;
    }
}

bool InterviewDAO::update(const Interview& interview) {
    LOG_DEBUG("Updating interview: ID " + std::to_string(interview.getId()));

    std::string sql = "UPDATE interviews SET application_id = ?, scheduled_time = ?, interviewer_name = ?, mode = ?, location = ?, note = ?, status = ?, updated_at = datetime('now') "
                      "WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(interview.getApplicationId()));
    parameters.push_back(QueryParameter(interview.getScheduledTime()));
    parameters.push_back(QueryParameter(interview.getInterviewerName()));
    parameters.push_back(QueryParameter(interview.getMode()));
    parameters.push_back(QueryParameter(interview.getLocationOrLink()));
    parameters.push_back(QueryParameter(interview.getNote()));
    parameters.push_back(QueryParameter(interview.getStatus()));
    parameters.push_back(QueryParameter(interview.getId()));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        int rows_affected = connection->executeNonQuery(sql, parameters);
        if (rows_affected == 0) {
            LOG_DEBUG("Interview not found for update: " + std::to_string(interview.getId()));
            return false;
        }

        LOG_INFO("Interview updated successfully: ID " + std::to_string(interview.getId()));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update interview: " + std::string(e.what()));
        throw;
    }
}

bool InterviewDAO::deleteById(long long id) {
    LOG_DEBUG("Deleting interview by ID: " + std::to_string(id));

    std::string sql = "DELETE FROM interviews WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(id));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        int rows_affected = connection->executeNonQuery(sql, parameters);
        if (rows_affected == 0) {
            LOG_DEBUG("Interview not found for deletion: " + std::to_string(id));
            return false;
        }

        LOG_INFO("Interview deleted successfully: ID " + std::to_string(id));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to delete interview by ID: " + std::string(e.what()));
        throw;
    }
}

std::vector<Interview> InterviewDAO::getAll() {
    LOG_DEBUG("Getting all interviews");

    std::string sql = "SELECT * FROM interviews ORDER BY created_at DESC;";

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, {});
        std::vector<Interview> interviews;

        for (const QueryRow& row : result.rows) {
            Interview interview;
            interview.setId(row["id"].int_value);
            interview.setApplicationId(row["application_id"].int_value);
            interview.setScheduledTime(row["scheduled_time"].text_value);
            interview.setInterviewerName(row["interviewer_name"].text_value);
            interview.setMode(row["mode"].text_value);
            interview.setLocationOrLink(row["location"].text_value);
            interview.setNote(row["note"].text_value);
            interview.setStatus(row["status"].text_value);
            interview.setCreatedAt(row["created_at"].text_value);
            interview.setUpdatedAt(row["updated_at"].text_value);

            interviews.push_back(interview);
        }

        LOG_DEBUG("Found " + std::to_string(interviews.size()) + " interviews");
        return interviews;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get all interviews: " + std::string(e.what()));
        throw;
    }
}

std::vector<Interview> InterviewDAO::findByCondition(const std::optional<long long>& application_id,
                                                         const std::optional<long long>& candidate_id,
                                                         const std::optional<std::string>& status,
                                                         int page,
                                                         int page_size) {
    LOG_DEBUG("Finding interviews by condition");

    std::ostringstream sql;
    std::vector<QueryParameter> parameters;

    // 使用 JOIN 来支持通过 candidate_id 查询
    sql << "SELECT i.* FROM interviews i "
        << "LEFT JOIN applications a ON i.application_id = a.id "
        << "WHERE 1=1";

    if (application_id) {
        sql << " AND i.application_id = ?";
        parameters.push_back(QueryParameter(static_cast<long long>(application_id.value())));
    }

    if (candidate_id) {
        sql << " AND a.candidate_id = ?";
        parameters.push_back(QueryParameter(static_cast<long long>(candidate_id.value())));
    }

    if (status) {
        sql << " AND i.status = ?";
        parameters.push_back(QueryParameter(status.value()));
    }

    sql << " ORDER BY i.created_at DESC";

    // 添加分页
    if (page > 0 && page_size > 0) {
        int offset = (page - 1) * page_size;
        sql << " LIMIT ? OFFSET ?";
        parameters.push_back(QueryParameter(static_cast<long long>(page_size)));
        parameters.push_back(QueryParameter(static_cast<long long>(offset)));
    }

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql.str(), parameters);
        std::vector<Interview> interviews;

        for (const QueryRow& row : result.rows) {
            Interview interview;
            interview.setId(row["id"].int_value);
            interview.setApplicationId(row["application_id"].int_value);
            interview.setScheduledTime(row["scheduled_time"].text_value);
            interview.setInterviewerName(row["interviewer_name"].text_value);
            interview.setMode(row["mode"].text_value);
            interview.setLocationOrLink(row["location"].text_value);
            interview.setNote(row["note"].text_value);
            interview.setStatus(row["status"].text_value);
            interview.setCreatedAt(row["created_at"].text_value);
            interview.setUpdatedAt(row["updated_at"].text_value);

            interviews.push_back(interview);
        }

        LOG_DEBUG("Found " + std::to_string(interviews.size()) + " interviews matching condition");
        return interviews;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to find interviews by condition: " + std::string(e.what()));
        throw;
    }
}

// Evaluation 相关方法

long long InterviewDAO::createEvaluation(const Evaluation& evaluation) {
    LOG_DEBUG("Creating evaluation for interview ID: " + std::to_string(evaluation.getInterviewId()));

    std::string sql = "INSERT INTO evaluations (application_id, interview_id, score, comment, created_at, updated_at, evaluator) "
                      "VALUES (?, ?, ?, ?, datetime('now'), datetime('now'), ?);";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(evaluation.getApplicationId()));
    parameters.push_back(QueryParameter(evaluation.getInterviewId()));
    parameters.push_back(QueryParameter(static_cast<long long>(evaluation.getScore())));
    parameters.push_back(QueryParameter(evaluation.getComment()));
    parameters.push_back(QueryParameter(evaluation.getEvaluator()));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        long long evaluation_id = result.last_insert_id;
        LOG_INFO("Evaluation created successfully with ID: " + std::to_string(evaluation_id));
        return evaluation_id;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create evaluation: " + std::string(e.what()));
        throw;
    }
}

std::optional<Evaluation> InterviewDAO::getEvaluationById(long long id) {
    LOG_DEBUG("Getting evaluation by ID: " + std::to_string(id));

    std::string sql = "SELECT * FROM evaluations WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(id));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        if (result.rows.empty()) {
            LOG_DEBUG("Evaluation not found with ID: " + std::to_string(id));
            return std::nullopt;
        }

        const QueryRow& row = result.rows[0];
        Evaluation evaluation;
        evaluation.setId(row["id"].int_value);
        evaluation.setApplicationId(row["application_id"].int_value);
        evaluation.setInterviewId(row["interview_id"].int_value);
        evaluation.setScore(row["score"].int_value);
        evaluation.setComment(row["comment"].text_value);
        evaluation.setCreatedAt(row["created_at"].text_value);
        evaluation.setEvaluator(row["evaluator"].text_value);

        LOG_DEBUG("Evaluation found: ID " + std::to_string(evaluation.getId()));
        return evaluation;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get evaluation by ID: " + std::string(e.what()));
        throw;
    }
}

bool InterviewDAO::updateEvaluation(const Evaluation& evaluation) {
    LOG_DEBUG("Updating evaluation: ID " + std::to_string(evaluation.getId()));

    std::string sql = "UPDATE evaluations SET application_id = ?, interview_id = ?, score = ?, comment = ?, evaluator = ?, updated_at = datetime('now') "
                      "WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(evaluation.getApplicationId()));
    parameters.push_back(QueryParameter(evaluation.getInterviewId()));
    parameters.push_back(QueryParameter(static_cast<long long>(evaluation.getScore())));
    parameters.push_back(QueryParameter(evaluation.getComment()));
    parameters.push_back(QueryParameter(evaluation.getEvaluator()));
    parameters.push_back(QueryParameter(evaluation.getId()));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        int rows_affected = connection->executeNonQuery(sql, parameters);
        if (rows_affected == 0) {
            LOG_DEBUG("Evaluation not found for update: " + std::to_string(evaluation.getId()));
            return false;
        }

        LOG_INFO("Evaluation updated successfully: ID " + std::to_string(evaluation.getId()));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update evaluation: " + std::string(e.what()));
        throw;
    }
}

bool InterviewDAO::deleteEvaluationById(long long id) {
    LOG_DEBUG("Deleting evaluation by ID: " + std::to_string(id));

    std::string sql = "DELETE FROM evaluations WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(id));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        int rows_affected = connection->executeNonQuery(sql, parameters);
        if (rows_affected == 0) {
            LOG_DEBUG("Evaluation not found for deletion: " + std::to_string(id));
            return false;
        }

        LOG_INFO("Evaluation deleted successfully: ID " + std::to_string(id));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to delete evaluation by ID: " + std::string(e.what()));
        throw;
    }
}

std::vector<Evaluation> InterviewDAO::findEvaluationsByInterviewId(long long interview_id) {
    LOG_DEBUG("Getting evaluations by interview ID: " + std::to_string(interview_id));

    std::string sql = "SELECT * FROM evaluations WHERE interview_id = ? ORDER BY created_at DESC;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(interview_id));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        std::vector<Evaluation> evaluations;

        for (const QueryRow& row : result.rows) {
            Evaluation evaluation;
            evaluation.setId(row["id"].int_value);
            evaluation.setApplicationId(row["application_id"].int_value);
            evaluation.setInterviewId(row["interview_id"].int_value);
            evaluation.setScore(row["score"].int_value);
            evaluation.setComment(row["comment"].text_value);
            evaluation.setCreatedAt(row["created_at"].text_value);
            evaluation.setEvaluator(row["evaluator"].text_value);

            evaluations.push_back(evaluation);
        }

        LOG_DEBUG("Found " + std::to_string(evaluations.size()) + " evaluations for interview ID " + std::to_string(interview_id));
        return evaluations;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get evaluations by interview ID: " + std::string(e.what()));
        throw;
    }
}

std::vector<Evaluation> InterviewDAO::findEvaluationsByApplicationId(long long application_id) {
    LOG_DEBUG("Getting evaluations by application ID: " + std::to_string(application_id));

    std::string sql = "SELECT * FROM evaluations WHERE application_id = ? ORDER BY created_at DESC;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(application_id));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        std::vector<Evaluation> evaluations;

        for (const QueryRow& row : result.rows) {
            Evaluation evaluation;
            evaluation.setId(row["id"].int_value);
            evaluation.setApplicationId(row["application_id"].int_value);
            evaluation.setInterviewId(row["interview_id"].int_value);
            evaluation.setScore(row["score"].int_value);
            evaluation.setComment(row["comment"].text_value);
            evaluation.setCreatedAt(row["created_at"].text_value);
            evaluation.setEvaluator(row["evaluator"].text_value);

            evaluations.push_back(evaluation);
        }

        LOG_DEBUG("Found " + std::to_string(evaluations.size()) + " evaluations for application ID " + std::to_string(application_id));
        return evaluations;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get evaluations by application ID: " + std::string(e.what()));
        throw;
    }
}

} // namespace recruitment