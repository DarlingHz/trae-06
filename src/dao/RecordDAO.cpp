#include "dao/RecordDAO.h"
#include <sstream>

namespace pet_hospital {

bool RecordDAO::create_record(const Record& record) {
    try {
        std::stringstream sql;
        sql << "INSERT INTO records (appointment_id, chief_complaint, diagnosis, treatment, notes) VALUES (" 
            << record.get_appointment_id() << ", ";
        
        if (record.get_chief_complaint()) {
            sql << "'" << record.get_chief_complaint().value() << "', ";
        } else {
            sql << "NULL, ";
        }
        
        if (record.get_diagnosis()) {
            sql << "'" << record.get_diagnosis().value() << "', ";
        } else {
            sql << "NULL, ";
        }
        
        if (record.get_treatment()) {
            sql << "'" << record.get_treatment().value() << "', ";
        } else {
            sql << "NULL, ";
        }
        
        if (record.get_notes()) {
            sql << "'" << record.get_notes().value() << "');";
        } else {
            sql << "NULL);";
        }

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create record: " + std::string(e.what()));
        return false;
    }
}

std::optional<Record> RecordDAO::get_record_by_id(int record_id) {
    try {
        std::stringstream sql;
        sql << "SELECT id, appointment_id, chief_complaint, diagnosis, treatment, doctor_notes, created_at, updated_at FROM records WHERE id = " << record_id;

        std::vector<std::vector<std::string>> result;
        if (!database_->execute_query(sql.str(), result)) {
            return std::nullopt;
        }

        if (result.empty()) {
            return std::nullopt;
        }

        const auto& row = result[0];
        int id = std::stoi(row[0]);
        int appointment_id = std::stoi(row[1]);
        std::optional<std::string> chief_complaint = row[2].empty() ? std::nullopt : std::optional<std::string>(row[2]);
        std::optional<std::string> diagnosis = row[3].empty() ? std::nullopt : std::optional<std::string>(row[3]);
        std::optional<std::string> treatment = row[4].empty() ? std::nullopt : std::optional<std::string>(row[4]);
        std::optional<std::string> doctor_notes = row[5].empty() ? std::nullopt : std::optional<std::string>(row[5]);
        std::string created_at = row[6];
        std::string updated_at = row[7];

        return Record(id, appointment_id, chief_complaint, diagnosis, treatment, std::nullopt, doctor_notes, created_at, updated_at);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get record by ID: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::vector<Record> RecordDAO::get_records_by_pet_id(int pet_id, int page, int page_size) {
    try {
        std::vector<Record> records;
        
        std::stringstream sql;
        sql << "SELECT r.id, r.appointment_id, r.chief_complaint, r.diagnosis, r.treatment, r.doctor_notes, r.created_at, r.updated_at " 
            << "FROM records r JOIN appointments a ON r.appointment_id = a.id " 
            << "WHERE a.pet_id = " << pet_id << " ORDER BY r.created_at DESC";
        
        // 添加分页
        if (page > 0 && page_size > 0) {
            int offset = (page - 1) * page_size;
            sql << " LIMIT " << page_size << " OFFSET " << offset;
        }

        std::vector<std::vector<std::string>> result;
        if (!database_->execute_query(sql.str(), result)) {
            return records;
        }

        for (const auto& row : result) {
            int id = std::stoi(row[0]);
            int appointment_id = std::stoi(row[1]);
            std::optional<std::string> chief_complaint = row[2].empty() ? std::nullopt : std::optional<std::string>(row[2]);
            std::optional<std::string> diagnosis = row[3].empty() ? std::nullopt : std::optional<std::string>(row[3]);
            std::optional<std::string> treatment = row[4].empty() ? std::nullopt : std::optional<std::string>(row[4]);
            std::optional<std::string> doctor_notes = row[5].empty() ? std::nullopt : std::optional<std::string>(row[5]);
            std::string created_at = row[6];
            std::string updated_at = row[7];

            records.emplace_back(id, appointment_id, chief_complaint, diagnosis, treatment, doctor_notes, created_at, updated_at);
        }

        return records;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get records by pet ID: " + std::string(e.what()));
        return {};
    }
}

bool RecordDAO::update_record(const Record& record) {
    try {
        std::stringstream sql;
        sql << "UPDATE records SET " 
            << "appointment_id = " << record.get_appointment_id() << ", ";
        
        if (record.get_chief_complaint().has_value()) {
            sql << "chief_complaint = '" << record.get_chief_complaint().value() << "', ";
        } else {
            sql << "chief_complaint = NULL, ";
        }
        
        if (record.get_diagnosis().has_value()) {
            sql << "diagnosis = '" << record.get_diagnosis().value() << "', ";
        } else {
            sql << "diagnosis = NULL, ";
        }
        
        if (record.get_treatment()) {
            sql << "treatment = '" << record.get_treatment().value() << "', ";
        } else {
            sql << "treatment = NULL, ";
        }
        
        if (record.get_notes().has_value()) {
            sql << "notes = '" << record.get_notes().value() << "', ";
        } else {
            sql << "notes = NULL, ";
        }
        
        sql << "updated_at = CURRENT_TIMESTAMP WHERE id = " << record.get_id();

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update record: " + std::string(e.what()));
        return false;
    }
}

bool RecordDAO::delete_record(int record_id) {
    try {
        std::stringstream sql;
        sql << "DELETE FROM records WHERE id = " << record_id;

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to delete record: " + std::string(e.what()));
        return false;
    }
}

} // namespace pet_hospital
