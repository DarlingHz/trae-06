#ifndef BASE_REPOSITORY_H
#define BASE_REPOSITORY_H

#include <memory>
#include <vector>
#include <optional>

namespace repository {

// 基础Repository接口，定义通用的数据访问方法
template <typename T>
class BaseRepository {
public:
    virtual ~BaseRepository() = default;

    // 创建实体
    virtual int create(const T& entity) = 0;

    // 根据ID查找实体
    virtual ::std::optional<T> findById(int id) = 0;

    // 查找所有实体
    virtual ::std::vector<T> findAll() = 0;

    // 更新实体
    virtual bool update(const T& entity) = 0;

    // 根据ID删除实体
    virtual bool deleteById(int id) = 0;
};

} // namespace repository

#endif // BASE_REPOSITORY_H