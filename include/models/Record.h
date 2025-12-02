#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace pet_hospital {

class Record {
public:
    Record() = default;
    Record(int id, int appointment_id, const std::optional<std::string>& chief_complaint, const std::optional<std::string>& diagnosis, 
           const std::optional<std::string>& treatment, const std::optional<std::string>& medication = std::nullopt, 
           const std::optional<std::string>& notes = std::nullopt, const std::string& created_at = "", 
           const std::string& updated_at = "")
        : id(id), appointment_id(appointment_id), chief_complaint(chief_complaint), diagnosis(diagnosis), 
          treatment(treatment), medication(medication), notes(notes), created_at(created_at), 
          updated_at(updated_at) {}

    // Getters
    int get_id() const { return id; }
    int get_appointment_id() const { return appointment_id; }
    const std::optional<std::string>& get_chief_complaint() const { return chief_complaint; }
    const std::optional<std::string>& get_diagnosis() const { return diagnosis; }
    const std::optional<std::string>& get_treatment() const { return treatment; }
    const std::optional<std::string>& get_medication() const { return medication; }
    const std::optional<std::string>& get_notes() const { return notes; }
    const std::string& get_created_at() const { return created_at; }
    const std::string& get_updated_at() const { return updated_at; }

    // Setters
    void set_id(int id) { this->id = id; }
    void set_appointment_id(int appointment_id) { this->appointment_id = appointment_id; }
    void set_chief_complaint(const std::optional<std::string>& chief_complaint) { this->chief_complaint = chief_complaint; }
    void set_diagnosis(const std::optional<std::string>& diagnosis) { this->diagnosis = diagnosis; }
    void set_treatment(const std::optional<std::string>& treatment) { this->treatment = treatment; }
    void set_medication(const std::optional<std::string>& medication) { this->medication = medication; }
    void set_notes(const std::optional<std::string>& notes) { this->notes = notes; }
    void set_created_at(const std::string& created_at) { this->created_at = created_at; }
    void set_updated_at(const std::string& updated_at) { this->updated_at = updated_at; }

    // JSON 序列化和反序列化
    friend void to_json(json& j, const Record& record) {
        j = json{
            {"id", record.id},
            {"appointment_id", record.appointment_id},
            {"chief_complaint", record.chief_complaint},
            {"diagnosis", record.diagnosis},
            {"treatment", record.treatment},
            {"medication", record.medication},
            {"notes", record.notes},
            {"created_at", record.created_at},
            {"updated_at", record.updated_at}
        };
    }

    friend void from_json(const json& j, Record& record) {
        j.at("appointment_id").get_to(record.appointment_id);
        if (j.contains("chief_complaint") && !j.at("chief_complaint").is_null()) {
            record.chief_complaint = j.at("chief_complaint").get<std::string>();
        }
        if (j.contains("diagnosis") && !j.at("diagnosis").is_null()) {
            record.diagnosis = j.at("diagnosis").get<std::string>();
        }
        if (j.contains("treatment")) {
            record.treatment = j.at("treatment").get<std::string>();
        }
        if (j.contains("medication")) {
            record.medication = j.at("medication").get<std::string>();
        }
        if (j.contains("notes")) {
            record.notes = j.at("notes").get<std::string>();
        }
    }

private:
    int id = 0;
    int appointment_id = 0;
    std::optional<std::string> chief_complaint;
    std::optional<std::string> diagnosis;
    std::optional<std::string> treatment;
    std::optional<std::string> medication;
    std::optional<std::string> notes;
    std::string created_at;
    std::string updated_at;
};

} // namespace pet_hospital
