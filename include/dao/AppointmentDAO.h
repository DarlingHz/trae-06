#pragma once

#include <string>
#include <optional>
#include <vector>
#include "BaseDAO.h"
#include "models/Appointment.h"

namespace pet_hospital {

class AppointmentDAO : public BaseDAO {
public:
    AppointmentDAO() = default;
    ~AppointmentDAO() override = default;

    // 创建预约
    bool create_appointment(const Appointment& appointment);

    // 根据 ID 查询预约
    std::optional<Appointment> get_appointment_by_id(int appointment_id);

    // 查询用户预约
    std::vector<Appointment> get_appointments_by_user_id(int user_id, 
                                                               const std::string& from = "", 
                                                               const std::string& to = "",
                                                               int page = 1, int page_size = 10);

    // 查询医生预约
    std::vector<Appointment> get_appointments_by_doctor_id(int doctor_id, 
                                                                 const std::string& date = "");

    // 查询宠物预约
    std::vector<Appointment> get_appointments_by_pet_id(int pet_id);

    // 取消预约
    bool cancel_appointment(int appointment_id);

    // 更新预约状态
    bool update_appointment_status(int appointment_id, Appointment::Status status);

    // 检查预约是否存在冲突
    bool check_appointment_conflict(int doctor_id, int pet_id, 
                                      const std::string& start_time, 
                                      const std::string& end_time, 
                                      int exclude_appointment_id = -1);
};

} // namespace pet_hospital
