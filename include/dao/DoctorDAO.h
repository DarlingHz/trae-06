#pragma once

#include <string>
#include <optional>
#include <vector>
#include "BaseDAO.h"
#include "models/Doctor.h"

namespace pet_hospital {

class DoctorDAO : public BaseDAO {
public:
    DoctorDAO() = default;
    ~DoctorDAO() override = default;

    // 创建医生
    bool create_doctor(const Doctor& doctor);

    // 根据 ID 查询医生
    std::optional<Doctor> get_doctor_by_id(int doctor_id);

    // 查询所有医生
    std::vector<Doctor> get_all_doctors();

    // 根据科室 ID 查询医生
    std::vector<Doctor> get_doctors_by_department_id(int department_id);

    // 更新医生信息
    bool update_doctor(const Doctor& doctor);

    // 删除医生
    bool delete_doctor(int doctor_id);
};

} // namespace pet_hospital
