#include "service/AppointmentService.h"
#include "dao/AppointmentDAO.h"
#include "dao/UserDAO.h"
#include "dao/PetDAO.h"
#include "dao/DoctorDAO.h"
#include "logging/Logging.h"

#include <sstream>
#include <iomanip>
#include <regex>

namespace pet_hospital {

AppointmentService::AppointmentService() {
    appointment_dao_ = std::make_unique<AppointmentDAO>();
    user_dao_ = std::make_unique<UserDAO>();
    pet_dao_ = std::make_unique<PetDAO>();
    doctor_dao_ = std::make_unique<DoctorDAO>();
}

AppointmentService::~AppointmentService() {
}

std::optional<Appointment> AppointmentService::create_appointment(int user_id, int pet_id, int doctor_id, 
                                                      const std::string& start_time, const std::string& end_time, 
                                                      const std::string& reason, std::string& error_message) {
    try {
        // 验证用户ID
        if (user_id <= 0) {
            error_message = "Invalid user ID";
            LOG_ERROR("Create appointment failed: " + error_message);
            return std::nullopt;
        }

        // 验证用户是否存在
        auto user = user_dao_->get_user_by_id(user_id);
        if (!user) {
            error_message = "User not found";
            LOG_ERROR("Create appointment failed: " + error_message);
            return std::nullopt;
        }

        // 验证宠物ID
        if (pet_id <= 0) {
            error_message = "Invalid pet ID";
            LOG_ERROR("Create appointment failed: " + error_message);
            return std::nullopt;
        }

        // 验证宠物是否存在
        auto pet = pet_dao_->get_pet_by_id(pet_id);
        if (!pet) {
            error_message = "Pet not found";
            LOG_ERROR("Create appointment failed: " + error_message);
            return std::nullopt;
        }

        // 验证宠物是否属于该用户
        if (pet->get_user_id() != user_id) {
            error_message = "Pet does not belong to the user";
            LOG_ERROR("Create appointment failed: " + error_message);
            return std::nullopt;
        }

        // 验证医生ID
        if (doctor_id <= 0) {
            error_message = "Invalid doctor ID";
            LOG_ERROR("Create appointment failed: " + error_message);
            return std::nullopt;
        }

        // 验证医生是否存在
        auto doctor = doctor_dao_->get_doctor_by_id(doctor_id);
        if (!doctor) {
            error_message = "Doctor not found";
            LOG_ERROR("Create appointment failed: " + error_message);
            return std::nullopt;
        }

        // 验证预约时间格式
        if (!validate_appointment_time(start_time, error_message)) {
            LOG_ERROR("Create appointment failed: " + error_message);
            return std::nullopt;
        }
        if (!validate_appointment_time(end_time, error_message)) {
            LOG_ERROR("Create appointment failed: " + error_message);
            return std::nullopt;
        }

        // 验证预约时间范围
        if (!validate_appointment_time_range(start_time, end_time, error_message)) {
            LOG_ERROR("Create appointment failed: " + error_message);
            return std::nullopt;
        }

        // 验证预约原因
        if (reason.empty()) {
            error_message = "Appointment reason cannot be empty";
            LOG_ERROR("Create appointment failed: " + error_message);
            return std::nullopt;
        }

        // 检查预约冲突
        if (check_appointment_conflict(doctor_id, start_time, end_time, std::nullopt, error_message)) {
            LOG_ERROR("Create appointment failed: " + error_message);
            return std::nullopt;
        }

        // 检查宠物是否有其他预约冲突
        auto pet_appointments = appointment_dao_->get_appointments_by_pet_id(pet_id);
        for (const auto& appointment : pet_appointments) {
            if (appointment.get_status() != Appointment::Status::CANCELLED && 
                appointment.get_status() != Appointment::Status::COMPLETED) {
                // 检查时间冲突
                if ((start_time >= appointment.get_start_time() && start_time < appointment.get_end_time()) ||
                    (end_time > appointment.get_start_time() && end_time <= appointment.get_end_time()) ||
                    (start_time <= appointment.get_start_time() && end_time >= appointment.get_end_time())) {
                    error_message = "Pet has another appointment at the same time";
                    LOG_ERROR("Create appointment failed: " + error_message);
                    return std::nullopt;
                }
            }
        }

        // 创建预约对象
        Appointment appointment;
        appointment.set_user_id(user_id);
        appointment.set_pet_id(pet_id);
        appointment.set_doctor_id(doctor_id);
        appointment.set_start_time(start_time);
        appointment.set_end_time(end_time);
        appointment.set_reason(reason);
        appointment.set_status(Appointment::Status::PENDING);

        // 保存预约到数据库
        if (!appointment_dao_->create_appointment(appointment)) {
            error_message = "Failed to create appointment";
            LOG_ERROR("Create appointment failed: " + error_message);
            return std::nullopt;
        }

        // 获取创建的预约信息
        auto created_appointment = appointment_dao_->get_appointment_by_id(appointment.get_id());
        if (!created_appointment) {
            error_message = "Failed to retrieve created appointment";
            LOG_ERROR("Create appointment failed: " + error_message);
            return std::nullopt;
        }

        LOG_INFO("Create appointment successfully: " + std::to_string(created_appointment->get_id()) + 
                 " (user: " + std::to_string(user_id) + ", pet: " + std::to_string(pet_id) + ", doctor: " + std::to_string(doctor_id) + ")");
        return created_appointment;
    } catch (const std::exception& e) {
        error_message = "Failed to create appointment: " + std::string(e.what());
        LOG_ERROR("Create appointment failed: " + error_message);
        return std::nullopt;
    }
}

std::optional<Appointment> AppointmentService::get_appointment_by_id(int appointment_id, std::string& error_message) {
    try {
        // 验证预约ID
        if (appointment_id <= 0) {
            error_message = "Invalid appointment ID";
            LOG_ERROR("Get appointment by ID failed: " + error_message);
            return std::nullopt;
        }

        // 获取预约信息
        auto appointment = appointment_dao_->get_appointment_by_id(appointment_id);
        if (!appointment) {
            error_message = "Appointment not found";
            LOG_ERROR("Get appointment by ID failed: " + error_message);
            return std::nullopt;
        }

        LOG_INFO("Get appointment by ID successfully: " + std::to_string(appointment_id));
        return appointment;
    } catch (const std::exception& e) {
        error_message = "Failed to get appointment by ID: " + std::string(e.what());
        LOG_ERROR("Get appointment by ID failed: " + error_message);
        return std::nullopt;
    }
}

std::vector<Appointment> AppointmentService::get_appointments_by_user_id(int user_id, const std::optional<std::string>& from_time, 
                                                               const std::optional<std::string>& to_time, std::string& error_message) {
    try {
        // 验证用户ID
        if (user_id <= 0) {
            error_message = "Invalid user ID";
            LOG_ERROR("Get appointments by user ID failed: " + error_message);
            return {};
        }

        // 验证时间格式（如果提供了时间参数）
        if (from_time) {
            if (!validate_appointment_time(from_time.value(), error_message)) {
                LOG_ERROR("Get appointments by user ID failed: " + error_message);
                return {};
            }
        }
        if (to_time) {
            if (!validate_appointment_time(to_time.value(), error_message)) {
                LOG_ERROR("Get appointments by user ID failed: " + error_message);
                return {};
            }
        }

        // 获取用户预约列表
        auto appointments = appointment_dao_->get_appointments_by_user_id(user_id);

        // 根据时间范围过滤预约（如果提供了时间参数）
        if (from_time || to_time) {
            std::vector<Appointment> filtered_appointments;
            for (const auto& appointment : appointments) {
                bool include = true;
                if (from_time && appointment.get_start_time() < from_time.value()) {
                    include = false;
                }
                if (to_time && appointment.get_end_time() > to_time.value()) {
                    include = false;
                }
                if (include) {
                    filtered_appointments.push_back(appointment);
                }
            }
            appointments = filtered_appointments;
        }

        LOG_INFO("Get appointments by user ID successfully: " + std::to_string(appointments.size()) + 
                 " appointments (user: " + std::to_string(user_id) + ")");
        return appointments;
    } catch (const std::exception& e) {
        error_message = "Failed to get appointments by user ID: " + std::string(e.what());
        LOG_ERROR("Get appointments by user ID failed: " + error_message);
        return {};
    }
}

std::vector<Appointment> AppointmentService::get_appointments_by_doctor_id(int doctor_id, const std::optional<std::string>& date, 
                                                                 std::string& error_message) {
    try {
        // 验证医生ID
        if (doctor_id <= 0) {
            error_message = "Invalid doctor ID";
            LOG_ERROR("Get appointments by doctor ID failed: " + error_message);
            return {};
        }

        // 验证日期格式（如果提供了日期参数）
        if (date) {
            // 日期格式：YYYY-MM-DD
            std::regex date_regex("\\d{4}-\\d{2}-\\d{2}");
            if (!std::regex_match(date.value(), date_regex)) {
                error_message = "Invalid date format. Expected: YYYY-MM-DD";
                LOG_ERROR("Get appointments by doctor ID failed: " + error_message);
                return {};
            }
        }

        // 获取医生预约列表
        auto appointments = appointment_dao_->get_appointments_by_doctor_id(doctor_id);

        // 根据日期过滤预约（如果提供了日期参数）
        if (date) {
            std::vector<Appointment> filtered_appointments;
            std::string date_str = date.value();
            for (const auto& appointment : appointments) {
                // 检查预约开始时间是否在指定日期
                if (appointment.get_start_time().substr(0, 10) == date_str) {
                    filtered_appointments.push_back(appointment);
                }
            }
            appointments = filtered_appointments;
        }

        LOG_INFO("Get appointments by doctor ID successfully: " + std::to_string(appointments.size()) + 
                 " appointments (doctor: " + std::to_string(doctor_id) + ")");
        return appointments;
    } catch (const std::exception& e) {
        error_message = "Failed to get appointments by doctor ID: " + std::string(e.what());
        LOG_ERROR("Get appointments by doctor ID failed: " + error_message);
        return {};
    }
}

bool AppointmentService::cancel_appointment(int appointment_id, int user_id, std::string& error_message) {
    try {
        // 验证预约ID和用户ID
        if (appointment_id <= 0 || user_id <= 0) {
            error_message = "Invalid appointment ID or user ID";
            LOG_ERROR("Cancel appointment failed: " + error_message);
            return false;
        }

        // 获取预约信息
        auto appointment = appointment_dao_->get_appointment_by_id(appointment_id);
        if (!appointment) {
            error_message = "Appointment not found";
            LOG_ERROR("Cancel appointment failed: " + error_message);
            return false;
        }

        // 验证预约是否属于该用户
        if (appointment->get_user_id() != user_id) {
            error_message = "Appointment does not belong to the user";
            LOG_ERROR("Cancel appointment failed: " + error_message);
            return false;
        }

        // 验证预约状态是否可以取消
        if (appointment->get_status() != Appointment::Status::PENDING) {
            error_message = "Appointment cannot be canceled";
            LOG_ERROR("Cancel appointment failed: " + error_message);
            return false;
        }

        // 更新预约状态为已取消
        if (!update_appointment_status(appointment_id, Appointment::Status::CANCELLED, error_message)) {
            LOG_ERROR("Cancel appointment failed: " + error_message);
            return false;
        }

        LOG_INFO("Cancel appointment successfully: " + std::to_string(appointment_id) + 
                 " (user: " + std::to_string(user_id) + ")");
        return true;
    } catch (const std::exception& e) {
        error_message = "Failed to cancel appointment: " + std::string(e.what());
        LOG_ERROR("Cancel appointment failed: " + error_message);
        return false;
    }
}

bool AppointmentService::update_appointment_status(int appointment_id, Appointment::Status status, std::string& error_message) {
    try {
        // 验证预约ID
        if (appointment_id <= 0) {
            error_message = "Invalid appointment ID";
            LOG_ERROR("Update appointment status failed: " + error_message);
            return false;
        }

        // 获取预约信息
        auto appointment = appointment_dao_->get_appointment_by_id(appointment_id);
        if (!appointment) {
            error_message = "Appointment not found";
            LOG_ERROR("Update appointment status failed: " + error_message);
            return false;
        }

        // 验证预约状态流转
        if (!validate_appointment_status_transition(appointment->get_status(), status, error_message)) {
            LOG_ERROR("Update appointment status failed: " + error_message);
            return false;
        }

        // 更新预约状态
        if (!appointment_dao_->update_appointment_status(appointment_id, status)) {
            error_message = "Failed to update appointment status";
            LOG_ERROR("Update appointment status failed: " + error_message);
            return false;
        }

        LOG_INFO("Update appointment status successfully: " + std::to_string(appointment_id) + 
                 " (from: " + Appointment::status_to_string(appointment->get_status()) + 
                 ", to: " + Appointment::status_to_string(status) + ")");
        return true;
    } catch (const std::exception& e) {
        error_message = "Failed to update appointment status: " + std::string(e.what());
        LOG_ERROR("Update appointment status failed: " + error_message);
        return false;
    }
}

bool AppointmentService::check_appointment_conflict(int doctor_id, const std::string& start_time, const std::string& end_time, 
                                      const std::optional<int>& exclude_appointment_id, std::string& error_message) {
    try {
        // 验证医生ID
        if (doctor_id <= 0) {
            error_message = "Invalid doctor ID";
            LOG_ERROR("Check appointment conflict failed: " + error_message);
            return true; // 返回true表示有冲突
        }

        // 验证预约时间格式
        if (!validate_appointment_time(start_time, error_message)) {
            LOG_ERROR("Check appointment conflict failed: " + error_message);
            return true; // 返回true表示有冲突
        }
        if (!validate_appointment_time(end_time, error_message)) {
            LOG_ERROR("Check appointment conflict failed: " + error_message);
            return true; // 返回true表示有冲突
        }

        // 验证预约时间范围
        if (!validate_appointment_time_range(start_time, end_time, error_message)) {
            LOG_ERROR("Check appointment conflict failed: " + error_message);
            return true; // 返回true表示有冲突
        }

        // 获取医生预约列表
        auto appointments = appointment_dao_->get_appointments_by_doctor_id(doctor_id);

        // 检查是否有时间冲突的预约
        for (const auto& appointment : appointments) {
            // 排除指定的预约（用于更新预约时的冲突检查）
            if (exclude_appointment_id && appointment.get_id() == exclude_appointment_id.value()) {
                continue;
            }

            // 检查预约状态是否为有效状态（未取消、未完成）
            if (appointment.get_status() != Appointment::Status::CANCELLED && 
                appointment.get_status() != Appointment::Status::COMPLETED) {
                // 检查时间冲突
                if ((start_time >= appointment.get_start_time() && start_time < appointment.get_end_time()) ||
                    (end_time > appointment.get_start_time() && end_time <= appointment.get_end_time()) ||
                    (start_time <= appointment.get_start_time() && end_time >= appointment.get_end_time())) {
                    error_message = "Doctor has another appointment at the same time";
                    LOG_ERROR("Check appointment conflict failed: " + error_message);
                    return true; // 返回true表示有冲突
                }
            }
        }

        LOG_INFO("Check appointment conflict successfully: No conflict found (doctor: " + std::to_string(doctor_id) + ")");
        return false; // 返回false表示没有冲突
    } catch (const std::exception& e) {
        error_message = "Failed to check appointment conflict: " + std::string(e.what());
        LOG_ERROR("Check appointment conflict failed: " + error_message);
        return true; // 返回true表示有冲突
    }
}

bool AppointmentService::validate_appointment_time(const std::string& time_str, std::string& error_message) {
    // 时间格式：YYYY-MM-DD HH:MM:SS
    std::regex time_regex("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}");
    if (!std::regex_match(time_str, time_regex)) {
        error_message = "Invalid time format. Expected: YYYY-MM-DD HH:MM:SS";
        return false;
    }

    // 验证时间是否为有效时间
    auto time_point = parse_time_string(time_str);
    if (!time_point) {
        error_message = "Invalid time value";
        return false;
    }

    return true;
}

bool AppointmentService::validate_appointment_time_range(const std::string& start_time, const std::string& end_time, std::string& error_message) {
    // 验证开始时间是否早于结束时间
    if (start_time >= end_time) {
        error_message = "Start time must be earlier than end time";
        return false;
    }

    // 验证预约时间是否在未来（至少提前30分钟）
    auto now = std::chrono::system_clock::now();
    auto start_time_point = parse_time_string(start_time);
    if (!start_time_point) {
        error_message = "Invalid start time value";
        return false;
    }

    auto time_diff = *start_time_point - now;
    if (time_diff < std::chrono::minutes(30)) {
        error_message = "Appointment must be scheduled at least 30 minutes in advance";
        return false;
    }

    return true;
}

bool AppointmentService::validate_appointment_status_transition(Appointment::Status current_status, Appointment::Status new_status, std::string& error_message) {
    // 定义允许的状态流转
    // 待就诊 -> 已取消 / 已完成
    // 已取消 -> 无（不能再改变状态）
    // 已完成 -> 无（不能再改变状态）

    if (current_status == Appointment::Status::PENDING) {
        if (new_status == Appointment::Status::CANCELLED || new_status == Appointment::Status::COMPLETED) {
            return true;
        }
    }

    error_message = "Invalid appointment status transition";
    return false;
}

std::optional<std::chrono::system_clock::time_point> AppointmentService::parse_time_string(const std::string& time_str) {
    try {
        std::tm tm = {};
        std::istringstream ss(time_str);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

        if (ss.fail()) {
            return std::nullopt;
        }

        auto time_point = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        return time_point;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse time string: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::string AppointmentService::format_time_point(const std::chrono::system_clock::time_point& time_point) {
    try {
        auto time_t = std::chrono::system_clock::to_time_t(time_point);
        std::tm tm = {};
        localtime_r(&time_t, &tm);

        std::ostringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

        return ss.str();
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to format time point: " + std::string(e.what()));
        return "";
    }
}

} // namespace pet_hospital
