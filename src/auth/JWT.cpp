#include "auth/JWT.h"
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <stdexcept>

using json = nlohmann::json;

namespace auth {

JWT::JWT(const std::string& secret_key, int expires_in) 
    : secret_key_(secret_key), expires_in_(expires_in) {
    if(secret_key.empty()) {
        throw std::invalid_argument("Secret key cannot be empty");
    }
}

std::string JWT::base64_encode(const std::string& data) const {
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for(const char& c : data) {
        char_array_3[i++] = c;
        if(i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; i < 4; i++)
                result += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if(i) {
        for(int j = i; j < 3; j++)
            char_array_3[j] = 0;

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for(int j = 0; j < i + 1; j++)
            result += base64_chars[char_array_4[j]];

        while(i++ < 3)
            result += '=';
    }

    return result;
}

std::string JWT::base64_decode(const std::string& data) const {
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int i = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<int> base64_index(256, -1);

    for(int j = 0; j < base64_chars.size(); j++)
        base64_index[base64_chars[j]] = j;

    for(const char& c : data) {
        if(base64_index[(unsigned char)c] == -1 || c == '=')
            break;

        char_array_4[i++] = (unsigned char)c;
        if(i == 4) {
            for(int j = 0; j < 4; j++)
                char_array_4[j] = base64_index[char_array_4[j]];

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for(int j = 0; j < 3; j++)
                result += char_array_3[j];
            i = 0;
        }
    }

    if(i) {
        for(int j = i; j < 4; j++)
            char_array_4[j] = 0;

        for(int j = 0; j < 4; j++)
            char_array_4[j] = base64_index[char_array_4[j]];

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for(int j = 0; j < i - 1; j++)
            result += char_array_3[j];
    }

    return result;
}

std::string JWT::sign(const std::string& header_payload, const std::string& secret) const {
    unsigned char digest[EVP_MAX_MD_SIZE];
    size_t digest_len = 0;

    EVP_PKEY* pkey = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, nullptr, 
                                         reinterpret_cast<const unsigned char*>(secret.c_str()), 
                                         secret.length());
    if(pkey == nullptr) {
        throw std::runtime_error("EVP_PKEY_new_mac_key failed");
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if(ctx == nullptr) {
        EVP_PKEY_free(pkey);
        throw std::runtime_error("EVP_MD_CTX_new failed");
    }

    if(EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) != 1) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("EVP_DigestSignInit failed");
    }

    if(EVP_DigestSignUpdate(ctx, header_payload.c_str(), header_payload.length()) != 1) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("EVP_DigestSignUpdate failed");
    }

    digest_len = EVP_MAX_MD_SIZE;
    if(EVP_DigestSignFinal(ctx, digest, &digest_len) != 1) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("EVP_DigestSignFinal failed");
    }

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);

    std::string signature(reinterpret_cast<char*>(digest), digest_len);
    return base64_encode(signature);
}

bool JWT::verify_signature(const std::string& header_payload, const std::string& signature, const std::string& secret) const {
    std::string expected = sign(header_payload, secret);
    return expected == signature;
}

std::string JWT::generate_token(int user_id) const {
    auto now = std::chrono::system_clock::now();
    auto now_s = std::chrono::system_clock::to_time_t(now);
    auto expires_s = now_s + expires_in_;

    json header = {
        {"alg", "HS256"},
        {"typ", "JWT"}
    };

    json payload = {
        {"user_id", user_id},
        {"iat", now_s},
        {"exp", expires_s}
    };

    std::string header_str = header.dump();
    std::string payload_str = payload.dump();

    std::string encoded_header = base64_encode(header_str);
    std::string encoded_payload = base64_encode(payload_str);

    std::string header_payload = encoded_header + "." + encoded_payload;
    std::string signature = sign(header_payload, secret_key_);

    return header_payload + "." + signature;
}

std::optional<int> JWT::validate_token(const std::string& token) const {
    std::size_t pos1 = token.find('.');
    std::size_t pos2 = token.find('.', pos1 + 1);

    if(pos1 == std::string::npos || pos2 == std::string::npos)
        return std::nullopt;

    std::string encoded_header = token.substr(0, pos1);
    std::string encoded_payload = token.substr(pos1 + 1, pos2 - pos1 - 1);
    std::string signature = token.substr(pos2 + 1);

    std::string header_payload = encoded_header + "." + encoded_payload;

    if(!verify_signature(header_payload, signature, secret_key_))
        return std::nullopt;

    try {
        std::string payload_str = base64_decode(encoded_payload);
        json payload = json::parse(payload_str);

        auto now = std::chrono::system_clock::now();
        auto now_s = std::chrono::system_clock::to_time_t(now);

        if(payload["exp"] < now_s)
            return std::nullopt;

        return payload["user_id"];
    } catch(const std::exception& e) {
        return std::nullopt;
    }
}

} // namespace auth
