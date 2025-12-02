#pragma once

#include <string>
#include <optional>
#include <vector>
#include "BaseDAO.h"
#include "models/Record.h"

namespace pet_hospital {

class RecordDAO : public BaseDAO {
public:
    RecordDAO() = default;
    ~RecordDAO() override = default;

    // 创建病例记录
    bool create_record(const Record& record);

    // 根据 ID 查询病例记录
    std::optional<Record> get_record_by_id(int record_id);

    // 查询宠物病例记录
    std::vector<Record> get_records_by_pet_id(int pet_id, int page = 1, int page_size = 10);

    // 更新病例记录
    bool update_record(const Record& record);

    // 删除病例记录
    bool delete_record(int record_id);
};

} // namespace pet_hospital
