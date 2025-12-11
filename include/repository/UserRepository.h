#pragma once

#include "models/User.h"
#include <memory>
#include <string>
#include "DatabasePool.h"

namespace repository {

class UserRepository {
public:
    virtual ~UserRepository() = default;

    virtual std::shared_ptr<models::User> create(const std::string& email, 
                                                 const std::string& password_hash, 
                                                 const std::string& nickname) = 0;
    
    virtual std::shared_ptr<models::User> find_by_id(int id) = 0;
    
    virtual std::shared_ptr<models::User> find_by_email(const std::string& email) = 0;
    
    virtual bool update(std::shared_ptr<models::User> user) = 0;
};

std::unique_ptr<UserRepository> create_user_repository(DatabasePool& db_pool);

} // namespace repository
