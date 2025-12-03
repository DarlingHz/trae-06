#pragma once

#include <string>
#include <ctime>

class User {
public:
    int id = 0;
    std::string name;
    std::string email;
    time_t createdAt = 0;

    User() = default;
    User(int id, const std::string& name, const std::string& email, time_t createdAt)
        : id(id), name(name), email(email), createdAt(createdAt) {}

    bool isValid() const {
        return !name.empty() && !email.empty();
    }
};
