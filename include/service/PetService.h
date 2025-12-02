#pragma once

#include <string>
#include <optional>
#include <vector>
#include "models/Pet.h"

namespace pet_hospital {

class PetDAO;

class PetService {
public:
    PetService();
    ~PetService();

    // 新增宠物
    std::optional<Pet> add_pet(int user_id, const std::string& name, 
                                 const std::string& species, const std::optional<std::string>& breed, 
                                 Pet::Gender gender, const std::optional<std::string>& birthday, 
                                 const std::optional<double>& weight, const std::optional<std::string>& notes, 
                                 std::string& error_message);

    // 查询当前用户名下宠物列表
    std::vector<Pet> get_pets(int user_id, std::string& error_message, int page = 1, int page_size = 10);

    // 根据ID查询宠物
    std::optional<Pet> get_pet_by_id(int pet_id, std::string& error_message);

    // 修改宠物信息
    bool update_pet(int pet_id, int user_id, const std::string& name, 
                     const std::string& species, const std::optional<std::string>& breed, 
                     Pet::Gender gender, const std::optional<std::string>& birthday, 
                     const std::optional<double>& weight, const std::optional<std::string>& notes, 
                     std::string& error_message);

    // 删除宠物
    bool delete_pet(int pet_id, int user_id, std::string& error_message);

private:
    // 验证宠物是否属于用户
    bool is_pet_owner(int pet_id, int user_id);

    std::unique_ptr<PetDAO> pet_dao_;
};

} // namespace pet_hospital
