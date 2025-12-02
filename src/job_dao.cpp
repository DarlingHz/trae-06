#include "job.h"
#include "database.h"
#include "log.h"
#include <sstream>
#include <vector>

namespace recruitment {

// JobDAO 实现

// JobDAO 实现

long long JobDAO::create(const Job& job) {
    LOG_DEBUG("Creating job: " + job.getTitle() + " for company ID: " + std::to_string(job.getCompanyId()));

    std::string sql = "INSERT INTO jobs (company_id, title, location, salary_range, description, required_skills, is_open, created_at, updated_at) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, datetime('now'), datetime('now'));";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(job.getCompanyId()));
    parameters.push_back(QueryParameter(job.getTitle()));
    parameters.push_back(QueryParameter(job.getLocation()));
    parameters.push_back(QueryParameter(job.getSalaryRange()));
    parameters.push_back(QueryParameter(job.getDescription()));
    parameters.push_back(QueryParameter(job.getRequiredSkills()));
    parameters.push_back(QueryParameter(static_cast<long long>(job.isOpen() ? 1 : 0)));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        long long job_id = result.last_insert_id;
        LOG_INFO("Job created successfully with ID: " + std::to_string(job_id));
        return job_id;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create job: " + std::string(e.what()));
        throw;
    }
}

std::optional<Job> JobDAO::getById(long long id) {
    LOG_DEBUG("Getting job by ID: " + std::to_string(id));

    std::string sql = "SELECT * FROM jobs WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(id));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        if (result.rows.empty()) {
            LOG_DEBUG("Job not found with ID: " + std::to_string(id));
            return std::nullopt;
        }

        const QueryRow& row = result.rows[0];
        Job job;
        job.setId(row["id"].int_value);
        job.setCompanyId(row["company_id"].int_value);
        job.setTitle(row["title"].text_value);
        job.setLocation(row["location"].text_value);
        job.setSalaryRange(row["salary_range"].text_value);
        job.setDescription(row["description"].text_value);
        job.setRequiredSkills(row["required_skills"].text_value);
        job.setIsOpen(row["is_open"].int_value == 1);
        job.setCreatedAt(row["created_at"].text_value);
        job.setUpdatedAt(row["updated_at"].text_value);

        LOG_DEBUG("Job found: " + job.getTitle());
        return job;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get job by ID: " + std::string(e.what()));
        throw;
    }
}

bool JobDAO::update(const Job& job) {
    LOG_DEBUG("Updating job: " + job.getTitle());

    std::string sql = "UPDATE jobs SET company_id = ?, title = ?, location = ?, salary_range = ?, description = ?, required_skills = ?, is_open = ?, updated_at = datetime('now') "
                      "WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(job.getCompanyId()));
    parameters.push_back(QueryParameter(job.getTitle()));
    parameters.push_back(QueryParameter(job.getLocation()));
    parameters.push_back(QueryParameter(job.getSalaryRange()));
    parameters.push_back(QueryParameter(job.getDescription()));
    parameters.push_back(QueryParameter(job.getRequiredSkills()));
    parameters.push_back(QueryParameter(static_cast<long long>(job.isOpen() ? 1 : 0)));
    parameters.push_back(QueryParameter(job.getId()));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        int rows_affected = connection->executeNonQuery(sql, parameters);
        if (rows_affected == 0) {
            LOG_DEBUG("Job not found for update: " + std::to_string(job.getId()));
            return false;
        }

        LOG_INFO("Job updated successfully: " + std::to_string(job.getId()));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update job: " + std::string(e.what()));
        throw;
    }
}

bool JobDAO::deleteById(long long id) {
    LOG_DEBUG("Deleting job by ID: " + std::to_string(id));

    std::string sql = "DELETE FROM jobs WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(id));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        int rows_affected = connection->executeNonQuery(sql, parameters);
        if (rows_affected == 0) {
            LOG_DEBUG("Job not found for deletion: " + std::to_string(id));
            return false;
        }

        LOG_INFO("Job deleted successfully: " + std::to_string(id));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to delete job by ID: " + std::string(e.what()));
        throw;
    }
}

std::vector<Job> JobDAO::getAll() {
    LOG_DEBUG("Getting all jobs");

    std::string sql = "SELECT * FROM jobs ORDER BY created_at DESC;";

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, {});
        std::vector<Job> jobs;

        for (const QueryRow& row : result.rows) {
            Job job;
            job.setId(row["id"].int_value);
            job.setCompanyId(row["company_id"].int_value);
            job.setTitle(row["title"].text_value);
            job.setLocation(row["location"].text_value);
            job.setSalaryRange(row["salary_range"].text_value);
            job.setDescription(row["description"].text_value);
            job.setRequiredSkills(row["required_skills"].text_value);
            job.setIsOpen(row["is_open"].int_value == 1);
            job.setCreatedAt(row["created_at"].text_value);
            job.setUpdatedAt(row["updated_at"].text_value);

            jobs.push_back(job);
        }

        LOG_DEBUG("Found " + std::to_string(jobs.size()) + " jobs");
        return jobs;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get all jobs: " + std::string(e.what()));
        throw;
    }
}

std::vector<Job> JobDAO::findByCondition(const std::optional<long long>& company_id,
                                             const std::optional<std::string>& location,
                                             const std::optional<std::string>& required_skills,
                                             const std::optional<bool>& is_open,
                                             int page,
                                             int page_size,
                                             const std::string& sort_by,
                                             const std::string& sort_order) {
    LOG_DEBUG("Finding jobs by condition");

    std::ostringstream sql;
    std::vector<QueryParameter> parameters;

    sql << "SELECT * FROM jobs WHERE 1=1";

    if (company_id) {
        sql << " AND company_id = ?";
        parameters.push_back(QueryParameter(company_id.value()));
    }

    if (location) {
        sql << " AND location LIKE ?";
        parameters.push_back(QueryParameter("%" + location.value() + "%"));
    }

    if (required_skills) {
        sql << " AND required_skills LIKE ?";
        parameters.push_back(QueryParameter("%" + required_skills.value() + "%"));
    }

    if (is_open) {
        sql << " AND is_open = ?";
        parameters.push_back(QueryParameter(static_cast<long long>(is_open.value() ? 1 : 0)));
    }

    // 验证排序字段和顺序
    std::string valid_sort_by = sort_by;
    if (valid_sort_by != "id" && valid_sort_by != "title" && valid_sort_by != "location" &&
        valid_sort_by != "salary_range" && valid_sort_by != "is_open" && valid_sort_by != "created_at" &&
        valid_sort_by != "updated_at") {
        valid_sort_by = "created_at";
    }

    std::string valid_sort_order = sort_order;
    if (valid_sort_order != "ASC" && valid_sort_order != "DESC") {
        valid_sort_order = "DESC";
    }

    sql << " ORDER BY " << valid_sort_by << " " << valid_sort_order;

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
        std::vector<Job> jobs;

        for (const QueryRow& row : result.rows) {
            Job job;
            job.setId(row["id"].int_value);
            job.setCompanyId(row["company_id"].int_value);
            job.setTitle(row["title"].text_value);
            job.setLocation(row["location"].text_value);
            job.setSalaryRange(row["salary_range"].text_value);
            job.setDescription(row["description"].text_value);
            job.setRequiredSkills(row["required_skills"].text_value);
            job.setIsOpen(row["is_open"].int_value == 1);
            job.setCreatedAt(row["created_at"].text_value);
            job.setUpdatedAt(row["updated_at"].text_value);

            jobs.push_back(job);
        }

        LOG_DEBUG("Found " + std::to_string(jobs.size()) + " jobs matching condition");
        return jobs;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to find jobs by condition: " + std::string(e.what()));
        throw;
    }
}

int JobDAO::getJobCount(const std::optional<long long>& company_id,
                         const std::optional<std::string>& location,
                         const std::optional<std::string>& required_skills,
                         const std::optional<bool>& is_open) {
    LOG_DEBUG("Getting job count by condition");

    std::ostringstream sql;
    std::vector<QueryParameter> parameters;

    sql << "SELECT COUNT(*) FROM jobs WHERE 1=1";

    if (company_id) {
        sql << " AND company_id = ?";
        parameters.push_back(QueryParameter(company_id.value()));
    }

    if (location) {
        sql << " AND location LIKE ?";
        parameters.push_back(QueryParameter("%" + location.value() + "%"));
    }

    if (required_skills) {
        sql << " AND required_skills LIKE ?";
        parameters.push_back(QueryParameter("%" + required_skills.value() + "%"));
    }

    if (is_open) {
        sql << " AND is_open = ?";
        parameters.push_back(QueryParameter(static_cast<long long>(is_open.value() ? 1 : 0)));
    }

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql.str(), parameters);
        if (result.rows.empty()) {
            LOG_DEBUG("No jobs found matching condition");
            return 0;
        }

        const QueryRow& row = result.rows[0];
        int count = row["COUNT(*)"] .int_value;

        LOG_DEBUG("Found " + std::to_string(count) + " jobs matching condition");
        return count;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get job count by condition: " + std::string(e.what()));
        throw;
    }
}

} // namespace recruitment