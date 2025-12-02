#include "company.h"
#include "database.h"
#include "log.h"
#include <sstream>
#include <vector>

namespace recruitment {

// CompanyDAO 实现

// CompanyDAO::CompanyDAO() {
//     LOG_DEBUG("CompanyDAO initialized");
// }

// CompanyDAO::~CompanyDAO() {
//     LOG_DEBUG("CompanyDAO destroyed");
// }

long long CompanyDAO::create(const Company& company) {
    std::stringstream ss1;
    ss1 << "Creating company: " << company.getName();
    LOG_DEBUG(ss1.str());

    std::string sql = "INSERT INTO companies (name, industry, location, description, created_at, updated_at) "
                      "VALUES (?, ?, ?, ?, datetime('now'), datetime('now'));";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(company.getName()));
    parameters.push_back(QueryParameter(company.getIndustry()));
    parameters.push_back(QueryParameter(company.getLocation()));
    parameters.push_back(QueryParameter(company.getDescription()));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        long long company_id = result.last_insert_id;
        std::stringstream ss2;
        ss2 << "Company created successfully with ID: " << company_id;
        LOG_INFO(ss2.str());
        return company_id;
    } catch (const std::exception& e) {
        std::stringstream ss3;
        ss3 << "Failed to create company: " << e.what();
        LOG_ERROR(ss3.str());
        throw;
    }
}

std::optional<Company> CompanyDAO::getById(long long id) {
    std::stringstream ss1;
    ss1 << "Getting company by ID: " << id;
    LOG_DEBUG(ss1.str());

    std::string sql = "SELECT * FROM companies WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(id));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        if (result.rows.empty()) {
            std::stringstream ss2;
            ss2 << "Company not found with ID: " << id;
            LOG_DEBUG(ss2.str());
            return std::nullopt;
        }

        const QueryRow& row = result.rows[0];
        Company company;
        company.setId(row["id"].int_value);
        company.setName(row["name"].text_value);
        company.setIndustry(row["industry"].text_value);
        company.setLocation(row["location"].text_value);
        company.setDescription(row["description"].text_value);
        company.setCreatedAt(row["created_at"].text_value);
        company.setUpdatedAt(row["updated_at"].text_value);

        std::stringstream ss3;
        ss3 << "Company found: " << company.getName();
        LOG_DEBUG(ss3.str());
        return company;
    } catch (const std::exception& e) {
        std::stringstream ss4;
        ss4 << "Failed to get company by ID: " << e.what();
        LOG_ERROR(ss4.str());
        throw;
    }
}

bool CompanyDAO::update(const Company& company) {
    std::stringstream ss1;
    ss1 << "Updating company: " << company.getName();
    LOG_DEBUG(ss1.str());

    std::string sql = "UPDATE companies SET name = ?, industry = ?, location = ?, description = ?, updated_at = datetime('now') "
                      "WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(company.getName()));
    parameters.push_back(QueryParameter(company.getIndustry()));
    parameters.push_back(QueryParameter(company.getLocation()));
    parameters.push_back(QueryParameter(company.getDescription()));
    parameters.push_back(QueryParameter(company.getId()));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        int rows_affected = connection->executeNonQuery(sql, parameters);
        if (rows_affected == 0) {
            std::stringstream ss2;
            ss2 << "Company not found for update: " << company.getId();
            LOG_DEBUG(ss2.str());
            return false;
        }

        std::stringstream ss3;
        ss3 << "Company updated successfully: " << company.getId();
        LOG_INFO(ss3.str());
        return true;
    } catch (const std::exception& e) {
        std::stringstream ss4;
        ss4 << "Failed to update company: " << e.what();
        LOG_ERROR(ss4.str());
        throw;
    }
}

bool CompanyDAO::deleteById(long long id) {
    std::stringstream ss1;
    ss1 << "Deleting company by ID: " << id;
    LOG_DEBUG(ss1.str());

    std::string sql = "DELETE FROM companies WHERE id = ?;";

    std::vector<QueryParameter> parameters;
    parameters.push_back(QueryParameter(id));

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        int rows_affected = connection->executeNonQuery(sql, parameters);
        if (rows_affected == 0) {
            std::stringstream ss2;
            ss2 << "Company not found for deletion: " << id;
            LOG_DEBUG(ss2.str());
            return false;
        }

        std::stringstream ss3;
        ss3 << "Company deleted successfully: " << id;
        LOG_INFO(ss3.str());
        return true;
    } catch (const std::exception& e) {
        std::stringstream ss4;
        ss4 << "Failed to delete company by ID: " << e.what();
        LOG_ERROR(ss4.str());
        throw;
    }
}

std::vector<Company> CompanyDAO::getAll() {
    LOG_DEBUG("Getting all companies");

    std::string sql = "SELECT * FROM companies ORDER BY created_at DESC;";

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        std::vector<QueryParameter> parameters;
        QueryResult result = connection->executeQuery(sql, parameters);
        std::vector<Company> companies;

        for (const QueryRow& row : result.rows) {
            Company company;
            company.setId(row["id"].int_value);
            company.setName(row["name"].text_value);
            company.setIndustry(row["industry"].text_value);
            company.setLocation(row["location"].text_value);
            company.setDescription(row["description"].text_value);
            company.setCreatedAt(row["created_at"].text_value);
            company.setUpdatedAt(row["updated_at"].text_value);

            companies.push_back(company);
        }

        std::stringstream ss1;
        ss1 << "Found " << companies.size() << " companies";
        LOG_DEBUG(ss1.str());
        return companies;
    } catch (const std::exception& e) {
        std::stringstream ss2;
        ss2 << "Failed to get all companies: " << e.what();
        LOG_ERROR(ss2.str());
        throw;
    }
}

int CompanyDAO::getCompanyCount(const std::optional<std::string>& industry,
                                   const std::optional<std::string>& location) {
    LOG_DEBUG("Getting company count by condition");

    std::string sql = "SELECT COUNT(*) FROM companies WHERE 1=1";
    std::vector<QueryParameter> parameters;

    if (industry) {
        sql += " AND industry LIKE ?";
        parameters.push_back(QueryParameter("%" + industry.value() + "%"));
    }

    if (location) {
        sql += " AND location LIKE ?";
        parameters.push_back(QueryParameter("%" + location.value() + "%"));
    }

    try {
        std::shared_ptr<DatabaseConnection> connection = Database::getConnection();
        QueryResult result = connection->executeQuery(sql, parameters);
        if (result.rows.empty()) {
            return 0;
        }

        const QueryRow& row = result.rows[0];
        int count = row["COUNT(*)"] .int_value;
        return count;
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Failed to get company count: " << e.what();
        LOG_ERROR(ss.str());
        throw;
    }
}

std::vector<Company> CompanyDAO::findByCondition(const std::optional<std::string>& industry,
                                                      const std::optional<std::string>& location,
                                                      int page,
                                                      int page_size) {
    LOG_DEBUG("Finding companies by condition");

    std::ostringstream sql;
    std::vector<QueryParameter> parameters;

    sql << "SELECT * FROM companies WHERE 1=1";

    if (industry) {
        sql << " AND industry LIKE ?";
        parameters.push_back(QueryParameter("%" + industry.value() + "%"));
    }

    if (location) {
        sql << " AND location LIKE ?";
        parameters.push_back(QueryParameter("%" + location.value() + "%"));
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
        std::vector<Company> companies;

        for (const QueryRow& row : result.rows) {
            Company company;
            company.setId(row["id"].int_value);
            company.setName(row["name"].text_value);
            company.setIndustry(row["industry"].text_value);
            company.setLocation(row["location"].text_value);
            company.setDescription(row["description"].text_value);
            company.setCreatedAt(row["created_at"].text_value);
            company.setUpdatedAt(row["updated_at"].text_value);

            companies.push_back(company);
        }

        std::stringstream ss1;
        ss1 << "Found " << companies.size() << " companies matching condition";
        LOG_DEBUG(ss1.str());
        return companies;
    } catch (const std::exception& e) {
        std::stringstream ss2;
        ss2 << "Failed to find companies by condition: " << e.what();
        LOG_ERROR(ss2.str());
        throw;
    }
}

} // namespace recruitment