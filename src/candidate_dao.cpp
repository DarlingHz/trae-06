#include "candidate.h"
#include "database.h"
#include "log.h"
#include <sstream>
#include <vector>

namespace recruitment {

// CandidateDAO 实现


CandidateDAO::~CandidateDAO() {
    LOG_DEBUG("CandidateDAO destroyed");
}

long long CandidateDAO::create(const Candidate& candidate) {
    std::stringstream ss1;
    ss1 << "Creating candidate: " << candidate.getName();
    LOG_DEBUG(ss1.str());

    std::string sql = "INSERT INTO candidates (name, contact, resume, skills, years_of_experience, created_at, updated_at) "
                      "VALUES (?, ?, ?, ?, ?, datetime('now'), datetime('now'));";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(candidate.getName()));
    parameters.push_back(QueryParameter(candidate.getContact()));
    parameters.push_back(QueryParameter(candidate.getResume()));
    parameters.push_back(QueryParameter(candidate.getSkills()));
    parameters.push_back(QueryParameter(static_cast<long long>(candidate.getYearsOfExperience())));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        long long candidate_id = result.last_insert_id;
        std::stringstream ss2;
        ss2 << "Candidate created successfully with ID: " << candidate_id;
        LOG_INFO(ss2.str());
        return candidate_id;
    } catch (const std::exception& e) {
        std::stringstream ss3;
        ss3 << "Failed to create candidate: " << e.what();
        LOG_ERROR(ss3.str());
        throw;
    }
}

std::optional<Candidate> CandidateDAO::getById(long long id) {
    std::stringstream ss1;
    ss1 << "Getting candidate by ID: " << id;
    LOG_DEBUG(ss1.str());

    std::string sql = "SELECT * FROM candidates WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(id));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        if (result.rows.empty()) {
            std::stringstream ss2;
            ss2 << "Candidate not found with ID: " << id;
            LOG_DEBUG(ss2.str());
            return std::nullopt;
        }

        const QueryRow& row = result.rows[0];
        Candidate candidate;
        candidate.setId(row["id"].int_value);
        candidate.setName(row["name"].text_value);
        candidate.setContact(row["contact"].text_value);
        candidate.setResume(row["resume"].text_value);
        candidate.setSkills(row["skills"].text_value);
        candidate.setYearsOfExperience(row["years_of_experience"].int_value);
        candidate.setCreatedAt(row["created_at"].text_value);
        candidate.setUpdatedAt(row["updated_at"].text_value);

        std::stringstream ss3;
        ss3 << "Candidate found: " << candidate.getName();
        LOG_DEBUG(ss3.str());
        return candidate;
    } catch (const std::exception& e) {
        std::stringstream ss4;
        ss4 << "Failed to get candidate by ID: " << e.what();
        LOG_ERROR(ss4.str());
        throw;
    }
}

bool CandidateDAO::update(const Candidate& candidate) {
    std::stringstream ss1;
    ss1 << "Updating candidate: " << candidate.getName();
    LOG_DEBUG(ss1.str());

    std::string sql = "UPDATE candidates SET name = ?, contact = ?, resume = ?, skills = ?, years_of_experience = ?, updated_at = datetime('now') "
                      "WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(candidate.getName()));
    parameters.push_back(QueryParameter(candidate.getContact()));
    parameters.push_back(QueryParameter(candidate.getResume()));
    parameters.push_back(QueryParameter(candidate.getSkills()));
    parameters.push_back(QueryParameter(static_cast<long long>(candidate.getYearsOfExperience())));
    parameters.push_back(QueryParameter(candidate.getId()));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        int rows_affected = connection->executeNonQuery(sql, parameters);
        if (rows_affected == 0) {
            std::stringstream ss2;
            ss2 << "Candidate not found for update: " << candidate.getId();
            LOG_DEBUG(ss2.str());
            return false;
        }

        std::stringstream ss3;
        ss3 << "Candidate updated successfully: " << candidate.getId();
        LOG_INFO(ss3.str());
        return true;
    } catch (const std::exception& e) {
        std::stringstream ss4;
        ss4 << "Failed to update candidate: " << e.what();
        LOG_ERROR(ss4.str());
        throw;
    }
}

bool CandidateDAO::deleteById(long long id) {
    std::stringstream ss1;
    ss1 << "Deleting candidate by ID: " << id;
    LOG_DEBUG(ss1.str());

    std::string sql = "DELETE FROM candidates WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(id));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        int rows_affected = connection->executeNonQuery(sql, parameters);
        if (rows_affected == 0) {
            std::stringstream ss2;
            ss2 << "Candidate not found for deletion: " << id;
            LOG_DEBUG(ss2.str());
            return false;
        }

        std::stringstream ss3;
        ss3 << "Candidate deleted successfully: " << id;
        LOG_INFO(ss3.str());
        return true;
    } catch (const std::exception& e) {
        std::stringstream ss4;
        ss4 << "Failed to delete candidate by ID: " << e.what();
        LOG_ERROR(ss4.str());
        throw;
    }
}

std::vector<Candidate> CandidateDAO::getAll() {
    LOG_DEBUG("Getting all candidates");

    std::string sql = "SELECT * FROM candidates ORDER BY created_at DESC;";

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, {});
        std::vector<Candidate> candidates;

        for (const QueryRow& row : result.rows) {
            Candidate candidate;
            candidate.setId(row["id"].int_value);
            candidate.setName(row["name"].text_value);
            candidate.setContact(row["contact"].text_value);
            candidate.setResume(row["resume"].text_value);
            candidate.setSkills(row["skills"].text_value);
            candidate.setYearsOfExperience(row["years_of_experience"].int_value);
            candidate.setCreatedAt(row["created_at"].text_value);
            candidate.setUpdatedAt(row["updated_at"].text_value);

            candidates.push_back(candidate);
        }

        std::stringstream ss1;
        ss1 << "Found " << candidates.size() << " candidates";
        LOG_DEBUG(ss1.str());
        return candidates;
    } catch (const std::exception& e) {
        std::stringstream ss2;
        ss2 << "Failed to get all candidates: " << e.what();
        LOG_ERROR(ss2.str());
        throw;
    }
}

std::vector<Candidate> CandidateDAO::findByCondition(const std::optional<std::string>& skills,
                                                         const std::optional<int>& years_of_experience,
                                                         int page,
                                                         int page_size) {
    LOG_DEBUG("Finding candidates by condition");

    std::ostringstream sql;
    std::vector<QueryParameter> parameters;

    sql << "SELECT * FROM candidates WHERE 1=1";

    if (skills) {
        sql << " AND skills LIKE ?";
        parameters.push_back(QueryParameter("%" + skills.value() + "%"));
    }

    if (years_of_experience) {
        sql << " AND years_of_experience >= ?";
        parameters.push_back(QueryParameter(static_cast<long long>(years_of_experience.value())));
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
        std::vector<Candidate> candidates;

        for (const QueryRow& row : result.rows) {
            Candidate candidate;
            candidate.setId(row["id"].int_value);
            candidate.setName(row["name"].text_value);
            candidate.setContact(row["contact"].text_value);
            candidate.setResume(row["resume"].text_value);
            candidate.setSkills(row["skills"].text_value);
            candidate.setYearsOfExperience(row["years_of_experience"].int_value);
            candidate.setCreatedAt(row["created_at"].text_value);
            candidate.setUpdatedAt(row["updated_at"].text_value);

            candidates.push_back(candidate);
        }

        std::stringstream ss1;
        ss1 << "Found " << candidates.size() << " candidates matching condition";
        LOG_DEBUG(ss1.str());
        return candidates;
    } catch (const std::exception& e) {
        std::stringstream ss2;
        ss2 << "Failed to find candidates by condition: " << e.what();
        LOG_ERROR(ss2.str());
        throw;
    }
}

} // namespace recruitment