#pragma once

#include "models/user.h"
#include <memory>
#include <optional>
#include <vector>

namespace repositories {

class UserRepository {
public:
    UserRepository() = default;
    virtual ~UserRepository() = default;

    virtual std::optional<models::User> find_by_id(int id) const;
    virtual std::optional<models::User> find_by_email(const std::string& email) const;
    virtual std::vector<models::User> find_by_department(const std::string& department) const;
    virtual int create(const models::User& user);
    virtual bool update(const models::User& user);
    virtual bool delete_by_id(int id);
    virtual std::vector<models::User> find_all(int page = 1, int page_size = 20) const;
    virtual int count_all() const;
};

std::unique_ptr<UserRepository> create_user_repository();

} // namespace repositories
