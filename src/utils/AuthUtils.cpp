#include "utils/AuthUtils.hpp"
#include "utils/BcryptUtils.hpp"
#include <nlohmann/json.hpp>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <regex>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace utils {

// Initialize static members
const std::string AuthUtils::secret_key_ = "your-secret-key-here-change-in-production";
const int AuthUtils::token_expiration_hours_ = 24;

std::string AuthUtils::generateToken(int user_id, const std::string& username) {
    // Create header
    std::string header = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";

    // Create payload with expiration time
    auto now = std::chrono::system_clock::now();
    auto expiration = now + std::chrono::hours(token_expiration_hours_);
    auto now_epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    auto expiration_epoch = std::chrono::duration_cast<std::chrono::seconds>(expiration.time_since_epoch()).count();

    std::stringstream payload_ss;
    payload_ss << "{\"user_id\":\"" << user_id << "\",\"username\":\"" << username << "\",\"iat\":\"" << now_epoch << "\",\"exp\":\"" << expiration_epoch << "\"}";
    std::string payload = payload_ss.str();

    // Base64url encode header and payload
    std::string encoded_header = base64UrlEncode(header);
    std::string encoded_payload = base64UrlEncode(payload);

    // Create signature
    std::string data = encoded_header + "." + encoded_payload;
    std::string signature = createHmac(data);
    std::string encoded_signature = base64UrlEncode(signature);

    // Combine all parts to form the token
    return encoded_header + "." + encoded_payload + "." + encoded_signature;
}

bool AuthUtils::validateToken(const std::string& token, int& user_id, std::string& username) {
    // Parse token into components
    std::string header, payload, signature;
    if (!parseToken(token, header, payload, signature)) {
        std::cerr << "Invalid token format" << std::endl;
        return false;
    }

    // Verify signature
    std::string data = header + "." + payload;
    std::string expected_signature = createHmac(data);
    std::string decoded_signature = base64UrlDecode(signature);

    if (decoded_signature != expected_signature) {
        std::cerr << "Invalid token signature" << std::endl;
        return false;
    }

    // Decode payload
    std::string decoded_payload = base64UrlDecode(payload);

    // Parse payload JSON
    try {
        nlohmann::json payload_json = nlohmann::json::parse(decoded_payload);

        // Check if token is expired
        auto now = std::chrono::system_clock::now();
        auto now_epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        auto exp_epoch = payload_json["exp"].get<int64_t>();

        if (now_epoch > exp_epoch) {
            std::cerr << "Token has expired" << std::endl;
            return false;
        }

        // Extract user information
        user_id = payload_json["user_id"].get<int>();
        username = payload_json["username"].get<std::string>();

        return true;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Invalid token payload: " << e.what() << std::endl;
        return false;
    }
}

std::string AuthUtils::hashPassword(const std::string& password) {
    // Generate a random salt
    std::string salt = utils::BcryptUtils::generateSalt(12);

    // Hash the password
    std::string hash = utils::BcryptUtils::hashPassword(password, salt);

    // Return the hashed password as a string
    return hash;
}

bool AuthUtils::verifyPassword(const std::string& password, const std::string& hashed_password) {
    // Verify the password against the hashed password
    return utils::BcryptUtils::verifyPassword(password, hashed_password);
}

bool AuthUtils::validateUsername(const std::string& username) {
    // Regular expression for username validation (alphanumeric, 3-20 characters)
    const std::regex username_regex(R"(^[a-zA-Z0-9]{3,20}$)");
    return std::regex_match(username, username_regex);
}

bool AuthUtils::validateEmail(const std::string& email) {
    // Regular expression for email validation
    const std::regex email_regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return std::regex_match(email, email_regex);
}

bool AuthUtils::validatePasswordStrength(const std::string& password) {
    // Check if password is at least 8 characters long
    if (password.length() < 8) {
        return false;
    }

    // Check if password contains at least one letter and one number
    bool has_letter = false;
    bool has_number = false;

    for (char c : password) {
        if (std::isalpha(c)) {
            has_letter = true;
        } else if (std::isdigit(c)) {
            has_number = true;
        }

        // If both letter and number are found, break early
        if (has_letter && has_number) {
            break;
        }
    }

    return has_letter && has_number;
}

int AuthUtils::verifyTokenAndGetUserId(const std::string& token) {
    int user_id = -1;
    std::string username;

    if (validateToken(token, user_id, username)) {
        return user_id;
    } else {
        return -1;
    }
}

// Helper function to encode a string into base64url format
std::string AuthUtils::base64UrlEncode(const std::string& str) {
    // Base64 encode the string
    BIO* bio = BIO_new(BIO_f_base64());
    BIO* b64 = BIO_new(BIO_s_mem());
    b64 = BIO_push(bio, b64);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, str.c_str(), str.length());
    BIO_flush(b64);
    char* buf = nullptr;
    long len = BIO_get_mem_data(b64, &buf);
    std::string encoded_str(buf, len);
    BIO_free_all(b64);

    // Replace base64 characters with base64url characters
    std::replace(encoded_str.begin(), encoded_str.end(), '+', '-');
    std::replace(encoded_str.begin(), encoded_str.end(), '/', '_');

    // Remove padding
    size_t padding_pos = encoded_str.find('=');
    if (padding_pos != std::string::npos) {
        encoded_str = encoded_str.substr(0, padding_pos);
    }

    return encoded_str;
}

// Helper function to decode a base64url encoded string
std::string AuthUtils::base64UrlDecode(const std::string& encoded_str) {
    // Replace base64url characters with base64 characters
    std::string base64_str = encoded_str;
    std::replace(base64_str.begin(), base64_str.end(), '-', '+');
    std::replace(base64_str.begin(), base64_str.end(), '_', '/');

    // Add padding if necessary
    size_t padding = base64_str.length() % 4;
    if (padding > 0) {
        base64_str.append(4 - padding, '=');
    }

    // Base64 decode the string
    BIO* bio = BIO_new(BIO_f_base64());
    BIO* b64 = BIO_new_mem_buf(base64_str.c_str(), base64_str.length());
    b64 = BIO_push(bio, b64);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    long len = BIO_read(b64, buf, sizeof(buf));
    std::string decoded_str(buf, len);
    BIO_free_all(b64);

    return decoded_str;
}

// Helper function to create a HMAC-SHA256 signature
std::string AuthUtils::createHmac(const std::string& data) {
    // Create a HMAC-SHA256 signature using OpenSSL 3.x API
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len = 0;
    
    // Create and initialize HMAC context
    EVP_MAC* mac = EVP_MAC_fetch(nullptr, "HMAC", nullptr);
    EVP_MAC_CTX* ctx = EVP_MAC_CTX_new(mac);
    
    // Set HMAC key and digest algorithm (SHA256)
    OSSL_PARAM params[2];
    params[0] = OSSL_PARAM_construct_utf8_string("digest", (char*)"SHA256", 0);
    params[1] = OSSL_PARAM_construct_end();
    
    EVP_MAC_init(ctx, reinterpret_cast<const unsigned char*>(secret_key_.c_str()), secret_key_.length(), params);
    
    // Update HMAC with data
    EVP_MAC_update(ctx, reinterpret_cast<const unsigned char*>(data.c_str()), data.length());
    
    // Finalize HMAC and get digest
    size_t out_len = sizeof(digest);
    EVP_MAC_final(ctx, digest, &out_len, sizeof(digest));
    digest_len = static_cast<unsigned int>(out_len);
    
    // Clean up
    EVP_MAC_CTX_free(ctx);
    EVP_MAC_free(mac);

    // Convert the digest to a string
    std::string signature(reinterpret_cast<char*>(digest), digest_len);

    return signature;
}

// Helper function to parse a token into its components
bool AuthUtils::parseToken(const std::string& token, std::string& header, std::string& payload, std::string& signature) {
    // Parse the token into its components
    size_t first_dot = token.find('.');
    size_t second_dot = token.find('.', first_dot + 1);

    if (first_dot == std::string::npos || second_dot == std::string::npos) {
        return false;
    }

    header = token.substr(0, first_dot);
    payload = token.substr(first_dot + 1, second_dot - first_dot - 1);
    signature = token.substr(second_dot + 1);

    return true;
}
} // namespace utils