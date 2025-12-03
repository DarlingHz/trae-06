#pragma once

#include <string>
#include <optional>

namespace auth {

class JWT {
private:
    std::string secret_key_;
    int expires_in_; // seconds

    std::string base64_encode(const std::string& data) const;
    std::string base64_decode(const std::string& data) const;
    std::string sign(const std::string& header_payload, const std::string& secret) const;
    bool verify_signature(const std::string& header_payload, const std::string& signature, const std::string& secret) const;

public:
    JWT(const std::string& secret_key, int expires_in = 3600);

    std::string generate_token(int user_id) const;
    std::optional<int> validate_token(const std::string& token) const;
};

} // namespace auth
