#ifndef JWT_H
#define JWT_H

#include "utils/config.h"
#include "jwt-cpp/jwt.h"
#include <string>

class JWT {
private:
    Config& config_;
    std::string secret_key_;
    int token_expiry_;

    // 私有构造函数，实现单例模式
    JWT();
    JWT(const JWT&) = delete;
    JWT& operator=(const JWT&) = delete;

public:
    // 单例模式获取实例
    static JWT& getInstance();

    // 析构函数
    ~JWT();

    // 生成JWT令牌
    std::string generateToken(int user_id, const std::string& username);

    // 验证JWT令牌
    bool verifyToken(const std::string& token);

    // 从令牌中获取用户ID
    int getUserIdFromToken(const std::string& token);

    // 从令牌中获取用户名
    std::string getUsernameFromToken(const std::string& token);
};

#endif // JWT_H
