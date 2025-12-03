#pragma once

#include <string>
#include <ctime>

namespace models {

class User {
public:
    enum class Role {
        ADMIN,
        EMPLOYEE
    };

    enum class Status {
        ACTIVE,
        INACTIVE
    };

    User() = default;
    User(int id, const std::string& name, const std::string& email, 
         const std::string& department, Role role, const std::string& password_hash,
         std::time_t created_at, std::time_t updated_at, Status status);

    int get_id() const { return id_; }
    void set_id(int id) { id_ = id; }

    const std::string& get_name() const { return name_; }
    void set_name(const std::string& name) { name_ = name; }

    const std::string& get_email() const { return email_; }
    void set_email(const std::string& email) { email_ = email; }

    const std::string& get_department() const { return department_; }
    void set_department(const std::string& department) { department_ = department; }

    Role get_role() const { return role_; }
    void set_role(Role role) { role_ = role; }
    static Role role_from_string(const std::string& role_str);
    static std::string role_to_string(Role role);

    const std::string& get_password_hash() const { return password_hash_; }
    void set_password_hash(const std::string& password_hash) { password_hash_ = password_hash; }

    std::time_t get_created_at() const { return created_at_; }
    void set_created_at(std::time_t created_at) { created_at_ = created_at; }

    std::time_t get_updated_at() const { return updated_at_; }
    void set_updated_at(std::time_t updated_at) { updated_at_ = updated_at; }

    Status get_status() const { return status_; }
    void set_status(Status status) { status_ = status; }
    static Status status_from_string(const std::string& status_str);
    static std::string status_to_string(Status status);

private:
    int id_ = 0;
    std::string name_;
    std::string email_;
    std::string department_;
    Role role_ = Role::EMPLOYEE;
    std::string password_hash_;
    std::time_t created_at_ = 0;
    std::time_t updated_at_ = 0;
    Status status_ = Status::ACTIVE;
};

} // namespace models
