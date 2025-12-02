#ifndef COMPANY_H
#define COMPANY_H

#include <string>
#include <optional>
#include <vector>
#include "base_dao.h"

namespace recruitment {

/**
 * @brief 公司实体类
 */
class Company {
public:
    /**
     * @brief 默认构造函数
     */
    Company() = default;

    /**
     * @brief 构造函数
     * @param name 公司名称
     * @param industry 行业
     * @param location 地点
     * @param description 描述
     */
    Company(const std::string& name,
            const std::string& industry,
            const std::string& location,
            const std::string& description)
        : name_(name), industry_(industry), location_(location), description_(description) {}

    /**
     * @brief 获取公司ID
     * @return 公司ID
     */
    long long getId() const { return id_; }

    /**
     * @brief 设置公司ID
     * @param id 公司ID
     */
    void setId(long long id) { id_ = id; }

    /**
     * @brief 获取公司名称
     * @return 公司名称
     */
    const std::string& getName() const { return name_; }

    /**
     * @brief 设置公司名称
     * @param name 公司名称
     */
    void setName(const std::string& name) { name_ = name; }

    /**
     * @brief 获取行业
     * @return 行业
     */
    const std::string& getIndustry() const { return industry_; }

    /**
     * @brief 设置行业
     * @param industry 行业
     */
    void setIndustry(const std::string& industry) { industry_ = industry; }

    /**
     * @brief 获取地点
     * @return 地点
     */
    const std::string& getLocation() const { return location_; }

    /**
     * @brief 设置地点
     * @param location 地点
     */
    void setLocation(const std::string& location) { location_ = location; }

    /**
     * @brief 获取描述
     * @return 描述
     */
    const std::string& getDescription() const { return description_; }

    /**
     * @brief 设置描述
     * @param description 描述
     */
    void setDescription(const std::string& description) { description_ = description; }

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

    /**
     * @brief 转换为JSON字符串
     * @return JSON字符串
     */
    std::string toJson() const;

    /**
     * @brief 从JSON字符串解析
     * @param json JSON字符串
     * @return 解析成功返回true，否则返回false
     */
    bool fromJson(const std::string& json);

private:
    long long id_ = 0; ///< 公司ID
    std::string name_; ///< 公司名称
    std::string industry_; ///< 行业
    std::string location_; ///< 地点
    std::string description_; ///< 描述
    std::string created_at_; ///< 创建时间
    std::string updated_at_; ///< 更新时间
};

/**
 * @brief 公司DAO类
 */
class CompanyDAO : public BaseDAO<Company> {
public:
    /**
     * @brief 默认构造函数
     */
    CompanyDAO() = default;

    /**
     * @brief 析构函数
     */
    virtual ~CompanyDAO() = default;
    /**
     * @brief 创建公司
     * @param company 公司实体
     * @return 创建成功返回公司ID，否则返回-1
     */
    long long create(const Company& company) override;

    /**
     * @brief 根据ID获取公司
     * @param id 公司ID
     * @return 公司实体，如果不存在则返回空
     */
    std::optional<Company> getById(long long id) override;

    /**
     * @brief 更新公司
     * @param company 公司实体
     * @return 更新成功返回true，否则返回false
     */
    bool update(const Company& company) override;

    /**
     * @brief 根据ID删除公司
     * @param id 公司ID
     * @return 删除成功返回true，否则返回false
     */
    bool deleteById(long long id) override;

    /**
     * @brief 获取所有公司
     * @return 公司实体列表
     */
    std::vector<Company> getAll() override;

    /**
     * @brief 根据条件查询公司
     * @param industry 行业，可选
     * @param location 地点，可选
     * @param page 页码，默认1
     * @param page_size 每页数量，默认10
     * @return 公司实体列表
     */
    std::vector<Company> findByCondition(const std::optional<std::string>& industry = std::nullopt,
                                             const std::optional<std::string>& location = std::nullopt,
                                             int page = 1,
                                             int page_size = 10);

    /**
     * @brief 根据条件获取公司数量
     * @param industry 行业，可选
     * @param location 地点，可选
     * @return 公司数量
     */
    int getCompanyCount(const std::optional<std::string>& industry = std::nullopt,
                        const std::optional<std::string>& location = std::nullopt);
};

} // namespace recruitment

#endif // COMPANY_H