#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <optional>
#include "dto.h"

class AuthManager {
private:
    AuthManager() = default;
    
public:
    static AuthManager& instance() {
        static AuthManager instance;
        return instance;
    }
    
    // 密码哈希
    std::string hash_password(const std::string& password);
    
    // 密码验证
    bool verify_password(const std::string& password, const std::string& hash);
    
    // 生成token
    std::string generate_token(const UserDTO& user);
    
    // 验证token并解析用户信息
    std::optional<UserDTO> verify_token(const std::string& token);
    
    // 从HTTP头获取token
    std::optional<std::string> extract_token_from_header(const std::string& header);
};

#endif // AUTH_H