#include "dao/PetDAO.h"
#include <sstream>

namespace pet_hospital {

bool PetDAO::create_pet(const Pet& pet) {
    try {
        std::stringstream sql;
        sql << "INSERT INTO pets (user_id, name, species, breed, gender, birthday, weight, notes) VALUES (" 
            << pet.get_user_id() << ", "
            << "'" << pet.get_name() << "', "
            << "'" << pet.get_species() << "', ";
        
        if (pet.get_breed()) {
            sql << "'" << pet.get_breed().value() << "', ";
        } else {
            sql << "NULL, ";
        }
        
        sql << static_cast<int>(pet.get_gender()) << ", ";
        
        if (pet.get_birthday()) {
            sql << "'" << pet.get_birthday().value() << "', ";
        } else {
            sql << "NULL, ";
        }
        
        if (pet.get_weight()) {
            sql << pet.get_weight().value() << ", ";
        } else {
            sql << "NULL, ";
        }
        
        if (pet.get_notes()) {
            sql << "'" << pet.get_notes().value() << "');";
        } else {
            sql << "NULL);";
        }

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create pet: " + std::string(e.what()));
        return false;
    }
}

std::optional<Pet> PetDAO::get_pet_by_id(int pet_id) {
    try {
        std::stringstream sql;
        sql << "SELECT id, user_id, name, species, breed, gender, birthday, weight, notes, created_at, updated_at FROM pets WHERE id = " << pet_id;

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
        std::string name = row[2];
        std::string species = row[3];
        std::optional<std::string> breed = row[4].empty() ? std::nullopt : std::optional<std::string>(row[4]);
        Pet::Gender gender = static_cast<Pet::Gender>(std::stoi(row[5]));
        std::optional<std::string> birthday = row[6].empty() ? std::nullopt : std::optional<std::string>(row[6]);
        std::optional<double> weight = row[7].empty() ? std::nullopt : std::optional<double>(std::stod(row[7]));
        std::optional<std::string> notes = row[8].empty() ? std::nullopt : std::optional<std::string>(row[8]);
        std::string created_at = row[9];
        std::string updated_at = row[10];

        return Pet(id, user_id, name, species, breed, gender, birthday, weight, notes, created_at, updated_at);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get pet by ID: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::vector<Pet> PetDAO::get_pets_by_user_id(int user_id, int page, int page_size) {
    try {
        std::vector<Pet> pets;
        
        std::stringstream sql;
        sql << "SELECT id, user_id, name, species, breed, gender, birthday, weight, notes, created_at, updated_at FROM pets WHERE user_id = " << user_id;
        
        // 添加分页
        if (page > 0 && page_size > 0) {
            int offset = (page - 1) * page_size;
            sql << " LIMIT " << page_size << " OFFSET " << offset;
        }

        std::vector<std::vector<std::string>> result;
        if (!database_->execute_query(sql.str(), result)) {
            return pets;
        }

        for (const auto& row : result) {
            int id = std::stoi(row[0]);
            int pet_user_id = std::stoi(row[1]);
            std::string name = row[2];
            std::string species = row[3];
            std::optional<std::string> breed = row[4].empty() ? std::nullopt : std::optional<std::string>(row[4]);
            Pet::Gender gender = static_cast<Pet::Gender>(std::stoi(row[5]));
            std::optional<std::string> birthday = row[6].empty() ? std::nullopt : std::optional<std::string>(row[6]);
            std::optional<double> weight = row[7].empty() ? std::nullopt : std::optional<double>(std::stod(row[7]));
            std::optional<std::string> notes = row[8].empty() ? std::nullopt : std::optional<std::string>(row[8]);
            std::string created_at = row[9];
            std::string updated_at = row[10];

            pets.emplace_back(id, pet_user_id, name, species, breed, gender, birthday, weight, notes, created_at, updated_at);
        }

        return pets;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get pets by user ID: " + std::string(e.what()));
        return {};
    }
}

bool PetDAO::update_pet(const Pet& pet) {
    try {
        std::stringstream sql;
        sql << "UPDATE pets SET " 
            << "user_id = " << pet.get_user_id() << ", "
            << "name = '" << pet.get_name() << "', "
            << "species = '" << pet.get_species() << "', ";
        
        if (pet.get_breed()) {
            sql << "breed = '" << pet.get_breed().value() << "', ";
        } else {
            sql << "breed = NULL, ";
        }
        
        sql << "gender = " << static_cast<int>(pet.get_gender()) << ", ";
        
        if (pet.get_birthday()) {
            sql << "birthday = '" << pet.get_birthday().value() << "', ";
        } else {
            sql << "birthday = NULL, ";
        }
        
        if (pet.get_weight()) {
            sql << "weight = " << pet.get_weight().value() << ", ";
        } else {
            sql << "weight = NULL, ";
        }
        
        if (pet.get_notes()) {
            sql << "notes = '" << pet.get_notes().value() << "', ";
        } else {
            sql << "notes = NULL, ";
        }
        
        sql << "updated_at = CURRENT_TIMESTAMP WHERE id = " << pet.get_id();

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update pet: " + std::string(e.what()));
        return false;
    }
}

bool PetDAO::delete_pet(int pet_id) {
    try {
        std::stringstream sql;
        sql << "DELETE FROM pets WHERE id = " << pet_id;

        int affected_rows;
        if (!database_->execute_statement(sql.str(), affected_rows)) {
            return false;
        }

        return affected_rows > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to delete pet: " + std::string(e.what()));
        return false;
    }
}

} // namespace pet_hospital
