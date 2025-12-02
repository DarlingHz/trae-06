#include "service/PetService.h"
#include "dao/PetDAO.h"
#include "logging/Logging.h"

namespace pet_hospital {

PetService::PetService() {
    pet_dao_ = std::make_unique<PetDAO>();
}

PetService::~PetService() {
}

std::optional<Pet> PetService::add_pet(int user_id, const std::string& name, 
                                 const std::string& species, const std::optional<std::string>& breed, 
                                 Pet::Gender gender, const std::optional<std::string>& birthday, 
                                 const std::optional<double>& weight, const std::optional<std::string>& notes, 
                                 std::string& error_message) {
    try {
        // 验证用户ID
        if (user_id <= 0) {
            error_message = "Invalid user ID";
            LOG_ERROR("Add pet failed: " + error_message);
            return std::nullopt;
        }

        // 验证宠物名称
        if (name.empty()) {
            error_message = "Pet name cannot be empty";
            LOG_ERROR("Add pet failed: " + error_message);
            return std::nullopt;
        }

        // 验证宠物种类
        if (species.empty()) {
            error_message = "Pet species cannot be empty";
            LOG_ERROR("Add pet failed: " + error_message);
            return std::nullopt;
        }

        // 创建宠物对象
        Pet pet;
        pet.set_user_id(user_id);
        pet.set_name(name);
        pet.set_species(species);
        if (breed) {
            pet.set_breed(breed.value());
        }
        pet.set_gender(gender);
        if (birthday) {
            pet.set_birthday(birthday.value());
        }
        if (weight) {
            pet.set_weight(weight.value());
        }
        if (notes) {
            pet.set_notes(notes.value());
        }

        // 保存宠物到数据库
        if (!pet_dao_->create_pet(pet)) {
            error_message = "Failed to create pet";
            LOG_ERROR("Add pet failed: " + error_message);
            return std::nullopt;
        }

        // 获取创建的宠物信息
        auto created_pet = pet_dao_->get_pet_by_id(pet.get_id());
        if (!created_pet) {
            error_message = "Failed to retrieve created pet";
            LOG_ERROR("Add pet failed: " + error_message);
            return std::nullopt;
        }

        LOG_INFO("Add pet successfully: " + name + " (user: " + std::to_string(user_id) + ")");
        return created_pet;
    } catch (const std::exception& e) {
        error_message = "Failed to add pet: " + std::string(e.what());
        LOG_ERROR("Add pet failed: " + error_message);
        return std::nullopt;
    }
}

std::vector<Pet> PetService::get_pets(int user_id, std::string& error_message, int page, int page_size) {
    try {
        // 验证用户ID
        if (user_id <= 0) {
            error_message = "Invalid user ID";
            LOG_ERROR("Get pets failed: " + error_message);
            return {};
        }

        // 验证分页参数
        if (page <= 0) {
            page = 1;
        }
        if (page_size <= 0) {
            page_size = 10;
        }

        // 获取宠物列表
        auto pets = pet_dao_->get_pets_by_user_id(user_id, page, page_size);

        LOG_INFO("Get pets successfully: " + std::to_string(pets.size()) + " pets (user: " + std::to_string(user_id) + ")");
        return pets;
    } catch (const std::exception& e) {
        error_message = "Failed to get pets: " + std::string(e.what());
        LOG_ERROR("Get pets failed: " + error_message);
        return {};
    }
}

std::optional<Pet> PetService::get_pet_by_id(int pet_id, std::string& error_message) {
    try {
        // 验证宠物ID
        if (pet_id <= 0) {
            error_message = "Invalid pet ID";
            LOG_ERROR("Get pet by ID failed: " + error_message);
            return std::nullopt;
        }

        // 获取宠物信息
        auto pet = pet_dao_->get_pet_by_id(pet_id);
        if (!pet) {
            error_message = "Pet not found";
            LOG_ERROR("Get pet by ID failed: " + error_message);
            return std::nullopt;
        }

        LOG_INFO("Get pet by ID successfully: " + std::to_string(pet_id));
        return pet;
    } catch (const std::exception& e) {
        error_message = "Failed to get pet by ID: " + std::string(e.what());
        LOG_ERROR("Get pet by ID failed: " + error_message);
        return std::nullopt;
    }
}

bool PetService::update_pet(int pet_id, int user_id, const std::string& name, 
                     const std::string& species, const std::optional<std::string>& breed, 
                     Pet::Gender gender, const std::optional<std::string>& birthday, 
                     const std::optional<double>& weight, const std::optional<std::string>& notes, 
                     std::string& error_message) {
    try {
        // 验证宠物ID和用户ID
        if (pet_id <= 0 || user_id <= 0) {
            error_message = "Invalid pet ID or user ID";
            LOG_ERROR("Update pet failed: " + error_message);
            return false;
        }

        // 验证宠物是否属于用户
        if (!is_pet_owner(pet_id, user_id)) {
            error_message = "You are not the owner of this pet";
            LOG_ERROR("Update pet failed: " + error_message);
            return false;
        }

        // 验证宠物名称
        if (name.empty()) {
            error_message = "Pet name cannot be empty";
            LOG_ERROR("Update pet failed: " + error_message);
            return false;
        }

        // 验证宠物种类
        if (species.empty()) {
            error_message = "Pet species cannot be empty";
            LOG_ERROR("Update pet failed: " + error_message);
            return false;
        }

        // 获取宠物信息
        auto pet = pet_dao_->get_pet_by_id(pet_id);
        if (!pet) {
            error_message = "Pet not found";
            LOG_ERROR("Update pet failed: " + error_message);
            return false;
        }

        // 更新宠物信息
        pet->set_name(name);
        pet->set_species(species);
        if (breed) {
            pet->set_breed(breed.value());
        } else {
            pet->set_breed(std::nullopt);
        }
        pet->set_gender(gender);
        if (birthday) {
            pet->set_birthday(birthday.value());
        } else {
            pet->set_birthday(std::nullopt);
        }
        if (weight) {
            pet->set_weight(weight.value());
        } else {
            pet->set_weight(std::nullopt);
        }
        if (notes) {
            pet->set_notes(notes.value());
        } else {
            pet->set_notes(std::nullopt);
        }

        // 保存更新后的宠物信息
        if (!pet_dao_->update_pet(*pet)) {
            error_message = "Failed to update pet";
            LOG_ERROR("Update pet failed: " + error_message);
            return false;
        }

        LOG_INFO("Update pet successfully: " + std::to_string(pet_id) + " (user: " + std::to_string(user_id) + ")");
        return true;
    } catch (const std::exception& e) {
        error_message = "Failed to update pet: " + std::string(e.what());
        LOG_ERROR("Update pet failed: " + error_message);
        return false;
    }
}

bool PetService::delete_pet(int pet_id, int user_id, std::string& error_message) {
    try {
        // 验证宠物ID和用户ID
        if (pet_id <= 0 || user_id <= 0) {
            error_message = "Invalid pet ID or user ID";
            LOG_ERROR("Delete pet failed: " + error_message);
            return false;
        }

        // 验证宠物是否属于用户
        if (!is_pet_owner(pet_id, user_id)) {
            error_message = "You are not the owner of this pet";
            LOG_ERROR("Delete pet failed: " + error_message);
            return false;
        }

        // 删除宠物
        if (!pet_dao_->delete_pet(pet_id)) {
            error_message = "Failed to delete pet";
            LOG_ERROR("Delete pet failed: " + error_message);
            return false;
        }

        LOG_INFO("Delete pet successfully: " + std::to_string(pet_id) + " (user: " + std::to_string(user_id) + ")");
        return true;
    } catch (const std::exception& e) {
        error_message = "Failed to delete pet: " + std::string(e.what());
        LOG_ERROR("Delete pet failed: " + error_message);
        return false;
    }
}

bool PetService::is_pet_owner(int pet_id, int user_id) {
    auto pet = pet_dao_->get_pet_by_id(pet_id);
    if (!pet) {
        return false;
    }
    return pet->get_user_id() == user_id;
}

} // namespace pet_hospital
