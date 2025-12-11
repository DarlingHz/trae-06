#ifndef BASE_DAO_H
#define BASE_DAO_H

#include <memory>
#include <vector>
#include <optional>
#include "database.h"

namespace recruitment {

/**
 * @brief DAO基类
 * @tparam T 实体类型
 */
template<typename T>
class BaseDAO {
public:
    /**
     * @brief 构造函数
     */
    BaseDAO() = default;

    /**
     * @brief 析构函数
     */
    virtual ~BaseDAO() = default;

    /**
     * @brief 禁止拷贝构造
     */
    BaseDAO(const BaseDAO&) = delete;

    /**
     * @brief 禁止赋值操作
     */
    BaseDAO& operator=(const BaseDAO&) = delete;

    /**
     * @brief 创建实体
     * @param entity 实体对象
     * @return 创建成功返回实体ID，否则返回-1
     */
    virtual long long create(const T& entity) = 0;

    /**
     * @brief 根据ID获取实体
     * @param id 实体ID
     * @return 实体对象，如果不存在则返回空
     */
    virtual std::optional<T> getById(long long id) = 0;

    /**
     * @brief 更新实体
     * @param entity 实体对象
     * @return 更新成功返回true，否则返回false
     */
    virtual bool update(const T& entity) = 0;

    /**
     * @brief 根据ID删除实体
     * @param id 实体ID
     * @return 删除成功返回true，否则返回false
     */
    virtual bool deleteById(long long id) = 0;

    /**
     * @brief 获取所有实体
     * @return 实体对象列表
     */
    virtual std::vector<T> getAll() = 0;

protected:
    /**
     * @brief 获取数据库连接
     * @return 数据库连接智能指针，如果获取失败返回空
     */
    std::shared_ptr<DatabaseConnection> getConnection() {
        return Database::getConnection();
    }

    /**
     * @brief 放回数据库连接
     * @param connection 数据库连接智能指针
     */
    void returnConnection(std::shared_ptr<DatabaseConnection> connection) {
        Database::returnConnection(connection);
    }
};

} // namespace recruitment

#endif // BASE_DAO_H