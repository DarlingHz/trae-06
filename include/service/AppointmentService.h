#pragma once

#include <optional>
#include <vector>
#include <string>
#include <chrono>

#include "models/Appointment.h"

namespace pet_hospital {

// 数据访问对象（DAO）的前向声明
class AppointmentDAO;
class UserDAO;
class PetDAO;
class DoctorDAO;

class AppointmentService {
public:
    AppointmentService();
    ~AppointmentService();

    // 创建预约
    std::optional<Appointment> create_appointment(int user_id, int pet_id, int doctor_id, 
                                                      const std::string& start_time, const std::string& end_time, 
                                                      const std::string& reason, std::string& error_message);

    // 根据ID获取预约
    std::optional<Appointment> get_appointment_by_id(int appointment_id, std::string& error_message);

    // 根据用户ID获取预约
    std::vector<Appointment> get_appointments_by_user_id(int user_id, const std::optional<std::string>& from_time, 
                                                               const std::optional<std::string>& to_time, std::string& error_message);

    // 根据医生ID获取预约
    std::vector<Appointment> get_appointments_by_doctor_id(int doctor_id, const std::optional<std::string>& date, 
                                                                 std::string& error_message);

    // 取消预约
    bool cancel_appointment(int appointment_id, int user_id, std::string& error_message);

    // 更新预约状态
    bool update_appointment_status(int appointment_id, Appointment::Status status, std::string& error_message);

    // 检查预约冲突
    bool check_appointment_conflict(int doctor_id, const std::string& start_time, const std::string& end_time, 
                                      const std::optional<int>& exclude_appointment_id, std::string& error_message);

private:
    // 验证预约时间格式
    bool validate_appointment_time(const std::string& time_str, std::string& error_message);

    // 验证预约时间范围
    bool validate_appointment_time_range(const std::string& start_time, const std::string& end_time, std::string& error_message);

    // 验证预约状态流转
    bool validate_appointment_status_transition(Appointment::Status current_status, Appointment::Status new_status, std::string& error_message);

    // 转换字符串时间为时间戳
    std::optional<std::chrono::system_clock::time_point> parse_time_string(const std::string& time_str);

    // 转换时间戳为字符串时间
    std::string format_time_point(const std::chrono::system_clock::time_point& time_point);

private:
    // 数据访问对象（DAO）的智能指针
    std::unique_ptr<AppointmentDAO> appointment_dao_;
    std::unique_ptr<UserDAO> user_dao_;
    std::unique_ptr<PetDAO> pet_dao_;
    std::unique_ptr<DoctorDAO> doctor_dao_;
};

} // namespace pet_hospital
