#pragma once

#include <string>
#include <optional>
#include <vector>
#include <unordered_map>
#include <chrono>
#include "models/Doctor.h"
#include "models/Department.h"

namespace pet_hospital {

class DoctorDAO;
class DepartmentDAO;

class DoctorService {
public:
    DoctorService();
    ~DoctorService();

    // 新增医生
    std::optional<Doctor> add_doctor(int department_id, const std::string& name, 
                                        const std::optional<std::string>& title, 
                                        const std::optional<std::string>& specialty, 
                                        const std::optional<std::string>& available_start, 
                                        const std::optional<std::string>& available_end, 
                                        std::string& error_message);

    // 查询所有医生
    std::vector<Doctor> get_all_doctors(std::string& error_message);

    // 根据ID查询医生
    std::optional<Doctor> get_doctor_by_id(int doctor_id, std::string& error_message);

    // 根据科室ID查询医生
    std::vector<Doctor> get_doctors_by_department_id(int department_id, std::string& error_message);

    // 修改医生信息
    bool update_doctor(int doctor_id, int department_id, const std::string& name, 
                        const std::optional<std::string>& title, 
                        const std::optional<std::string>& specialty, 
                        const std::optional<std::string>& available_start, 
                        const std::optional<std::string>& available_end, 
                        std::string& error_message);

    // 删除医生
    bool delete_doctor(int doctor_id, std::string& error_message);

    // 查询所有科室
    std::vector<Department> get_all_departments(std::string& error_message);

    // 根据ID查询科室
    std::optional<Department> get_department_by_id(int department_id, std::string& error_message);

private:
    // 刷新医生列表缓存
    void refresh_doctor_cache();

    // 检查医生列表缓存是否过期
    bool is_doctor_cache_expired();

    std::unique_ptr<DoctorDAO> doctor_dao_;
    std::unique_ptr<DepartmentDAO> department_dao_;

    // 医生列表缓存
    std::vector<Doctor> doctor_cache_;
    std::chrono::system_clock::time_point doctor_cache_expiry_;
    const int DOCTOR_CACHE_DURATION = 300; // 5分钟
};

} // namespace pet_hospital
