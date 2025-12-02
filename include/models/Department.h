#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace pet_hospital {

class Department {
public:
    Department() = default;
    Department(int id, const std::string& name, const std::string& description, 
               const std::string& created_at, const std::string& updated_at)
        : id(id), name(name), description(description), 
          created_at(created_at), updated_at(updated_at) {}

    // Getters
    int get_id() const { return id; }
    const std::string& get_name() const { return name; }
    const std::string& get_description() const { return description; }
    const std::string& get_created_at() const { return created_at; }
    const std::string& get_updated_at() const { return updated_at; }

    // Setters
    void set_id(int id) { this->id = id; }
    void set_name(const std::string& name) { this->name = name; }
    void set_description(const std::string& description) { this->description = description; }
    void set_created_at(const std::string& created_at) { this->created_at = created_at; }
    void set_updated_at(const std::string& updated_at) { this->updated_at = updated_at; }

    // JSON 序列化和反序列化
    friend void to_json(json& j, const Department& department) {
        j = json{
            {"id", department.id},
            {"name", department.name},
            {"description", department.description},
            {"created_at", department.created_at},
            {"updated_at", department.updated_at}
        };
    }

    friend void from_json(const json& j, Department& department) {
        j.at("name").get_to(department.name);
        if (j.contains("description")) {
            department.description = j.at("description").get<std::string>();
        }
    }

private:
    int id = 0;
    std::string name;
    std::string description;
    std::string created_at;
    std::string updated_at;
};

} // namespace pet_hospital
