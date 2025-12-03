#ifndef PARKING_UTILS_H
#define PARKING_UTILS_H

#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/rand.h>

// 密码哈希工具
class PasswordHasher {
public:
    // 生成密码哈希
    static std::string hash(const std::string& password);

    // 验证密码
    static bool verify(const std::string& password, const std::string& hash);

private:
    // 生成随机盐
    static std::string generate_salt(size_t length = 16);

    // PBKDF2哈希
    static std::string pbkdf2_hash(const std::string& password, const std::string& salt,
        int iterations = 100000, size_t hash_length = 32);
};

// Token生成工具
class TokenGenerator {
public:
    // 生成随机Token
    static std::string generate(size_t length = 32) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 255);

        std::ostringstream oss;
        for (size_t i = 0; i < length; ++i) {
            unsigned char byte = static_cast<unsigned char>(dis(gen));
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }
        return oss.str();
    }

    // 生成UUID v4
    static std::string generate_uuid() {
        unsigned char buf[16];
        RAND_bytes(buf, 16);

        // 设置UUID版本和变体
        buf[6] = (buf[6] & 0x0F) | 0x40; // UUID v4
        buf[8] = (buf[8] & 0x3F) | 0x80; // RFC4122变体

        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (int i = 0; i < 16; ++i) {
            if (i == 4 || i == 6 || i == 8) {
                oss << '-';
            }
            oss << std::setw(2) << static_cast<int>(buf[i]);
        }
        return oss.str();
    }
};

// 日志工具
class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    // 设置日志级别
    static void set_level(Level level) { current_level_ = level; }

    // 日志输出
    static void log(Level level, const std::string& message);

    // 便捷方法
    static void debug(const std::string& message) { log(Level::DEBUG, message); }
    static void info(const std::string& message) { log(Level::INFO, message); }
    static void warn(const std::string& message) { log(Level::WARNING, message); }
    static void error(const std::string& message) { log(Level::ERROR, message); }

    // 请求日志
    static void log_request(const std::string& method, const std::string& path,
        int status_code, const std::string& client_ip = "");

private:
    static Level current_level_;
    static std::string level_to_string(Level level);
};

// 配置管理
class Config {
public:
    static Config& instance() {
        static Config instance;
        return instance;
    }

    // 加载配置
    void load(const std::string& config_file = "config.json");

    // 获取端口
    int port() const { return port_; }

    // 获取数据库路径
    const std::string& db_path() const { return db_path_; }

    // 获取Token过期时间（秒）
    int token_expiration() const { return token_expiration_; }

private:
    Config() : port_(8080), db_path_("parking.db"), token_expiration_(3600 * 24) {}

    int port_;
    std::string db_path_;
    int token_expiration_;
};

#endif // PARKING_UTILS_H
