#ifndef CHECK_IN_LOG_H
#define CHECK_IN_LOG_H

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

namespace event_signup_service::model {

enum class CheckInChannel {
    QR_CODE,     // 二维码签到
    MANUAL,      // 手动签到
    UNKNOWN      // 未知渠道
};

class CheckInLog {
private:
    int64_t id_{};
    int64_t registration_id_{};
    std::chrono::system_clock::time_point check_in_time_{};
    CheckInChannel channel_{CheckInChannel::UNKNOWN};

public:
    // 构造函数
    CheckInLog() = default;
    CheckInLog(
        int64_t registration_id,
        CheckInChannel channel = CheckInChannel::UNKNOWN
    ) : registration_id_(registration_id),
        check_in_time_(std::chrono::system_clock::now()),
        channel_(channel) {}

    // Getter方法
    int64_t id() const { return id_; }
    int64_t registration_id() const { return registration_id_; }
    const std::chrono::system_clock::time_point& check_in_time() const { return check_in_time_; }
    CheckInChannel channel() const { return channel_; }

    // Setter方法
    void set_id(int64_t id) { id_ = id; }

    // 辅助方法
    std::string channel_to_string() const;
    static CheckInChannel string_to_channel(const std::string& channel);

    // JSON序列化
    friend void to_json(nlohmann::json& j, const CheckInLog& log);
    friend void from_json(const nlohmann::json& j, CheckInLog& log);
};

} // namespace event_signup_service::model

#endif // CHECK_IN_LOG_H
