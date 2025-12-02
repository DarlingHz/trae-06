#include "application.h"
#include "database.h"
#include "log.h"
#include <sstream>
#include <vector>

namespace recruitment {

// ApplicationDAO 实现

ApplicationDAO::ApplicationDAO() {
    LOG_DEBUG("ApplicationDAO initialized");
}

ApplicationDAO::~ApplicationDAO() {
    LOG_DEBUG("ApplicationDAO destroyed");
}

long long ApplicationDAO::create(const Application& application) {
    std::stringstream ss;
    ss << "Creating application: candidate ID " << application.getCandidateId() << ", job ID " << application.getJobId();
    LOG_DEBUG(ss.str());

    std::string sql = "INSERT INTO applications (job_id, candidate_id, status, applied_at, created_at, updated_at) "
                      "VALUES (?, ?, ?, datetime('now'), datetime('now'), datetime('now'));";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(application.getJobId()));
    parameters.push_back(QueryParameter(application.getCandidateId()));
    parameters.push_back(QueryParameter(application.getStatus()));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        long long application_id = result.last_insert_id;
        std::stringstream ss;
        ss << "Application created successfully with ID: " << application_id;
        LOG_INFO(ss.str());
        return application_id;
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Failed to create application: " << e.what();
        LOG_ERROR(ss.str());
        throw;
    }
}

std::optional<Application> ApplicationDAO::getById(long long id) {
    std::stringstream ss;
    ss << "Getting application by ID: " << id;
    LOG_DEBUG(ss.str());

    std::string sql = "SELECT * FROM applications WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(id));

    try {
            std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
            QueryResult result = connection->executeQuery(sql, parameters);
        if (result.rows.empty()) {
            std::stringstream ss;
            ss << "Application not found with ID: " << id;
            LOG_DEBUG(ss.str());
            return std::nullopt;
        }

        const QueryRow& row = result.rows[0];
        Application application;
        application.setId(row["id"].int_value);
        application.setJobId(row["job_id"].int_value);
        application.setCandidateId(row["candidate_id"].int_value);
        application.setStatus(row["status"].text_value);
        application.setAppliedAt(row["applied_at"].text_value);
        application.setCreatedAt(row["created_at"].text_value);
        application.setUpdatedAt(row["updated_at"].text_value);

        std::stringstream ss;
        ss << "Application found: ID " << application.getId();
        LOG_DEBUG(ss.str());
        return application;
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Failed to get application by ID: " << e.what();
        LOG_ERROR(ss.str());
        throw;
    }
}

bool ApplicationDAO::update(const Application& application) {
    std::stringstream ss;
    ss << "Updating application: ID " << application.getId();
    LOG_DEBUG(ss.str());

    std::string sql = "UPDATE applications SET job_id = ?, candidate_id = ?, status = ?, applied_at = ?, updated_at = datetime('now') "
                      "WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(application.getJobId()));
    parameters.push_back(QueryParameter(application.getCandidateId()));
    parameters.push_back(QueryParameter(application.getStatus()));
    parameters.push_back(QueryParameter(application.getAppliedAt()));
    parameters.push_back(QueryParameter(application.getId()));

    try {
            std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
            int rows_affected = connection->executeNonQuery(sql, parameters);
        if (rows_affected == 0) {
            std::stringstream ss;
            ss << "Application not found for update: " << application.getId();
            LOG_DEBUG(ss.str());
            return false;
        }

        std::stringstream ss;
        ss << "Application updated successfully: ID " << application.getId();
        LOG_INFO(ss.str());
        return true;
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Failed to update application: " << e.what();
        LOG_ERROR(ss.str());
        throw;
    }
}

bool ApplicationDAO::deleteById(long long id) {
    std::stringstream ss;
    ss << "Deleting application by ID: " << id;
    LOG_DEBUG(ss.str());

    std::string sql = "DELETE FROM applications WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(id));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        int rows_affected = connection->executeNonQuery(sql, parameters);
        if (rows_affected == 0) {
            std::stringstream ss;
            ss << "Application not found for deletion: " << id;
            LOG_DEBUG(ss.str());
            return false;
        }

        std::stringstream ss;
        ss << "Application deleted successfully: ID " << id;
        LOG_INFO(ss.str());
        return true;
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Failed to delete application by ID: " << e.what();
        LOG_ERROR(ss.str());
        throw;
    }
}

std::vector<Application> ApplicationDAO::getAll() {
    LOG_DEBUG("Getting all applications");

    std::string sql = "SELECT * FROM applications ORDER BY created_at DESC;";

    try {
            std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
            QueryResult result = connection->executeQuery(sql, {});
        std::vector<Application> applications;

        for (const QueryRow& row : result.rows) {
            Application application;
            application.setId(row["id"].int_value);
            application.setJobId(row["job_id"].int_value);
            application.setCandidateId(row["candidate_id"].int_value);
            application.setStatus(row["status"].text_value);
            application.setAppliedAt(row["applied_at"].text_value);
            application.setCreatedAt(row["created_at"].text_value);
            application.setUpdatedAt(row["updated_at"].text_value);

            applications.push_back(application);
        }

        std::stringstream ss1;
        ss1 << "Found " << applications.size() << " applications";
        LOG_DEBUG(ss1.str());
        return applications;
    } catch (const std::exception& e) {
        std::stringstream ss2;
        ss2 << "Failed to get all applications: " << e.what();
        LOG_ERROR(ss2.str());
        throw;
    }
}

std::vector<Application> ApplicationDAO::findByCondition(const std::optional<long long>& job_id,
                                                             const std::optional<long long>& candidate_id,
                                                             const std::optional<std::string>& status,
                                                             int page,
                                                             int page_size) {
    LOG_DEBUG("Finding applications by condition");

    std::ostringstream sql;
    std::vector<QueryParameter> parameters;

    sql << "SELECT * FROM applications WHERE 1=1";

    if (job_id) {
        sql << " AND job_id = ?";
        parameters.push_back(QueryParameter(job_id.value()));
    }

    if (candidate_id) {
        sql << " AND candidate_id = ?";
        parameters.push_back(QueryParameter(candidate_id.value()));
    }

    if (status) {
        sql << " AND status = ?";
        parameters.push_back(QueryParameter(status.value()));
    }

    sql << " ORDER BY created_at DESC";

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
        std::vector<Application> applications;

        for (const QueryRow& row : result.rows) {
            Application application;
            application.setId(row["id"].int_value);
            application.setJobId(row["job_id"].int_value);
            application.setCandidateId(row["candidate_id"].int_value);
            application.setStatus(row["status"].text_value);
            application.setAppliedAt(row["applied_at"].text_value);
            application.setCreatedAt(row["created_at"].text_value);
            application.setUpdatedAt(row["updated_at"].text_value);

            applications.push_back(application);
        }

        std::stringstream ss1;
        ss1 << "Found " << applications.size() << " applications matching condition";
        LOG_DEBUG(ss1.str());
        return applications;
    } catch (const std::exception& e) {
        std::stringstream ss2;
        ss2 << "Failed to find applications by condition: " << e.what();
        LOG_ERROR(ss2.str());
        throw;
    }
}

bool ApplicationDAO::updateStatus(long long application_id, const std::string& new_status) {
    std::stringstream ss1;
    ss1 << "Updating application status: ID " << application_id << ", new status " << new_status;
    LOG_DEBUG(ss1.str());

    std::string sql = "UPDATE applications SET status = ?, updated_at = datetime('now') WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(new_status));
    parameters.push_back(QueryParameter(application_id));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        int rows_affected = connection->executeNonQuery(sql, parameters);
        if (rows_affected == 0) {
            std::stringstream ss2;
            ss2 << "Application not found for status update: " << application_id;
            LOG_DEBUG(ss2.str());
            return false;
        }

        std::stringstream ss3;
        ss3 << "Application status updated successfully: ID " << application_id << ", new status " << new_status;
        LOG_INFO(ss3.str());
        return true;
    } catch (const std::exception& e) {
        std::stringstream ss4;
        ss4 << "Failed to update application status: " << e.what();
        LOG_ERROR(ss4.str());
        throw;
    }
}

long long ApplicationDAO::addStatusHistory(long long application_id, const std::string& from_status, const std::string& to_status) {
    std::stringstream ss1;
    ss1 << "Adding application status history: ID " << application_id << ", from " << from_status << " to " << to_status;
    LOG_DEBUG(ss1.str());

    std::string sql = "INSERT INTO application_status_history (application_id, from_status, to_status, changed_at, created_at, updated_at) "
                      "VALUES (?, ?, ?, datetime('now'), datetime('now'), datetime('now'));";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(application_id));
    parameters.push_back(QueryParameter(from_status));
    parameters.push_back(QueryParameter(to_status));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        long long history_id = result.last_insert_id;
        std::stringstream ss2;
        ss2 << "Application status history added successfully with ID: " << history_id;
        LOG_INFO(ss2.str());
        return history_id;
    } catch (const std::exception& e) {
        std::stringstream ss3;
        ss3 << "Failed to add application status history: " << e.what();
        LOG_ERROR(ss3.str());
        throw;
    }
}

std::vector<ApplicationStatusHistory> ApplicationDAO::getStatusHistory(long long application_id) {
    std::stringstream ss1;
    ss1 << "Getting application status history: ID " << application_id;
    LOG_DEBUG(ss1.str());

    std::string sql = "SELECT * FROM application_status_history WHERE application_id = ? ORDER BY changed_at DESC;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(application_id));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        std::vector<ApplicationStatusHistory> history;

        for (const QueryRow& row : result.rows) {
            ApplicationStatusHistory entry;
            entry.setId(row["id"].int_value);
            entry.setApplicationId(row["application_id"].int_value);
            entry.setFromStatus(row["from_status"].text_value);
            entry.setToStatus(row["to_status"].text_value);
            entry.setChangedAt(row["changed_at"].text_value);
            entry.setCreatedAt(row["created_at"].text_value);
            entry.setUpdatedAt(row["updated_at"].text_value);

            history.push_back(entry);
        }

        std::stringstream ss2;
        ss2 << "Found " << history.size() << " status history entries for application ID " << application_id;
        LOG_DEBUG(ss2.str());
        return history;
    } catch (const std::exception& e) {
        std::stringstream ss3;
        ss3 << "Failed to get application status history: " << e.what();
        LOG_ERROR(ss3.str());
        throw;
    }
}

} // namespace recruitment