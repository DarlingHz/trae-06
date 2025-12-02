#include "dao/AppointmentDAO.h"
#include <sstream>

namespace pet_hospital {

bool AppointmentDAO::create_appointment(const Appointment& appointment) {
    try {
        std::stringstream sql;
        sql << "INSERT INTO appointments (user_id, pet_id, doctor_id, start_time, end_time, reason, status) VALUES (" 
            << appointment.get_user_id() << ", "
            << appointment.get_pet_id() << ", "
            << appointment.get_doctor_id() << ", "
            << "'" << appointment.get_start_time() << "', "
            << "'" << appointment.get_end_time() << "', ";
        
        if (appointment.get_reason()) {
            sql << "'" << appointment.get_reason().value() << "', ";
        } else {
            sql << "NULL, ";
        }
        
        sql << static_cast<int>(appointment.get_status()) << ");";

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create appointment: " + std::string(e.what()));
        return false;
    }
}

std::optional<Appointment> AppointmentDAO::get_appointment_by_id(int appointment_id) {
    try {
        std::stringstream sql;
        sql << "SELECT id, user_id, pet_id, doctor_id, start_time, end_time, reason, status, created_at, updated_at FROM appointments WHERE id = " << appointment_id;

        std::vector<std::vector<std::string>> result;
        if (!database_->execute_query(sql.str(), result)) {
            return std::nullopt;
        }

        if (result.empty()) {
            return std::nullopt;
        }

        const auto& row = result[0];
        int id = std::stoi(row[0]);
        int user_id = std::stoi(row[1]);
        int pet_id = std::stoi(row[2]);
        int doctor_id = std::stoi(row[3]);
        std::string start_time = row[4];
        std::string end_time = row[5];
        std::optional<std::string> reason = row[6].empty() ? std::nullopt : std::optional<std::string>(row[6]);
        Appointment::Status status = static_cast<Appointment::Status>(std::stoi(row[7]));
        std::string created_at = row[8];
        std::string updated_at = row[9];

        return Appointment(id, user_id, pet_id, doctor_id, start_time, end_time, reason, status, created_at, updated_at);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get appointment by ID: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::vector<Appointment> AppointmentDAO::get_appointments_by_user_id(int user_id, 
                                                               const std::string& from, 
                                                               const std::string& to, 
                                                               int page, int page_size) {
    try {
        std::vector<Appointment> appointments;
        
        std::stringstream sql;
        sql << "SELECT id, user_id, pet_id, doctor_id, start_time, end_time, reason, status, created_at, updated_at FROM appointments WHERE user_id = " << user_id;
        
        // 添加时间范围筛选
        if (!from.empty() && !to.empty()) {
            sql << " AND start_time >= '" << from << "' AND end_time <= '" << to << "'";
        } else if (!from.empty()) {
            sql << " AND start_time >= '" << from << "'";
        } else if (!to.empty()) {
            sql << " AND end_time <= '" << to << "'";
        }
        
        // 添加分页
        if (page > 0 && page_size > 0) {
            int offset = (page - 1) * page_size;
            sql << " LIMIT " << page_size << " OFFSET " << offset;
        }

        std::vector<std::vector<std::string>> result;
        if (!database_->execute_query(sql.str(), result)) {
            return appointments;
        }

        for (const auto& row : result) {
            int id = std::stoi(row[0]);
            int appointment_user_id = std::stoi(row[1]);
            int pet_id = std::stoi(row[2]);
            int doctor_id = std::stoi(row[3]);
            std::string start_time = row[4];
            std::string end_time = row[5];
            std::optional<std::string> reason = row[6].empty() ? std::nullopt : std::optional<std::string>(row[6]);
            Appointment::Status status = static_cast<Appointment::Status>(std::stoi(row[7]));
            std::string created_at = row[8];
            std::string updated_at = row[9];

            appointments.emplace_back(id, appointment_user_id, pet_id, doctor_id, start_time, end_time, reason, status, created_at, updated_at);
        }

        return appointments;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get appointments by user ID: " + std::string(e.what()));
        return {};
    }
}

std::vector<Appointment> AppointmentDAO::get_appointments_by_doctor_id(int doctor_id, 
                                                                 const std::string& date) {
    try {
        std::vector<Appointment> appointments;
        
        std::stringstream sql;
        sql << "SELECT id, user_id, pet_id, doctor_id, start_time, end_time, reason, status, created_at, updated_at FROM appointments WHERE doctor_id = " << doctor_id;
        
        // 添加日期筛选
        if (!date.empty()) {
            sql << " AND DATE(start_time) = '" << date << "'";
        }
        
        sql << " ORDER BY start_time";

        std::vector<std::vector<std::string>> result;
        if (!database_->execute_query(sql.str(), result)) {
            return appointments;
        }

        for (const auto& row : result) {
            int id = std::stoi(row[0]);
            int user_id = std::stoi(row[1]);
            int pet_id = std::stoi(row[2]);
            int appointment_doctor_id = std::stoi(row[3]);
            std::string start_time = row[4];
            std::string end_time = row[5];
            std::optional<std::string> reason = row[6].empty() ? std::nullopt : std::optional<std::string>(row[6]);
            Appointment::Status status = static_cast<Appointment::Status>(std::stoi(row[7]));
            std::string created_at = row[8];
            std::string updated_at = row[9];

            appointments.emplace_back(id, user_id, pet_id, appointment_doctor_id, start_time, end_time, reason, status, created_at, updated_at);
        }

        return appointments;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get appointments by doctor ID: " + std::string(e.what()));
        return {};
    }
}

std::vector<Appointment> AppointmentDAO::get_appointments_by_pet_id(int pet_id) {
    try {
        std::vector<Appointment> appointments;
        
        std::stringstream sql;
        sql << "SELECT id, user_id, pet_id, doctor_id, start_time, end_time, reason, status, created_at, updated_at FROM appointments WHERE pet_id = " << pet_id;

        std::vector<std::vector<std::string>> result;
        if (!database_->execute_query(sql.str(), result)) {
            return appointments;
        }

        for (const auto& row : result) {
            int id = std::stoi(row[0]);
            int user_id = std::stoi(row[1]);
            int appointment_pet_id = std::stoi(row[2]);
            int doctor_id = std::stoi(row[3]);
            std::string start_time = row[4];
            std::string end_time = row[5];
            std::optional<std::string> reason = row[6].empty() ? std::nullopt : std::optional<std::string>(row[6]);
            Appointment::Status status = static_cast<Appointment::Status>(std::stoi(row[7]));
            std::string created_at = row[8];
            std::string updated_at = row[9];

            appointments.emplace_back(id, user_id, appointment_pet_id, doctor_id, start_time, end_time, reason, status, created_at, updated_at);
        }

        return appointments;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get appointments by pet ID: " + std::string(e.what()));
        return {};
    }
}

bool AppointmentDAO::cancel_appointment(int appointment_id) {
    try {
        std::stringstream sql;
        sql << "UPDATE appointments SET status = " << static_cast<int>(Appointment::Status::CANCELLED) 
            << ", updated_at = CURRENT_TIMESTAMP WHERE id = " << appointment_id;

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to cancel appointment: " + std::string(e.what()));
        return false;
    }
}

bool AppointmentDAO::update_appointment_status(int appointment_id, Appointment::Status status) {
    try {
        std::stringstream sql;
        sql << "UPDATE appointments SET status = " << static_cast<int>(status) 
            << ", updated_at = CURRENT_TIMESTAMP WHERE id = " << appointment_id;

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update appointment status: " + std::string(e.what()));
        return false;
    }
}

bool AppointmentDAO::check_appointment_conflict(int doctor_id, int pet_id, 
                                      const std::string& start_time, 
                                      const std::string& end_time, 
                                      int exclude_appointment_id) {
    try {
        std::stringstream sql;
        sql << "SELECT COUNT(*) FROM appointments WHERE (" 
            << "(doctor_id = " << doctor_id << " OR pet_id = " << pet_id << ") " 
            << "AND (" 
            << "(start_time < '" << end_time << "' AND end_time > '" << start_time << "') " 
            << ") " 
            << "AND status != " << static_cast<int>(Appointment::Status::CANCELLED);
            
        if (exclude_appointment_id > 0) {
            sql << " AND id != " << exclude_appointment_id;
        }
        
        sql << ")";

        std::vector<std::vector<std::string>> result;
        if (!database_->execute_query(sql.str(), result)) {
            return true; // 发生错误时默认返回有冲突
        }

        if (result.empty() || result[0].empty()) {
            return true;
        }

        int count = std::stoi(result[0][0]);
        return count > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to check appointment conflict: " + std::string(e.what()));
        return true;
    }
}

} // namespace pet_hospital
