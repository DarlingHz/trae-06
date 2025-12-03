#pragma once

#include <string>
#include <ctime>
#include <optional>

namespace models {

class ReadReceipt {
public:
    ReadReceipt() = default;
    ReadReceipt(int id, int announcement_id, int user_id, std::time_t read_at,
                const std::optional<std::string>& client_ip = std::nullopt,
                const std::optional<std::string>& user_agent = std::nullopt,
                const std::optional<std::string>& extra_metadata = std::nullopt);

    int get_id() const { return id_; }
    void set_id(int id) { id_ = id; }

    int get_announcement_id() const { return announcement_id_; }
    void set_announcement_id(int announcement_id) { announcement_id_ = announcement_id; }

    int get_user_id() const { return user_id_; }
    void set_user_id(int user_id) { user_id_ = user_id; }

    std::time_t get_read_at() const { return read_at_; }
    void set_read_at(std::time_t read_at) { read_at_ = read_at; }

    const std::optional<std::string>& get_client_ip() const { return client_ip_; }
    void set_client_ip(const std::optional<std::string>& client_ip) { client_ip_ = client_ip; }

    const std::optional<std::string>& get_user_agent() const { return user_agent_; }
    void set_user_agent(const std::optional<std::string>& user_agent) { user_agent_ = user_agent; }

    const std::optional<std::string>& get_extra_metadata() const { return extra_metadata_; }
    void set_extra_metadata(const std::optional<std::string>& extra_metadata) { extra_metadata_ = extra_metadata; }

private:
    int id_ = 0;
    int announcement_id_ = 0;
    int user_id_ = 0;
    std::time_t read_at_ = 0;
    std::optional<std::string> client_ip_;
    std::optional<std::string> user_agent_;
    std::optional<std::string> extra_metadata_;
};

} // namespace models
