#include "models/user.h"

namespace models {

User::User(int id, const std::string& name, const std::string& email,
           const std::string& department, Role role, const std::string& password_hash,
           std::time_t created_at, std::time_t updated_at, Status status)
    : id_(id), name_(name), email_(email), department_(department), role_(role),
      password_hash_(password_hash), created_at_(created_at), updated_at_(updated_at),
      status_(status) {
}

User::Role User::role_from_string(const std::string& role_str) {
    if (role_str == "admin") {
        return Role::ADMIN;
    } else if (role_str == "employee") {
        return Role::EMPLOYEE;
    }
    throw std::invalid_argument("Invalid role string: " + role_str);
}

std::string User::role_to_string(Role role) {
    switch (role) {
        case Role::ADMIN: return "admin";
        case Role::EMPLOYEE: return "employee";
        default: return "unknown";
    }
}

User::Status User::status_from_string(const std::string& status_str) {
    if (status_str == "active") {
        return Status::ACTIVE;
    } else if (status_str == "inactive") {
        return Status::INACTIVE;
    }
    throw std::invalid_argument("Invalid status string: " + status_str);
}

std::string User::status_to_string(Status status) {
    switch (status) {
        case Status::ACTIVE: return "active";
        case Status::INACTIVE: return "inactive";
        default: return "unknown";
    }
}

} // namespace models
