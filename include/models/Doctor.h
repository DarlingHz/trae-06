#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace pet_hospital {

class Doctor {
public:
    Doctor() = default;
    Doctor(int id, int department_id, const std::string& name, const std::optional<std::string>& title, 
           const std::optional<std::string>& specialty, const std::optional<std::string>& phone, 
           const std::optional<std::string>& email, const std::string& available_start, 
           const std::string& available_end, bool is_active, const std::string& created_at, 
           const std::string& updated_at)
        : id(id), department_id(department_id), name(name), title(title), specialty(specialty), 
          phone(phone), email(email), available_start(available_start), available_end(available_end), 
          is_active(is_active), created_at(created_at), updated_at(updated_at) {}

    // Getters
    int get_id() const { return id; }
    int get_department_id() const { return department_id; }
    const std::string& get_name() const { return name; }
    const std::optional<std::string>& get_title() const { return title; }
    const std::optional<std::string>& get_specialty() const { return specialty; }
    const std::optional<std::string>& get_phone() const { return phone; }
    const std::optional<std::string>& get_email() const { return email; }
    const std::string& get_available_start() const { return available_start; }
    const std::string& get_available_end() const { return available_end; }
    bool get_is_active() const { return is_active; }
    const std::string& get_created_at() const { return created_at; }
    const std::string& get_updated_at() const { return updated_at; }

    // Setters
    void set_id(int id) { this->id = id; }
    void set_department_id(int department_id) { this->department_id = department_id; }
    void set_name(const std::string& name) { this->name = name; }
    void set_title(const std::optional<std::string>& title) { this->title = title; }
    void set_specialty(const std::optional<std::string>& specialty) { this->specialty = specialty; }
    void set_phone(const std::optional<std::string>& phone) { this->phone = phone; }
    void set_email(const std::optional<std::string>& email) { this->email = email; }
    void set_available_start(const std::string& available_start) { this->available_start = available_start; }
    void set_available_end(const std::string& available_end) { this->available_end = available_end; }
    void set_is_active(bool is_active) { this->is_active = is_active; }
    void set_created_at(const std::string& created_at) { this->created_at = created_at; }
    void set_updated_at(const std::string& updated_at) { this->updated_at = updated_at; }

    // JSON 序列化和反序列化
    friend void to_json(json& j, const Doctor& doctor) {
        j = json{
            {"id", doctor.id},
            {"department_id", doctor.department_id},
            {"name", doctor.name},
            {"title", doctor.title},
            {"specialty", doctor.specialty},
            {"phone", doctor.phone},
            {"email", doctor.email},
            {"available_start", doctor.available_start},
            {"available_end", doctor.available_end},
            {"is_active", doctor.is_active},
            {"created_at", doctor.created_at},
            {"updated_at", doctor.updated_at}
        };
    }

    friend void from_json(const json& j, Doctor& doctor) {
        j.at("department_id").get_to(doctor.department_id);
        j.at("name").get_to(doctor.name);
        if (j.contains("title")) {
            doctor.title = j.at("title").get<std::string>();
        }
        if (j.contains("specialty")) {
            doctor.specialty = j.at("specialty").get<std::string>();
        }
        if (j.contains("phone")) {
            doctor.phone = j.at("phone").get<std::string>();
        }
        if (j.contains("email")) {
            doctor.email = j.at("email").get<std::string>();
        }
        if (j.contains("available_start")) {
            doctor.available_start = j.at("available_start").get<std::string>();
        }
        if (j.contains("available_end")) {
            doctor.available_end = j.at("available_end").get<std::string>();
        }
        if (j.contains("is_active")) {
            doctor.is_active = j.at("is_active").get<bool>();
        }
    }

private:
    int id = 0;
    int department_id = 0;
    std::string name;
    std::optional<std::string> title;
    std::optional<std::string> specialty;
    std::optional<std::string> phone;
    std::optional<std::string> email;
    std::string available_start = "08:00:00";
    std::string available_end = "18:00:00";
    bool is_active = true;
    std::string created_at;
    std::string updated_at;
};

} // namespace pet_hospital
