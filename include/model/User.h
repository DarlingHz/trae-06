#ifndef USER_H
#define USER_H

#include <string>

namespace model {

struct User {
    int id;
    std::string email;
    std::string password_hash;
    std::string nickname;
    std::string timezone;
    std::string created_at;
};

} // namespace model

#endif // USER_H
