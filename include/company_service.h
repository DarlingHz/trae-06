#ifndef COMPANY_SERVICE_H
#define COMPANY_SERVICE_H

#include <string>
#include <optional>
#include <vector>
#include "company.h"

namespace recruitment {

/**
 * @brief 公司服务类
 */
class CompanyService {
public:
    /**
     * @brief 构造函数
     * @param company_dao 公司DAO对象
     */
    CompanyService(std::shared_ptr<CompanyDAO> company_dao)
        : company_dao_(std::move(company_dao)) {}

    /**
     * @brief 析构函数
     */
    virtual ~CompanyService() = default;

    /**
     * @brief 根据条件查询公司数量
     * @param industry 行业，可选
     * @param location 地点，可选
     * @return 公司数量
     */
    virtual int getCompanyCount(const std::optional<std::string>& industry = std::nullopt,
                                 const std::optional<std::string>& location = std::nullopt) = 0;





protected:
    std::shared_ptr<CompanyDAO> company_dao_; ///< 公司DAO对象

    /**
     * @brief 创建公司
     * @param company 公司实体
     * @return 创建成功返回公司ID，否则返回-1
     */
    virtual long long createCompany(const Company& company) = 0;

    /**
     * @brief 根据ID获取公司
     * @param id 公司ID
     * @return 公司实体，如果不存在则返回空
     */
    virtual std::optional<Company> getCompanyById(long long id) = 0;

    /**
     * @brief 更新公司信息
     * @param company 公司实体
     * @return 更新成功返回true，否则返回false
     */
    virtual bool updateCompany(const Company& company) = 0;

    /**
     * @brief 根据ID删除公司
     * @param id 公司ID
     * @return 删除成功返回true，否则返回false
     */
    virtual bool deleteCompanyById(long long id) = 0;

    /**
     * @brief 获取所有公司
     * @return 公司实体列表
     */
    virtual std::vector<Company> getAllCompanies() = 0;

    /**
     * @brief 根据条件查询公司
     * @param industry 行业，可选
     * @param location 地点，可选
     * @param page 页码，默认1
     * @param page_size 每页数量，默认10
     * @return 公司实体列表
     */
    virtual std::vector<Company> findCompaniesByCondition(const std::optional<std::string>& industry = std::nullopt,
                                                               const std::optional<std::string>& location = std::nullopt,
                                                               int page = 1,
                                                               int page_size = 10) = 0;


};

/**
 * @brief 公司服务实现类
 */
class CompanyServiceImpl : public CompanyService {
public:
    /**
     * @brief 构造函数
     * @param company_dao 公司DAO对象
     */
    CompanyServiceImpl(std::shared_ptr<CompanyDAO> company_dao);

    /**
     * @brief 析构函数
     */
    virtual ~CompanyServiceImpl() = default;

    /**
     * @brief 创建公司
     * @param company 公司实体
     * @return 创建成功返回公司ID，否则返回-1
     */
    long long createCompany(const Company& company) override;

    /**
     * @brief 根据ID获取公司
     * @param id 公司ID
     * @return 公司实体，如果不存在则返回空
     */
    std::optional<Company> getCompanyById(long long id) override;

    /**
     * @brief 更新公司信息
     * @param company 公司实体
     * @return 更新成功返回true，否则返回false
     */
    bool updateCompany(const Company& company) override;

    /**
     * @brief 根据ID删除公司
     * @param id 公司ID
     * @return 删除成功返回true，否则返回false
     */
    bool deleteCompanyById(long long id) override;

    /**
     * @brief 获取所有公司
     * @return 公司实体列表
     */
    std::vector<Company> getAllCompanies() override;

    /**
     * @brief 根据条件查询公司
     * @param industry 行业，可选
     * @param location 地点，可选
     * @param page 页码，默认1
     * @param page_size 每页数量，默认10
     * @return 公司实体列表
     */
    std::vector<Company> findCompaniesByCondition(const std::optional<std::string>& industry = std::nullopt,
                                                     const std::optional<std::string>& location = std::nullopt,
                                                     int page = 1,
                                                     int page_size = 10) override;

    /**
     * @brief 根据条件查询公司数量
     * @param industry 行业，可选
     * @param location 地点，可选
     * @return 公司数量
     */
    int getCompanyCount(const std::optional<std::string>& industry = std::nullopt,
                         const std::optional<std::string>& location = std::nullopt) override;

};

} // namespace recruitment

#endif // COMPANY_SERVICE_H