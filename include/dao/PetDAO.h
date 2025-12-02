#pragma once

#include <string>
#include <optional>
#include <vector>
#include "BaseDAO.h"
#include "models/Pet.h"

namespace pet_hospital {

class PetDAO : public BaseDAO {
public:
    PetDAO() = default;
    ~PetDAO() override = default;

    // 创建宠物
    bool create_pet(const Pet& pet);

    // 根据 ID 查询宠物
    std::optional<Pet> get_pet_by_id(int pet_id);

    // 根据用户 ID 查询宠物列表
    std::vector<Pet> get_pets_by_user_id(int user_id, int page = 1, int page_size = 10);

    // 更新宠物信息
    bool update_pet(const Pet& pet);

    // 删除宠物
    bool delete_pet(int pet_id);
};

} // namespace pet_hospital
