#ifndef HASH_H
#define HASH_H

#include <string>

class Hash {
private:
    static const int SALT_LENGTH = 16;

    // 私有构造函数，实现单例模式
    Hash();
    Hash(const Hash&) = delete;
    Hash& operator=(const Hash&) = delete;

public:
    // 单例模式获取实例
    static Hash& getInstance();

    // 析构函数
    ~Hash();

    // 生成盐值
    std::string generateSalt();

    // 对密码进行哈希
    std::string hashPassword(const std::string& password, const std::string& salt);

    // 验证密码
    bool verifyPassword(const std::string& password, const std::string& hash, const std::string& salt);
};

#endif // HASH_H
