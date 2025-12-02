#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace pet_hospital {

class Pet {
public:
    enum class Gender {
        UNKNOWN = 0,
        MALE = 1,
        FEMALE = 2
    };

    Pet() = default;
    Pet(int id, int user_id, const std::string& name, const std::string& species, 
        const std::optional<std::string>& breed, Gender gender, 
        const std::optional<std::string>& birthday, const std::optional<double>& weight, 
        const std::optional<std::string>& notes, const std::string& created_at, 
        const std::string& updated_at)
        : id(id), user_id(user_id), name(name), species(species), breed(breed), 
          gender(gender), birthday(birthday), weight(weight), notes(notes), 
          created_at(created_at), updated_at(updated_at) {}

    // Getters
    int get_id() const { return id; }
    int get_user_id() const { return user_id; }
    const std::string& get_name() const { return name; }
    const std::string& get_species() const { return species; }
    const std::optional<std::string>& get_breed() const { return breed; }
    Gender get_gender() const { return gender; }
    const std::optional<std::string>& get_birthday() const { return birthday; }
    const std::optional<double>& get_weight() const { return weight; }
    const std::optional<std::string>& get_notes() const { return notes; }
    const std::string& get_created_at() const { return created_at; }
    const std::string& get_updated_at() const { return updated_at; }

    // Setters
    void set_id(int id) { this->id = id; }
    void set_user_id(int user_id) { this->user_id = user_id; }
    void set_name(const std::string& name) { this->name = name; }
    void set_species(const std::string& species) { this->species = species; }
    void set_breed(const std::optional<std::string>& breed) { this->breed = breed; }
    void set_gender(Gender gender) { this->gender = gender; }
    void set_birthday(const std::optional<std::string>& birthday) { this->birthday = birthday; }
    void set_weight(const std::optional<double>& weight) { this->weight = weight; }
    void set_notes(const std::optional<std::string>& notes) { this->notes = notes; }
    void set_created_at(const std::string& created_at) { this->created_at = created_at; }
    void set_updated_at(const std::string& updated_at) { this->updated_at = updated_at; }

    // JSON 序列化和反序列化
    friend void to_json(json& j, const Pet& pet) {
        j = json{
            {"id", pet.id},
            {"user_id", pet.user_id},
            {"name", pet.name},
            {"species", pet.species},
            {"breed", pet.breed},
            {"gender", static_cast<int>(pet.gender)},
            {"birthday", pet.birthday},
            {"weight", pet.weight},
            {"notes", pet.notes},
            {"created_at", pet.created_at},
            {"updated_at", pet.updated_at}
        };
    }

    friend void from_json(const json& j, Pet& pet) {
        j.at("name").get_to(pet.name);
        j.at("species").get_to(pet.species);
        if (j.contains("breed")) {
            pet.breed = j.at("breed").get<std::string>();
        }
        if (j.contains("gender")) {
            int gender = j.at("gender").get<int>();
            pet.gender = static_cast<Gender>(gender);
        }
        if (j.contains("birthday")) {
            pet.birthday = j.at("birthday").get<std::string>();
        }
        if (j.contains("weight")) {
            pet.weight = j.at("weight").get<double>();
        }
        if (j.contains("notes")) {
            pet.notes = j.at("notes").get<std::string>();
        }
    }

private:
    int id = 0;
    int user_id = 0;
    std::string name;
    std::string species;
    std::optional<std::string> breed;
    Gender gender = Gender::UNKNOWN;
    std::optional<std::string> birthday;
    std::optional<double> weight;
    std::optional<std::string> notes;
    std::string created_at;
    std::string updated_at;
};

} // namespace pet_hospital
