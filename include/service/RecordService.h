#pragma once

#include <optional>
#include <vector>
#include <string>

#include "models/Record.h"
#include "dao/RecordDAO.h"
#include "dao/AppointmentDAO.h"
#include "dao/PetDAO.h"
#include "dao/UserDAO.h"

namespace pet_hospital {

class RecordService {
public:
    RecordService();
    ~RecordService();

    // 创建病例记录
    std::optional<Record> create_record(int appointment_id, const std::string& chief_complaint, 
                                          const std::string& diagnosis, const std::string& treatment, 
                                          const std::optional<std::string>& notes, std::string& error_message);

    // 根据ID获取病例记录
    std::optional<Record> get_record_by_id(int record_id, std::string& error_message);

    // 根据宠物ID获取病例记录
    std::vector<Record> get_records_by_pet_id(int pet_id, int user_id, int page, int page_size, 
                                                   std::string& error_message);

    // 更新病例记录
    bool update_record(int record_id, int appointment_id, const std::string& chief_complaint, 
                       const std::string& diagnosis, const std::string& treatment, 
                       const std::optional<std::string>& notes, std::string& error_message);

    // 删除病例记录
    bool delete_record(int record_id, std::string& error_message);

private:
    // 验证病例记录参数
    bool validate_record_params(int appointment_id, const std::string& chief_complaint, 
                                 const std::string& diagnosis, const std::string& treatment, 
                                 std::string& error_message);

    // 验证预约是否可以创建病例记录
    bool validate_appointment_for_record(int appointment_id, std::string& error_message);

    // 验证宠物是否属于该用户
    bool validate_pet_ownership(int pet_id, int user_id, std::string& error_message);

private:
    // 数据访问对象（DAO）的智能指针
    std::unique_ptr<RecordDAO> record_dao_;
    std::unique_ptr<AppointmentDAO> appointment_dao_;
    std::unique_ptr<PetDAO> pet_dao_;
    std::unique_ptr<UserDAO> user_dao_;
};

} // namespace pet_hospital
