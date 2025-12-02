#include "service/RecordService.h"
#include "dao/RecordDAO.h"
#include "dao/AppointmentDAO.h"
#include "dao/PetDAO.h"
#include "dao/UserDAO.h"
#include "logging/Logging.h"

namespace pet_hospital {

RecordService::RecordService() {
    record_dao_ = std::make_unique<RecordDAO>();
    appointment_dao_ = std::make_unique<AppointmentDAO>();
    pet_dao_ = std::make_unique<PetDAO>();
    user_dao_ = std::make_unique<UserDAO>();
}

RecordService::~RecordService() {
}

std::optional<Record> RecordService::create_record(int appointment_id, const std::string& chief_complaint, 
                                          const std::string& diagnosis, const std::string& treatment, 
                                          const std::optional<std::string>& notes, std::string& error_message) {
    try {
        // 验证病例记录参数
        if (!validate_record_params(appointment_id, chief_complaint, diagnosis, treatment, error_message)) {
            LOG_ERROR("Create record failed: " + error_message);
            return std::nullopt;
        }

        // 验证预约是否可以创建病例记录
        if (!validate_appointment_for_record(appointment_id, error_message)) {
            LOG_ERROR("Create record failed: " + error_message);
            return std::nullopt;
        }

        // 获取预约信息
        auto appointment = appointment_dao_->get_appointment_by_id(appointment_id);
        if (!appointment) {
            error_message = "Appointment not found";
            LOG_ERROR("Create record failed: " + error_message);
            return std::nullopt;
        }

        // 创建病例记录对象
        Record record;
        record.set_appointment_id(appointment_id);
        record.set_chief_complaint(chief_complaint);
        record.set_diagnosis(diagnosis);
        record.set_treatment(treatment);
        if (notes) {
            record.set_notes(notes.value());
        }

        // 保存病例记录到数据库
        if (!record_dao_->create_record(record)) {
            error_message = "Failed to create record";
            LOG_ERROR("Create record failed: " + error_message);
            return std::nullopt;
        }

        // 获取创建的病例记录信息
        auto created_record = record_dao_->get_record_by_id(record.get_id());
        if (!created_record) {
            error_message = "Failed to retrieve created record";
            LOG_ERROR("Create record failed: " + error_message);
            return std::nullopt;
        }

        LOG_INFO("Create record successfully: " + std::to_string(created_record->get_id()) + 
                 " (appointment: " + std::to_string(appointment_id) + ")");
        return created_record;
    } catch (const std::exception& e) {
        error_message = "Failed to create record: " + std::string(e.what());
        LOG_ERROR("Create record failed: " + error_message);
        return std::nullopt;
    }
}

std::optional<Record> RecordService::get_record_by_id(int record_id, std::string& error_message) {
    try {
        // 验证病例记录ID
        if (record_id <= 0) {
            error_message = "Invalid record ID";
            LOG_ERROR("Get record by ID failed: " + error_message);
            return std::nullopt;
        }

        // 获取病例记录信息
        auto record = record_dao_->get_record_by_id(record_id);
        if (!record) {
            error_message = "Record not found";
            LOG_ERROR("Get record by ID failed: " + error_message);
            return std::nullopt;
        }

        LOG_INFO("Get record by ID successfully: " + std::to_string(record_id));
        return record;
    } catch (const std::exception& e) {
        error_message = "Failed to get record by ID: " + std::string(e.what());
        LOG_ERROR("Get record by ID failed: " + error_message);
        return std::nullopt;
    }
}

std::vector<Record> RecordService::get_records_by_pet_id(int pet_id, int user_id, int page, int page_size, 
                                                   std::string& error_message) {
    try {
        // 验证宠物ID和用户ID
        if (pet_id <= 0 || user_id <= 0) {
            error_message = "Invalid pet ID or user ID";
            LOG_ERROR("Get records by pet ID failed: " + error_message);
            return {};
        }

        // 验证宠物是否属于该用户
        if (!validate_pet_ownership(pet_id, user_id, error_message)) {
            LOG_ERROR("Get records by pet ID failed: " + error_message);
            return {};
        }

        // 验证分页参数
        if (page <= 0) {
            page = 1;
        }
        if (page_size <= 0 || page_size > 100) {
            page_size = 10;
        }

        // 获取宠物病例记录列表
        auto records = record_dao_->get_records_by_pet_id(pet_id);

        // 按时间倒序排序（最新的病例记录在前）
        std::sort(records.begin(), records.end(), [](const Record& a, const Record& b) {
            return a.get_created_at() > b.get_created_at();
        });

        // 分页处理
        int start_index = (page - 1) * page_size;
        int end_index = start_index + page_size;
        if (static_cast<size_t>(start_index) >= records.size()) {
            return {};
        }
        if (static_cast<size_t>(end_index) > records.size()) {
            end_index = records.size();
        }

        std::vector<Record> paginated_records(records.begin() + start_index, records.begin() + end_index);

        LOG_INFO("Get records by pet ID successfully: " + std::to_string(paginated_records.size()) + 
                 " records (pet: " + std::to_string(pet_id) + ", user: " + std::to_string(user_id) + ")");
        return paginated_records;
    } catch (const std::exception& e) {
        error_message = "Failed to get records by pet ID: " + std::string(e.what());
        LOG_ERROR("Get records by pet ID failed: " + error_message);
        return {};
    }
}

bool RecordService::update_record(int record_id, int appointment_id, const std::string& chief_complaint, 
                       const std::string& diagnosis, const std::string& treatment, 
                       const std::optional<std::string>& notes, std::string& error_message) {
    try {
        // 验证病例记录ID
        if (record_id <= 0) {
            error_message = "Invalid record ID";
            LOG_ERROR("Update record failed: " + error_message);
            return false;
        }

        // 验证病例记录参数
        if (!validate_record_params(appointment_id, chief_complaint, diagnosis, treatment, error_message)) {
            LOG_ERROR("Update record failed: " + error_message);
            return false;
        }

        // 验证预约是否可以创建病例记录
        if (!validate_appointment_for_record(appointment_id, error_message)) {
            LOG_ERROR("Update record failed: " + error_message);
            return false;
        }

        // 获取病例记录信息
        auto record = record_dao_->get_record_by_id(record_id);
        if (!record) {
            error_message = "Record not found";
            LOG_ERROR("Update record failed: " + error_message);
            return false;
        }

        // 更新病例记录信息
        record->set_appointment_id(appointment_id);
        record->set_chief_complaint(chief_complaint);
        record->set_diagnosis(diagnosis);
        record->set_treatment(treatment);
        if (notes) {
            record->set_notes(notes.value());
        } else {
            record->set_notes(std::nullopt);
        }

        // 保存更新后的病例记录信息
        if (!record_dao_->update_record(*record)) {
            error_message = "Failed to update record";
            LOG_ERROR("Update record failed: " + error_message);
            return false;
        }

        LOG_INFO("Update record successfully: " + std::to_string(record_id) + 
                 " (appointment: " + std::to_string(appointment_id) + ")");
        return true;
    } catch (const std::exception& e) {
        error_message = "Failed to update record: " + std::string(e.what());
        LOG_ERROR("Update record failed: " + error_message);
        return false;
    }
}

bool RecordService::delete_record(int record_id, std::string& error_message) {
    try {
        // 验证病例记录ID
        if (record_id <= 0) {
            error_message = "Invalid record ID";
            LOG_ERROR("Delete record failed: " + error_message);
            return false;
        }

        // 获取病例记录信息
        auto record = record_dao_->get_record_by_id(record_id);
        if (!record) {
            error_message = "Record not found";
            LOG_ERROR("Delete record failed: " + error_message);
            return false;
        }

        // 删除病例记录
        if (!record_dao_->delete_record(record_id)) {
            error_message = "Failed to delete record";
            LOG_ERROR("Delete record failed: " + error_message);
            return false;
        }

        LOG_INFO("Delete record successfully: " + std::to_string(record_id));
        return true;
    } catch (const std::exception& e) {
        error_message = "Failed to delete record: " + std::string(e.what());
        LOG_ERROR("Delete record failed: " + error_message);
        return false;
    }
}

bool RecordService::validate_record_params(int appointment_id, const std::string& chief_complaint, 
                                 const std::string& diagnosis, const std::string& treatment, 
                                 std::string& error_message) {
    // 验证预约ID
    if (appointment_id <= 0) {
        error_message = "Invalid appointment ID";
        return false;
    }

    // 验证主诉
    if (chief_complaint.empty()) {
        error_message = "Chief complaint cannot be empty";
        return false;
    }

    // 验证诊断结论
    if (diagnosis.empty()) {
        error_message = "Diagnosis cannot be empty";
        return false;
    }

    // 验证用药建议
    if (treatment.empty()) {
        error_message = "Treatment cannot be empty";
        return false;
    }

    return true;
}

bool RecordService::validate_appointment_for_record(int appointment_id, std::string& error_message) {
    // 获取预约信息
    auto appointment = appointment_dao_->get_appointment_by_id(appointment_id);
    if (!appointment) {
        error_message = "Appointment not found";
        return false;
    }

    // 验证预约状态是否为已完成
    if (appointment->get_status() != Appointment::Status::COMPLETED) {
        error_message = "Only completed appointments can create records";
        return false;
    }

    return true;
}

bool RecordService::validate_pet_ownership(int pet_id, int user_id, std::string& error_message) {
    // 获取宠物信息
    auto pet = pet_dao_->get_pet_by_id(pet_id);
    if (!pet) {
        error_message = "Pet not found";
        return false;
    }

    // 验证宠物是否属于该用户
    if (pet->get_user_id() != user_id) {
        error_message = "Pet does not belong to the user";
        return false;
    }

    return true;
}

} // namespace pet_hospital
