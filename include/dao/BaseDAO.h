#pragma once

#include <string>
#include <vector>
#include <memory>
#include "database/Database.h"

namespace pet_hospital {

class BaseDAO {
public:
    BaseDAO() = default;
    virtual ~BaseDAO() = default;

    // 设置数据库连接
    void set_database(std::shared_ptr<Database> database) {
        database_ = database;
    }

    // 获取数据库连接
    std::shared_ptr<Database> get_database() const {
        return database_;
    }

protected:
    std::shared_ptr<Database> database_;
};

} // namespace pet_hospital
