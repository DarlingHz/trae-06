#ifndef SESSION_H
#define SESSION_H

#include <string>
#include <chrono>

namespace model {

class Session {
public:
    Session() = default;
    Session(int id, int user_id, const std::string& token,
            const std::chrono::system_clock::time_point& expire_at,
            const std::chrono::system_clock::time_point& created_at)
        : id_(id), user_id_(user_id), token_(token), expire_at_(expire_at), created_at_(created_at) {}

    // Getters
    int getId() const { return id_; }
    int getUserId() const { return user_id_; }
    const std::string& getToken() const { return token_; }
    const std::chrono::system_clock::time_point& getExpireAt() const { return expire_at_; }
    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setUserId(int user_id) { user_id_ = user_id; }
    void setToken(const std::string& token) { token_ = token; }
    void setExpireAt(const std::chrono::system_clock::time_point& expire_at) { expire_at_ = expire_at; }
    void setCreatedAt(const std::chrono::system_clock::time_point& created_at) { created_at_ = created_at; }

    // 辅助方法
    bool isExpired() const {
        return std::chrono::system_clock::now() > expire_at_;
    }

private:
    int id_ = 0;
    int user_id_ = 0;
    std::string token_;
    std::chrono::system_clock::time_point expire_at_;
    std::chrono::system_clock::time_point created_at_;
};

} // namespace model

#endif // SESSION_H