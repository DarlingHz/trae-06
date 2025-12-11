#include "company_service.h"
#include "company.h"
#include "log.h"
#include <algorithm>

namespace recruitment {

// CompanyService 实现

// CompanyService类的构造函数已经在头文件中定义了，所以这里不需要再实现它。

// CompanyServiceImpl 实现

CompanyServiceImpl::CompanyServiceImpl(std::shared_ptr<CompanyDAO> company_dao)
    : CompanyService(std::move(company_dao)) {
    LOG_DEBUG("CompanyServiceImpl initialized");
}

long long CompanyServiceImpl::createCompany(const Company& company) {
    LOG_DEBUG("Creating company: " + company.getName());

    // 验证公司名称是否为空
    if (company.getName().empty()) {
        LOG_ERROR("Company name cannot be empty");
        throw std::invalid_argument("Company name cannot be empty");
    }

    try {
        long long company_id = company_dao_->create(company);
        LOG_INFO("Company created successfully with ID: " + std::to_string(company_id));
        return company_id;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create company: " + std::string(e.what()));
        throw;
    }
}

std::optional<Company> CompanyServiceImpl::getCompanyById(long long id) {
    LOG_DEBUG("Getting company by ID: " + std::to_string(id));

    try {
        std::optional<Company> company = company_dao_->getById(id);
        if (company) {
            LOG_DEBUG("Company found: " + company->getName());
        } else {
            LOG_DEBUG("Company not found with ID: " + std::to_string(id));
        }
        return company;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get company by ID: " + std::string(e.what()));
        throw;
    }
}

bool CompanyServiceImpl::updateCompany(const Company& company) {
    LOG_DEBUG("Updating company: ID " + std::to_string(company.getId()));

    // 验证公司ID是否有效
    if (company.getId() <= 0) {
        LOG_ERROR("Invalid company ID: " + std::to_string(company.getId()));
        throw std::invalid_argument("Invalid company ID");
    }

    // 验证公司名称是否为空
    if (company.getName().empty()) {
        LOG_ERROR("Company name cannot be empty");
        throw std::invalid_argument("Company name cannot be empty");
    }

    try {
        bool success = company_dao_->update(company);
        if (success) {
            LOG_INFO("Company updated successfully: ID " + std::to_string(company.getId()));
        } else {
            LOG_DEBUG("Company not found for update: ID " + std::to_string(company.getId()));
        }
        return success;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update company: " + std::string(e.what()));
        throw;
    }
}

bool CompanyServiceImpl::deleteCompanyById(long long id) {
    LOG_DEBUG("Deleting company by ID: " + std::to_string(id));

    // 验证公司ID是否有效
    if (id <= 0) {
        LOG_ERROR("Invalid company ID: " + std::to_string(id));
        throw std::invalid_argument("Invalid company ID");
    }

    try {
        bool success = company_dao_->deleteById(id);
        if (success) {
            LOG_INFO("Company deleted successfully: ID " + std::to_string(id));
        } else {
            LOG_DEBUG("Company not found for deletion: ID " + std::to_string(id));
        }
        return success;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to delete company by ID: " + std::string(e.what()));
        throw;
    }
}

std::vector<Company> CompanyServiceImpl::getAllCompanies() {
    LOG_DEBUG("Getting all companies");

    try {
        std::vector<Company> companies = company_dao_->getAll();
        LOG_DEBUG("Found " + std::to_string(companies.size()) + " companies");
        return companies;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get all companies: " + std::string(e.what()));
        throw;
    }
}

std::vector<Company> CompanyServiceImpl::findCompaniesByCondition(const std::optional<std::string>& industry,
                                                                   const std::optional<std::string>& location,
                                                                   int page,
                                                                   int page_size) {
    LOG_DEBUG("Finding companies by condition");

    // 验证分页参数
    if (page <= 0) {
        LOG_WARN("Invalid page number, using default: 1");
        page = 1;
    }

    if (page_size <= 0 || page_size > 100) {
        LOG_WARN("Invalid page size, using default: 20");
        page_size = 20;
    }

    try {
        std::vector<Company> companies = company_dao_->findByCondition(industry, location, page, page_size);
        LOG_DEBUG("Found " + std::to_string(companies.size()) + " companies matching condition");
        return companies;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to find companies by condition: " + std::string(e.what()));
        throw;
    }
}

int CompanyServiceImpl::getCompanyCount(const std::optional<std::string>& industry,
                                      const std::optional<std::string>& location) {
    LOG_DEBUG("Getting company count by condition");

    try {
        int count = company_dao_->getCompanyCount(industry, location);
        LOG_DEBUG("Found " + std::to_string(count) + " companies matching condition");
        return count;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get company count: " + std::string(e.what()));
        throw;
    }
}

} // namespace recruitment