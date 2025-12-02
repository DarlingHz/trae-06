#include "dao/DoctorDAO.h"
#include <sstream>

namespace pet_hospital {

bool DoctorDAO::create_doctor(const Doctor& doctor) {
    try {
        std::stringstream sql;
        sql << "INSERT INTO doctors (department_id, name, title, specialty, available_time) VALUES (" 
            << doctor.get_department_id() << ", "
            << "'" << doctor.get_name() << "', ";
        
        if (doctor.get_title()) {
            sql << "'" << doctor.get_title().value() << "', ";
        } else {
            sql << "NULL, ";
        }
        
        if (doctor.get_specialty()) {
            sql << "'" << doctor.get_specialty().value() << "', ";
        } else {
            sql << "NULL, ";
        }
        
        sql << "'" << doctor.get_available_start() << "', ";
        sql << "'" << doctor.get_available_end() << "');";

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create doctor: " + std::string(e.what()));
        return false;
    }
}

std::optional<Doctor> DoctorDAO::get_doctor_by_id(int doctor_id) {
    try {
        std::stringstream sql;
        sql << "SELECT id, department_id, name, title, specialty, phone, email, available_start, available_end, is_active, created_at, updated_at FROM doctors WHERE id = " << doctor_id;

        std::vector<std::vector<std::string>> result;
        if (!database_->execute_query(sql.str(), result)) {
            return std::nullopt;
        }

        if (result.empty()) {
            return std::nullopt;
        }

        const auto& row = result[0];
        int id = std::stoi(row[0]);
        int department_id = std::stoi(row[1]);
        std::string name = row[2];
        std::optional<std::string> title = row[3].empty() ? std::nullopt : std::optional<std::string>(row[3]);
        std::optional<std::string> specialty = row[4].empty() ? std::nullopt : std::optional<std::string>(row[4]);
        std::optional<std::string> phone = row[5].empty() ? std::nullopt : std::optional<std::string>(row[5]);
        std::optional<std::string> email = row[6].empty() ? std::nullopt : std::optional<std::string>(row[6]);
        std::string available_start = row[7];
        std::string available_end = row[8];
        bool is_active = row[9] == "1";
        std::string created_at = row[10];
        std::string updated_at = row[11];

        return Doctor(id, department_id, name, title, specialty, phone, email, available_start, available_end, is_active, created_at, updated_at);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get doctor by ID: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::vector<Doctor> DoctorDAO::get_all_doctors() {
    try {
        std::vector<Doctor> doctors;
        
        std::stringstream sql;
        sql << "SELECT id, department_id, name, title, specialty, phone, email, available_start, available_end, is_active, created_at, updated_at FROM doctors ORDER BY name";

        std::vector<std::vector<std::string>> result;
        if (!database_->execute_query(sql.str(), result)) {
            return doctors;
        }

        for (const auto& row : result) {
            int id = std::stoi(row[0]);
            int department_id = std::stoi(row[1]);
            std::string name = row[2];
            std::optional<std::string> title = row[3].empty() ? std::nullopt : std::optional<std::string>(row[3]);
            std::optional<std::string> specialty = row[4].empty() ? std::nullopt : std::optional<std::string>(row[4]);
            std::optional<std::string> phone = row[5].empty() ? std::nullopt : std::optional<std::string>(row[5]);
            std::optional<std::string> email = row[6].empty() ? std::nullopt : std::optional<std::string>(row[6]);
            std::string available_start = row[7];
            std::string available_end = row[8];
            bool is_active = row[9] == "1";
            std::string created_at = row[10];
            std::string updated_at = row[11];

            doctors.emplace_back(id, department_id, name, title, specialty, phone, email, available_start, available_end, is_active, created_at, updated_at);
        }

        return doctors;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get all doctors: " + std::string(e.what()));
        return {};
    }
}

std::vector<Doctor> DoctorDAO::get_doctors_by_department_id(int department_id) {
    try {
        std::vector<Doctor> doctors;
        
        std::stringstream sql;
        sql << "SELECT id, department_id, name, title, specialty, phone, email, available_start, available_end, is_active, created_at, updated_at FROM doctors WHERE department_id = " << department_id << " ORDER BY name";

        std::vector<std::vector<std::string>> result;
        if (!database_->execute_query(sql.str(), result)) {
            return doctors;
        }

        for (const auto& row : result) {
            int id = std::stoi(row[0]);
            int dept_id = std::stoi(row[1]);
            std::string name = row[2];
            std::optional<std::string> title = row[3].empty() ? std::nullopt : std::optional<std::string>(row[3]);
            std::optional<std::string> specialty = row[4].empty() ? std::nullopt : std::optional<std::string>(row[4]);
            std::optional<std::string> phone = row[5].empty() ? std::nullopt : std::optional<std::string>(row[5]);
            std::optional<std::string> email = row[6].empty() ? std::nullopt : std::optional<std::string>(row[6]);
            std::string available_start = row[7];
            std::string available_end = row[8];
            bool is_active = row[9] == "1";
            std::string created_at = row[10];
            std::string updated_at = row[11];

            doctors.emplace_back(id, dept_id, name, title, specialty, phone, email, available_start, available_end, is_active, created_at, updated_at);
        }

        return doctors;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get doctors by department ID: " + std::string(e.what()));
        return {};
    }
}

bool DoctorDAO::update_doctor(const Doctor& doctor) {
    try {
        std::stringstream sql;
        sql << "UPDATE doctors SET " 
            << "department_id = " << doctor.get_department_id() << ", "
            << "name = '" << doctor.get_name() << "', ";
        
        if (doctor.get_title()) {
            sql << "title = '" << doctor.get_title().value() << "', ";
        } else {
            sql << "title = NULL, ";
        }
        
        if (doctor.get_specialty()) {
            sql << "specialty = '" << doctor.get_specialty().value() << "', ";
        } else {
            sql << "specialty = NULL, ";
        }
        
        sql << "available_start = '" << doctor.get_available_start() << "', ";
        sql << "available_end = '" << doctor.get_available_end() << "', ";
        
        sql << "updated_at = CURRENT_TIMESTAMP WHERE id = " << doctor.get_id();

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update doctor: " + std::string(e.what()));
        return false;
    }
}

bool DoctorDAO::delete_doctor(int doctor_id) {
    try {
        std::stringstream sql;
        sql << "DELETE FROM doctors WHERE id = " << doctor_id;

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to delete doctor: " + std::string(e.what()));
        return false;
    }
}

} // namespace pet_hospital
