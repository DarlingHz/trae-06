#pragma once

#include <string>
#include <optional>
#include <vector>
#include "BaseDAO.h"
#include "models/Department.h"

namespace pet_hospital {

class DepartmentDAO : public BaseDAO {
public:
    DepartmentDAO() = default;
    ~DepartmentDAO() override = default;

    // 创建科室
    bool create_department(const Department& department);

    // 根据 ID 查询科室
    std::optional<Department> get_department_by_id(int department_id);

    // 查询所有科室
    std::vector<Department> get_all_departments();

    // 更新科室信息
    bool update_department(const Department& department);

    // 删除科室
    bool delete_department(int department_id);
};

} // namespace pet_hospital