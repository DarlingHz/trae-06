#pragma once
#include <string>
#include <optional>

namespace domain {

struct User {
    int id;
    std::string name;
    std::string department;
    std::string role;

    // Default constructor
    User() : id(-1) {}
    
    // Constructor with parameters
    User(int id, std::string name, std::string department, std::string role)
        : id(id), name(std::move(name)), department(std::move(department)), role(std::move(role)) {}
};

} // namespace domain
