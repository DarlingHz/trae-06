#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace pet_hospital {

class Appointment {
public:
    enum class Status {
        PENDING = 0,
        COMPLETED = 1,
        CANCELLED = 2
    };

    Appointment() = default;
    Appointment(int id, int user_id, int pet_id, int doctor_id, const std::string& start_time, 
                const std::string& end_time, const std::optional<std::string>& reason, Status status, 
                const std::string& created_at, const std::string& updated_at)
        : id(id), user_id(user_id), pet_id(pet_id), doctor_id(doctor_id), start_time(start_time), 
          end_time(end_time), reason(reason), status(status), created_at(created_at), 
          updated_at(updated_at) {}

    // Getters
    int get_id() const { return id; }
    int get_user_id() const { return user_id; }
    int get_pet_id() const { return pet_id; }
    int get_doctor_id() const { return doctor_id; }
    const std::string& get_start_time() const { return start_time; }
    const std::string& get_end_time() const { return end_time; }
    const std::optional<std::string>& get_reason() const { return reason; }
    Status get_status() const { return status; }
    const std::string& get_created_at() const { return created_at; }
    const std::string& get_updated_at() const { return updated_at; }

    // Setters
    void set_id(int id) { this->id = id; }
    void set_user_id(int user_id) { this->user_id = user_id; }
    void set_pet_id(int pet_id) { this->pet_id = pet_id; }
    void set_doctor_id(int doctor_id) { this->doctor_id = doctor_id; }
    void set_start_time(const std::string& start_time) { this->start_time = start_time; }
    void set_end_time(const std::string& end_time) { this->end_time = end_time; }
    void set_reason(const std::optional<std::string>& reason) { this->reason = reason; }
    void set_status(Status status) { this->status = status; }
    void set_created_at(const std::string& created_at) { this->created_at = created_at; }
    void set_updated_at(const std::string& updated_at) { this->updated_at = updated_at; }

    // 状态转换为字符串
    static std::string status_to_string(Status status) {
        switch (status) {
            case Status::PENDING:
                return "pending";
            case Status::COMPLETED:
                return "completed";
            case Status::CANCELLED:
                return "cancelled";
            default:
                return "unknown";
        }
    }

    // JSON 序列化和反序列化
    friend void to_json(json& j, const Appointment& appointment) {
        j = json{
            {"id", appointment.id},
            {"user_id", appointment.user_id},
            {"pet_id", appointment.pet_id},
            {"doctor_id", appointment.doctor_id},
            {"start_time", appointment.start_time},
            {"end_time", appointment.end_time},
            {"reason", appointment.reason},
            {"status", static_cast<int>(appointment.status)},
            {"created_at", appointment.created_at},
            {"updated_at", appointment.updated_at}
        };
    }

    friend void from_json(const json& j, Appointment& appointment) {
        j.at("pet_id").get_to(appointment.pet_id);
        j.at("doctor_id").get_to(appointment.doctor_id);
        j.at("start_time").get_to(appointment.start_time);
        j.at("end_time").get_to(appointment.end_time);
        if (j.contains("reason")) {
            appointment.reason = j.at("reason").get<std::string>();
        }
    }

private:
    int id = 0;
    int user_id = 0;
    int pet_id = 0;
    int doctor_id = 0;
    std::string start_time;
    std::string end_time;
    std::optional<std::string> reason;
    Status status = Status::PENDING;
    std::string created_at;
    std::string updated_at;
};

} // namespace pet_hospital
