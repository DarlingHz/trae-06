#include "service/DoctorService.h"
#include "dao/DoctorDAO.h"
#include "dao/DepartmentDAO.h"
#include "logging/Logging.h"

namespace pet_hospital {

DoctorService::DoctorService() {
    doctor_dao_ = std::make_unique<DoctorDAO>();
    department_dao_ = std::make_unique<DepartmentDAO>();
    // 初始化医生列表缓存
    refresh_doctor_cache();
}

DoctorService::~DoctorService() {
}

std::optional<Doctor> DoctorService::add_doctor(int department_id, const std::string& name, 
                                        const std::optional<std::string>& title, 
                                        const std::optional<std::string>& specialty, 
                                        const std::optional<std::string>& available_start, 
                                        const std::optional<std::string>& available_end, 
                                        std::string& error_message) {
    try {
        // 验证科室ID
        if (department_id <= 0) {
            error_message = "Invalid department ID";
            LOG_ERROR("Add doctor failed: " + error_message);
            return std::nullopt;
        }

        // 验证科室是否存在
        auto department = department_dao_->get_department_by_id(department_id);
        if (!department) {
            error_message = "Department not found";
            LOG_ERROR("Add doctor failed: " + error_message);
            return std::nullopt;
        }

        // 验证医生姓名
        if (name.empty()) {
            error_message = "Doctor name cannot be empty";
            LOG_ERROR("Add doctor failed: " + error_message);
            return std::nullopt;
        }

        // 创建医生对象
        Doctor doctor;
        doctor.set_department_id(department_id);
        doctor.set_name(name);
        if (title) {
            doctor.set_title(title.value());
        }
        if (specialty) {
            doctor.set_specialty(specialty.value());
        }
        if (available_start) {
            doctor.set_available_start(available_start.value());
        }
        if (available_end) {
            doctor.set_available_end(available_end.value());
        }

        // 保存医生到数据库
        if (!doctor_dao_->create_doctor(doctor)) {
            error_message = "Failed to create doctor";
            LOG_ERROR("Add doctor failed: " + error_message);
            return std::nullopt;
        }

        // 获取创建的医生信息
        auto created_doctor = doctor_dao_->get_doctor_by_id(doctor.get_id());
        if (!created_doctor) {
            error_message = "Failed to retrieve created doctor";
            LOG_ERROR("Add doctor failed: " + error_message);
            return std::nullopt;
        }

        // 刷新医生列表缓存
        refresh_doctor_cache();

        LOG_INFO("Add doctor successfully: " + name + " (department: " + std::to_string(department_id) + ")");
        return created_doctor;
    } catch (const std::exception& e) {
        error_message = "Failed to add doctor: " + std::string(e.what());
        LOG_ERROR("Add doctor failed: " + error_message);
        return std::nullopt;
    }
}

std::vector<Doctor> DoctorService::get_all_doctors(std::string& error_message) {
    try {
        // 检查医生列表缓存是否过期
        if (is_doctor_cache_expired()) {
            // 刷新医生列表缓存
            refresh_doctor_cache();
        }

        LOG_INFO("Get all doctors successfully: " + std::to_string(doctor_cache_.size()) + " doctors");
        return doctor_cache_;
    } catch (const std::exception& e) {
        error_message = "Failed to get all doctors: " + std::string(e.what());
        LOG_ERROR("Get all doctors failed: " + error_message);
        return {};
    }
}

std::optional<Doctor> DoctorService::get_doctor_by_id(int doctor_id, std::string& error_message) {
    try {
        // 验证医生ID
        if (doctor_id <= 0) {
            error_message = "Invalid doctor ID";
            LOG_ERROR("Get doctor by ID failed: " + error_message);
            return std::nullopt;
        }

        // 获取医生信息
        auto doctor = doctor_dao_->get_doctor_by_id(doctor_id);
        if (!doctor) {
            error_message = "Doctor not found";
            LOG_ERROR("Get doctor by ID failed: " + error_message);
            return std::nullopt;
        }

        LOG_INFO("Get doctor by ID successfully: " + std::to_string(doctor_id));
        return doctor;
    } catch (const std::exception& e) {
        error_message = "Failed to get doctor by ID: " + std::string(e.what());
        LOG_ERROR("Get doctor by ID failed: " + error_message);
        return std::nullopt;
    }
}

std::vector<Doctor> DoctorService::get_doctors_by_department_id(int department_id, std::string& error_message) {
    try {
        // 验证科室ID
        if (department_id <= 0) {
            error_message = "Invalid department ID";
            LOG_ERROR("Get doctors by department ID failed: " + error_message);
            return {};
        }

        // 验证科室是否存在
        auto department = department_dao_->get_department_by_id(department_id);
        if (!department) {
            error_message = "Department not found";
            LOG_ERROR("Get doctors by department ID failed: " + error_message);
            return {};
        }

        // 获取科室医生列表
        auto doctors = doctor_dao_->get_doctors_by_department_id(department_id);

        LOG_INFO("Get doctors by department ID successfully: " + std::to_string(doctors.size()) + " doctors (department: " + std::to_string(department_id) + ")");
        return doctors;
    } catch (const std::exception& e) {
        error_message = "Failed to get doctors by department ID: " + std::string(e.what());
        LOG_ERROR("Get doctors by department ID failed: " + error_message);
        return {};
    }
}

bool DoctorService::update_doctor(int doctor_id, int department_id, const std::string& name, 
                        const std::optional<std::string>& title, 
                        const std::optional<std::string>& specialty, 
                        const std::optional<std::string>& available_start, 
                        const std::optional<std::string>& available_end, 
                        std::string& error_message) {
    try {
        // 验证医生ID和科室ID
        if (doctor_id <= 0 || department_id <= 0) {
            error_message = "Invalid doctor ID or department ID";
            LOG_ERROR("Update doctor failed: " + error_message);
            return false;
        }

        // 验证科室是否存在
        auto department = department_dao_->get_department_by_id(department_id);
        if (!department) {
            error_message = "Department not found";
            LOG_ERROR("Update doctor failed: " + error_message);
            return false;
        }

        // 验证医生姓名
        if (name.empty()) {
            error_message = "Doctor name cannot be empty";
            LOG_ERROR("Update doctor failed: " + error_message);
            return false;
        }

        // 获取医生信息
        auto doctor = doctor_dao_->get_doctor_by_id(doctor_id);
        if (!doctor) {
            error_message = "Doctor not found";
            LOG_ERROR("Update doctor failed: " + error_message);
            return false;
        }

        // 更新医生信息
        doctor->set_department_id(department_id);
        doctor->set_name(name);
        if (title) {
            doctor->set_title(title.value());
        } else {
            doctor->set_title(std::nullopt);
        }
        if (specialty) {
            doctor->set_specialty(specialty.value());
        } else {
            doctor->set_specialty(std::nullopt);
        }
        if (available_start) {
            doctor->set_available_start(available_start.value());
        }
        if (available_end) {
            doctor->set_available_end(available_end.value());
        }

        // 保存更新后的医生信息
        if (!doctor_dao_->update_doctor(*doctor)) {
            error_message = "Failed to update doctor";
            LOG_ERROR("Update doctor failed: " + error_message);
            return false;
        }

        // 刷新医生列表缓存
        refresh_doctor_cache();

        LOG_INFO("Update doctor successfully: " + std::to_string(doctor_id) + " (department: " + std::to_string(department_id) + ")");
        return true;
    } catch (const std::exception& e) {
        error_message = "Failed to update doctor: " + std::string(e.what());
        LOG_ERROR("Update doctor failed: " + error_message);
        return false;
    }
}

bool DoctorService::delete_doctor(int doctor_id, std::string& error_message) {
    try {
        // 验证医生ID
        if (doctor_id <= 0) {
            error_message = "Invalid doctor ID";
            LOG_ERROR("Delete doctor failed: " + error_message);
            return false;
        }

        // 获取医生信息
        auto doctor = doctor_dao_->get_doctor_by_id(doctor_id);
        if (!doctor) {
            error_message = "Doctor not found";
            LOG_ERROR("Delete doctor failed: " + error_message);
            return false;
        }

        // 删除医生
        if (!doctor_dao_->delete_doctor(doctor_id)) {
            error_message = "Failed to delete doctor";
            LOG_ERROR("Delete doctor failed: " + error_message);
            return false;
        }

        // 刷新医生列表缓存
        refresh_doctor_cache();

        LOG_INFO("Delete doctor successfully: " + std::to_string(doctor_id));
        return true;
    } catch (const std::exception& e) {
        error_message = "Failed to delete doctor: " + std::string(e.what());
        LOG_ERROR("Delete doctor failed: " + error_message);
        return false;
    }
}

std::vector<Department> DoctorService::get_all_departments(std::string& error_message) {
    try {
        // 获取所有科室
        auto departments = department_dao_->get_all_departments();

        LOG_INFO("Get all departments successfully: " + std::to_string(departments.size()) + " departments");
        return departments;
    } catch (const std::exception& e) {
        error_message = "Failed to get all departments: " + std::string(e.what());
        LOG_ERROR("Get all departments failed: " + error_message);
        return {};
    }
}

std::optional<Department> DoctorService::get_department_by_id(int department_id, std::string& error_message) {
    try {
        // 验证科室ID
        if (department_id <= 0) {
            error_message = "Invalid department ID";
            LOG_ERROR("Get department by ID failed: " + error_message);
            return std::nullopt;
        }

        // 获取科室信息
        auto department = department_dao_->get_department_by_id(department_id);
        if (!department) {
            error_message = "Department not found";
            LOG_ERROR("Get department by ID failed: " + error_message);
            return std::nullopt;
        }

        LOG_INFO("Get department by ID successfully: " + std::to_string(department_id));
        return department;
    } catch (const std::exception& e) {
        error_message = "Failed to get department by ID: " + std::string(e.what());
        LOG_ERROR("Get department by ID failed: " + error_message);
        return std::nullopt;
    }
}

void DoctorService::refresh_doctor_cache() {
    try {
        // 获取所有医生
        auto doctors = doctor_dao_->get_all_doctors();
        // 更新医生列表缓存
        doctor_cache_ = doctors;
        // 更新医生列表缓存过期时间
        doctor_cache_expiry_ = std::chrono::system_clock::now() + std::chrono::seconds(DOCTOR_CACHE_DURATION);

        LOG_INFO("Refresh doctor cache successfully: " + std::to_string(doctor_cache_.size()) + " doctors");
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to refresh doctor cache: " + std::string(e.what()));
    }
}

bool DoctorService::is_doctor_cache_expired() {
    auto now = std::chrono::system_clock::now();
    return now > doctor_cache_expiry_;
}

} // namespace pet_hospital
