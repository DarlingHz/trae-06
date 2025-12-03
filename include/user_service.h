#ifndef USER_SERVICE_H
#define USER_SERVICE_H

#include <optional>
#include <string>
#include "dto.h"

class UserService {
private:
    UserService() = default;
    
public:
    static UserService& instance() {
        static UserService instance;
        return instance;
    }
    
    std::optional<UserDTO> register_user(const RegisterRequest& request);
    std::optional<UserDTO> login(const LoginRequest& request);
    std::optional<UserDTO> get_user_by_id(int user_id);
    std::optional<UserDTO> get_user_by_email(const std::string& email);
    bool is_email_exists(const std::string& email);
};

#endif // USER_SERVICE_H